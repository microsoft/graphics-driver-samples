#pragma once
#include "..\roscommon\Vc4Qpu.h"
#include "HLSLBinary.hpp"

#if VC4

class RosCompiler;

typedef enum _VC4_UNIFORM_TYPE
{
    VC4_UNIFORM_TYPE_INVALID,
    VC4_UNIFORM_TYPE_USER_CONSTANT,
    VC4_UNIFORM_TYPE_SAMPLER_CONFIG_P0,
    VC4_UNIFORM_TYPE_SAMPLER_CONFIG_P1,
    VC4_UNIFORM_TYPE_SAMPLER_CONFIG_P2,
    VC4_UNIFORM_TYPE_VIEWPORT_SCALE_X,
    VC4_UNIFORM_TYPE_VIEWPORT_SCALE_Y,
} VC4_UNIFORM_TYPE;

struct VC4_UNIFORM_FORMAT
{
    VC4_UNIFORM_FORMAT() :
        Type(VC4_UNIFORM_TYPE_INVALID)
    {
        memset(&value[0], 0, sizeof(value));
    }

    VC4_UNIFORM_TYPE Type;
    union
    {
        struct
        {
            UINT32 bufferSlot;    // slot X as register (bX).
            UINT32 bufferOffset;  // offset n as float[n]
        } userConstant;
        struct
        {
            UINT32 samplerIndex;  // sampler slot
            UINT32 resourceIndex; // resource binding slot.
            UINT32 samplerConfiguration; // default sampler configuration embeded in shader code.
        } sampilerConfiguration;

        UINT32 value[4];
    };
};

#define VC4_DEFAULT_STORAGE_SIZE      64
#define VC4_DEFAULT_STORAGE_INCREMENT 32

class Vc4ShaderStorage
{
public:

    Vc4ShaderStorage() :
        pStorage(NULL),
        cStorage(0),
        pCurrent(NULL),
        cUsed(0)
    { ; }

    ~Vc4ShaderStorage()
    {
        Reset();
    }
    
    HRESULT Initialize();
    void Reset()
    {
        delete[] this->pStorage;
        this->pStorage = NULL;
        this->cStorage = 0;
        this->pCurrent = NULL;
        this->cUsed = 0;
    }
    HRESULT Grow(uint32_t size);
    HRESULT CopyFrom(Vc4ShaderStorage &Storage)
    {
        HRESULT hr = this->Ensure(Storage.GetUsedSize());
        if (FAILED(hr))
        {
            return hr;
        }
        memcpy(this->pStorage, Storage.GetStorage(), Storage.GetUsedSize());
        this->cUsed = Storage.GetUsedSize();
        this->pCurrent = this->pStorage + this->cUsed;
        return S_OK;
    }

    HRESULT Ensure(uint32_t size, boolean bGrow = true)
    {
        assert(this->cStorage >= this->cUsed);
        if (size > (this->cStorage - this->cUsed))
        {
            if (bGrow)
            {
                return this->Grow(size);
            }
            return E_FAIL;
        }
        return S_OK;
    }

    void Store(BYTE *p, uint32_t size)
    { 
        assert(this->cStorage > this->cUsed);
        assert(SUCCEEDED(this->Ensure(size, false)));
        memcpy(this->pCurrent, p, size);
        this->pCurrent += size;
        this->cUsed += size;
    }
    
    BYTE *GetStorage()
    {
        return this->pStorage;
    }

    uint32_t GetUsedSize()
    {
        return this->cUsed;
    }

    template<class _Ty>
    HRESULT Ensure(uint32_t size)
    {
        return Ensure(size * sizeof(_Ty));
    }

    template <class _Ty>
    void Store(_Ty p)
    {
        assert(SUCCEEDED(this->Ensure(sizeof(_Ty), false)));
        this->Store(reinterpret_cast<BYTE*>(&p), (sizeof(_Ty)));
    }

    template<class _Ty>
    _Ty *GetStorage()
    {
        return (_Ty*)(this->GetStorage());
    }

    template<class _Ty>
    uint32_t GetUsedSize()
    {
        assert((this->cUsed % sizeof(_Ty)) == 0);
        return this->cUsed / sizeof(_Ty);
    }

private:

    BYTE *pStorage;
    BYTE *pCurrent;
    uint32_t cStorage;
    uint32_t cUsed;
};

typedef enum
{
    vc4_reg_vpm,
    vc4_reg_vary,
    vc4_reg_temp,
} vc4_register_usage;

class Vc4Shader
{
public:

    Vc4Shader(RosCompiler *Compiler) :
        UmdCompiler(Compiler),
        uShaderType(D3D11_SB_RESERVED0), // invalid shader type.
        CurrentStorage(NULL),
        CurrentUniform(NULL),
        ShaderStorage(NULL),
        ShaderUniform(NULL),
        ShaderStorageAux(NULL),
        ShaderUniformAux(NULL),
        cInput(0),
        cOutput(0),
        cTemp(0),
        cSampler(0),
        cConstants(0),
        cResource(0)
    { 
        memset(this->InputRegister, 0, sizeof(this->InputRegister));
        memset(this->OutputRegister, 0, sizeof(this->OutputRegister));
        memset(this->TempRegister, 0, sizeof(this->TempRegister));
        memset(this->ResourceDimension, 0, sizeof(this->ResourceDimension));
    }
    ~Vc4Shader() { ; }

    void SetShaderCode(const UINT *pShaderCode)
    {
        HLSLParser.SetShader(pShaderCode);
        uShaderType = HLSLParser.ShaderType();
    }

    void SetShaderStorage(Vc4ShaderStorage *Storage, Vc4ShaderStorage *Uniform)
    {
        this->ShaderStorage = Storage;
        this->ShaderUniform = Uniform;
    }

    void SetShaderStorageAux(Vc4ShaderStorage *Storage, Vc4ShaderStorage *Uniform)
    {
        this->ShaderStorageAux = Storage;
        this->ShaderUniformAux = Uniform;
    }

    void SetInputSignatures(UINT numInputSignatureEntries, D3D11_1DDIARG_SIGNATURE_ENTRY *pInputSignatureEntries)
    {
        numInputSignatureEntries;
        pInputSignatureEntries;
    }
    void SetOutputSignatures(UINT numOutputSignatureEntries, D3D11_1DDIARG_SIGNATURE_ENTRY *pOutputSignatureEntries)
    {
        numOutputSignatureEntries;
        pOutputSignatureEntries;
    }

    HRESULT Translate_VS(); // vertex shader
    HRESULT Translate_PS(); // Fragmaent shader

private:

    void SetCurrentStorage(Vc4ShaderStorage *Storage, Vc4ShaderStorage *Uniform)
    {
        this->CurrentStorage = Storage;
        this->CurrentUniform = Uniform;
    }

    boolean HLSL_GetShaderInstruction(CInstruction &Inst)
    {
        if (!HLSLParser.EndOfShader())
        {
            HLSLParser.ParseInstruction(&Inst);
            return true;
        }
        return false;
    }
    
    boolean HLSL_PeekShaderInstructionOpCode(D3D10_SB_OPCODE_TYPE &OpCode)
    {
          if (!HLSLParser.EndOfShader())
        {
            OpCode = HLSLParser.PeekNextInstructionOpCode();
            return true;
        }
        return false;
    }

    void HLSL_ParseDecl();

    void Emit_Prologue_VS();
    void Emit_Prologue_PS();
    void Emit_Epilogue();
    void Emit_ShaderOutput_VS(boolean bVS);

    void Emit_Mov(CInstruction &Inst);

    void Emit_with_Add_pipe(CInstruction &Inst);
    void Emit_with_Mul_pipe(CInstruction &Inst);

    void Emit_Sample(CInstruction &Inst);

    Vc4Register Find_Vc4Register_M(COperandBase c, uint8_t swizzleMask)
    {
        Vc4Register ret;

        assert(c.m_IndexDimension == D3D10_SB_OPERAND_INDEX_1D);
        assert(c.m_IndexType[0] == D3D10_SB_OPERAND_INDEX_IMMEDIATE32);
 
        switch (c.m_Type)
        { 
        case D3D10_SB_OPERAND_TYPE_INPUT:
            for (uint8_t i = 0; i < 4; i++)
            {
                if (this->InputRegister[c.m_Index[0].m_RegIndex][i].GetFlags().valid && 
                    (this->InputRegister[c.m_Index[0].m_RegIndex][i].GetSwizzleMask() & swizzleMask))
                {
                    return this->InputRegister[c.m_Index[0].m_RegIndex][i];
                }
            }
            break;
        case D3D10_SB_OPERAND_TYPE_OUTPUT:
            for (uint8_t i = 0; i < 4; i++)
            {
                if (this->OutputRegister[c.m_Index[0].m_RegIndex][i].GetFlags().valid && 
                    (this->OutputRegister[c.m_Index[0].m_RegIndex][i].GetSwizzleMask() & swizzleMask))
                {
                    return this->OutputRegister[c.m_Index[0].m_RegIndex][i];
                }
            }
            break;
        case D3D10_SB_OPERAND_TYPE_TEMP:
            for (uint8_t i = 0; i < 4; i++)
            {
                if (this->TempRegister[c.m_Index[0].m_RegIndex][i].GetFlags().valid && 
                    (this->TempRegister[c.m_Index[0].m_RegIndex][i].GetSwizzleMask() & swizzleMask))
                {
                    return this->TempRegister[c.m_Index[0].m_RegIndex][i];
                }
            }
            break;
        default:
            assert(false);
        }

        assert(ret.GetFlags().valid);
        return ret;
    }

    Vc4Register Find_Vc4Register_I(COperandBase c, uint8_t swizzleIndex)
    {
        Vc4Register ret;

        switch (c.m_Type)
        {
        case D3D10_SB_OPERAND_TYPE_IMMEDIATE32:
        {
            assert(c.m_Modifier == D3D10_SB_OPERAND_MODIFIER_NONE);

            switch (c.m_NumComponents)
            {
            case D3D10_SB_OPERAND_1_COMPONENT:
                assert(c.m_IndexDimension == D3D10_SB_OPERAND_INDEX_0D);
                ret.SetImmediateI(c.m_Value[0]);
                break;
            case D3D10_SB_OPERAND_4_COMPONENT:
                assert(c.m_IndexDimension == D3D10_SB_OPERAND_INDEX_1D);
                ret.SetImmediateI(c.m_Value[swizzleIndex]);
                break;
            default:
                assert(false);
            }
            break;
        }
        case D3D10_SB_OPERAND_TYPE_IMMEDIATE64:
        {
            // 64bit load is not supported.
            assert(false);
            break;
        }
        case D3D10_SB_OPERAND_TYPE_OUTPUT:
        {
            // output can't be source.
            assert(false);
            break;
        }
        case D3D10_SB_OPERAND_TYPE_CONSTANT_BUFFER:
        {
            assert(c.m_NumComponents == D3D10_SB_OPERAND_4_COMPONENT);
            assert(c.m_ComponentSelection == D3D10_SB_OPERAND_4_COMPONENT_SWIZZLE_MODE);

            assert(c.m_IndexDimension == D3D10_SB_OPERAND_INDEX_2D);
            assert(c.m_IndexType[0] == D3D10_SB_OPERAND_INDEX_IMMEDIATE32);
            assert(c.m_IndexType[1] == D3D10_SB_OPERAND_INDEX_IMMEDIATE32);

            Vc4Register unif(VC4_QPU_ALU_REG_A, VC4_QPU_RADDR_UNIFORM); // TODO: fix hardcoded REG_A.
            ret = unif;

            {
                VC4_UNIFORM_FORMAT u;
                u.Type = VC4_UNIFORM_TYPE_USER_CONSTANT;
                u.userConstant.bufferSlot = c.m_Index[0].m_RegIndex;
                u.userConstant.bufferOffset = (c.m_Index[1].m_RegIndex * 4) + c.m_Swizzle[swizzleIndex];
                this->AddUniformReference(u);
            }

            ret.SetModifier(c.m_Modifier);
            break;
        }
        case D3D10_SB_OPERAND_TYPE_TEMP:
        case D3D10_SB_OPERAND_TYPE_INPUT:
        {
            assert(c.m_Modifier == D3D10_SB_OPERAND_MODIFIER_NONE);

            assert(c.m_NumComponents == D3D10_SB_OPERAND_4_COMPONENT);
            assert(c.m_IndexDimension == D3D10_SB_OPERAND_INDEX_1D);
            assert(c.m_IndexType[0] == D3D10_SB_OPERAND_INDEX_IMMEDIATE32);

            switch (c.m_ComponentSelection)
            {
            case D3D10_SB_OPERAND_4_COMPONENT_SWIZZLE_MODE:
                ret = Find_Vc4Register_M(c, D3D10_SB_OPERAND_4_COMPONENT_MASK(c.m_Swizzle[swizzleIndex]));
                break;
            case D3D10_SB_OPERAND_4_COMPONENT_SELECT_1_MODE:
                ret = Find_Vc4Register_M(c, D3D10_SB_OPERAND_4_COMPONENT_MASK(c.m_ComponentName));
                break;
            default:
                assert(false);
            }
            break;
        }
        default:
            assert(false);
        }
        
        assert(ret.GetFlags().valid);
        return ret;
    }

    boolean Resolve_Modifier(Vc4Register &src, uint8_t tempRIndex = 0)
    {
        boolean bReplaced = false;

        assert(src.GetFlags().immediate == false);
        switch (src.modifier)
        {
        case D3D10_SB_OPERAND_MODIFIER_NONE:
            break;
        case D3D10_SB_OPERAND_MODIFIER_NEG:
        {
            Vc4Register value; value.SetImmediateF(-1.0f);
            Vc4Register rX(VC4_QPU_ALU_R1 + tempRIndex, VC4_QPU_WADDR_ACC1 + tempRIndex);

            {
                Vc4Instruction Vc4Inst(vc4_load_immediate_32);
                Vc4Inst.Vc4_m_LOAD32(rX, value);
                Vc4Inst.Emit(CurrentStorage);
            }

            {
                Vc4Instruction Vc4Inst;
                Vc4Inst.Vc4_m_FMUL(rX, src, rX);
                Vc4Inst.Emit(CurrentStorage);
            }

            src = rX;
            bReplaced = true;
            break;
        }
        case D3D10_SB_OPERAND_MODIFIER_ABS:
        case D3D10_SB_OPERAND_MODIFIER_ABSNEG:
            // TODO:
            assert(false);
            break;
        default:
            assert(false);
        }

        return bReplaced;
    }

    void Setup_SourceRegister(CInstruction &Inst, uint8_t opIndex, uint8_t sizzleIndex, Vc4Register &src)
    {
        src = Find_Vc4Register_I(Inst.m_Operands[opIndex], sizzleIndex);
        assert(src.GetFlags().valid);
    }

    void Setup_SourceRegisters(CInstruction &Inst, uint8_t swizzleIndex, Vc4Register src[], uint8_t len)
    {
        assert(len == 2);
        for (uint8_t i = 0, j = 1; i < 2; i++, j++)
        {
            Setup_SourceRegister(Inst, j, swizzleIndex, src[i]);
            assert(src[i].GetFlags().valid);
            if (src[i].GetFlags().immediate)
            {
                // move immediate value to r1~r2 temorary.
                Vc4Register rX(VC4_QPU_ALU_R1 + i, VC4_QPU_WADDR_ACC1 + i);
                Vc4Instruction Vc4Inst(vc4_load_immediate_32);
                Vc4Inst.Vc4_m_LOAD32(rX, src[i]);
                Vc4Inst.Emit(CurrentStorage);
                src[i] = rX;
            }
            else if (Resolve_Modifier(src[i], i))
            {
                // register is already replaced, so there should be no conflict.
            }
            else if ((j == 2) &&
                     (src[0].GetMux() == src[1].GetMux()) &&
                     (src[0].GetAddr() != src[1].GetAddr()))
            {
                // if any of them is exchangeable (paticularly uniform), switch A and B file to avoid conflict.
                if (VC4_QPU_RADDR_LOOKUP[src[0].GetAddr()].Exchangeable)
                {
                    assert(src[0].GetAddr() == VC4_QPU_RADDR_UNIFORM);
                    src[0].SetMux(src[0].GetMux() == VC4_QPU_ALU_REG_A ? VC4_QPU_ALU_REG_B : VC4_QPU_ALU_REG_A);
                }
                else if (VC4_QPU_RADDR_LOOKUP[src[1].GetAddr()].Exchangeable)
                {
                    assert(src[1].GetAddr() == VC4_QPU_RADDR_UNIFORM);
                    src[1].SetMux(src[1].GetMux() == VC4_QPU_ALU_REG_A ? VC4_QPU_ALU_REG_B : VC4_QPU_ALU_REG_A);
                }
                else
                {
                    // move to r2 to avoid confilct in register file.
                    Vc4Register r2(VC4_QPU_ALU_R2, VC4_QPU_WADDR_ACC2);
                    Vc4Instruction Vc4Inst;
                    Vc4Inst.Vc4_m_MOV(r2, src[1]);
                    Vc4Inst.Emit(CurrentStorage);
                    src[1] = r2;
                }
            }
        }
    }
    
    uint8_t SwizzleMaskToIndex(uint8_t aMask)
    {
        uint8_t ret = 0;
        switch (aMask)
        {
        case D3D10_SB_OPERAND_4_COMPONENT_MASK_X:
            ret = 0;
            break;
        case D3D10_SB_OPERAND_4_COMPONENT_MASK_Y:
            ret = 1;
            break;
        case D3D10_SB_OPERAND_4_COMPONENT_MASK_Z:
            ret = 2;
            break;
        case D3D10_SB_OPERAND_4_COMPONENT_MASK_W:
            ret = 3;
            break;
        default:
            assert(false);
        }
        return ret;
    }

    HRESULT AddUniformReference(VC4_UNIFORM_FORMAT &fmt)
    {
        HRESULT hr = CurrentUniform->Ensure<VC4_UNIFORM_FORMAT>(1);
        if (FAILED(hr))
        {
            return hr;
        }
        CurrentUniform->Store<VC4_UNIFORM_FORMAT>(fmt);
        return S_OK;
    }

private:

    RosCompiler *UmdCompiler;

    D3D10_SB_TOKENIZED_PROGRAM_TYPE uShaderType;

    CShaderCodeParser HLSLParser;

    Vc4ShaderStorage *CurrentStorage;
    Vc4ShaderStorage *CurrentUniform;
    Vc4ShaderStorage *ShaderStorage; // Used for vertex/pixel shader.
    Vc4ShaderStorage *ShaderUniform;
    Vc4ShaderStorage *ShaderStorageAux; // Used for coordinate shader.
    Vc4ShaderStorage *ShaderUniformAux;

    uint8_t cSampler;
    uint8_t cConstants;

    // Register map
    uint8_t cInput;
    Vc4Register InputRegister[8][4];

    uint8_t cOutput;
    Vc4Register OutputRegister[8][4];

    uint8_t cTemp;
    Vc4Register TempRegister[4][4];

    uint8_t cResource;
    uint32_t ResourceDimension[8];

    // TEMPORARY Register Usage Map
    //
    // ra0 ~ ra14  : Input
#define ROS_VC4_INPUT_REGISTER_FILE        VC4_QPU_ALU_REG_A
#define ROS_VC4_INPUT_REGISTER_FILE_START  0
#define ROS_VC4_INPUT_REGISTER_FILE_END    14
    // ra15        : Reserved - W (in pixel shader only)
    // ra16 ~ ra31 : Temp
#define ROS_VC4_TEMP_REGISTER_FILE         VC4_QPU_ALU_REG_A
#define ROS_VC4_TEMP_REGISTER_FILE_START   16
#define ROS_VC4_TEMP_REGISTER_FILE_END     31
    // rb0 ~ rb14  : Output
#define ROS_VC4_OUTPUT_REGISTER_FILE       VC4_QPU_ALU_REG_B
#define ROS_VC4_OUTPUT_REGISTER_FILE_START 0
#define ROS_VC4_OUTPUT_REGISTER_FILE_END   14
    // rb15        : Reserved - Z (in pixel shader only)

};


#endif // VC4