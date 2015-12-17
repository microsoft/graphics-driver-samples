#pragma once
#include "..\roscommon\Vc4Qpu.h"
#include "HLSLBinary.hpp"

#if VC4

#define VC4_DEFAULT_STORAGE_SIZE 64
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
        delete[] this->pStorage;
    }
    
    HRESULT Initialize();
    HRESULT Grow(uint32_t size);
    HRESULT CopyFrom(Vc4ShaderStorage *Storage)
    {
        HRESULT hr = this->Ensure(Storage->GetStorageUsedSize());
        if (FAILED(hr))
        {
            return hr;
        }
        memcpy(this->pStorage, Storage->GetStorage(), Storage->GetStorageUsedSize());
        this->cUsed = Storage->GetStorageUsedSize();
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
    
    BYTE *GetStorage(uint32_t *pSize = NULL)
    {
        if (pSize)
        {
            *pSize = this->cUsed;
        }
        return this->pStorage;
    }

    uint32_t GetStorageUsedSize()
    {
        return this->cUsed;
    }

    void EmitInstruction(VC4_QPU_INSTRUCTION Instruction)
    {
        assert(SUCCEEDED(this->Ensure(sizeof(Instruction), false)));
        this->Store(reinterpret_cast<BYTE*>(&Instruction), sizeof(Instruction));
    }
    HRESULT EnsureInstruction(uint32_t size)
    {
        return this->Ensure(size * sizeof(VC4_QPU_INSTRUCTION));
    }
    uint32_t GetInstructionSize()
    {
        assert((this->cUsed % sizeof(VC4_QPU_INSTRUCTION)) == 0);
        return this->cUsed / sizeof(VC4_QPU_INSTRUCTION);
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

    Vc4Shader() :
        uShaderType(D3D11_SB_RESERVED0), // invalid shader type.
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

    HRESULT Translate()
    {
        // __debugbreak();

        HRESULT hr = E_NOTIMPL;
        if (this->uShaderType == D3D10_SB_VERTEX_SHADER)
        {
            hr = Translate_VS();
        }
        else if (this->uShaderType == D3D10_SB_PIXEL_SHADER)
        {
            hr = Translate_PS();
        }
        else
        {
            assert(false);
        }

        // __debugbreak();
        
        return hr;
    }

    UINT GetVc4ShaderCodeSize()
    {
        return (max(ShaderStorage.GetStorageUsedSize(), ShaderStorageAux.GetStorageUsedSize()) * 2);
    }

    void GetVc4ShaderCode(BYTE *p, UINT size)
    {
        assert(size == GetVc4ShaderCodeSize());
        memcpy(p, ShaderStorage.GetStorage(), ShaderStorage.GetStorageUsedSize());
        p += (size / 2);
        memcpy(p, ShaderStorageAux.GetStorage(), ShaderStorageAux.GetStorageUsedSize());
    }

private:

    boolean HLSL_GetShaderInstruction(CInstruction *pInst)
    {
        if (!HLSLParser.EndOfShader())
        {
            HLSLParser.ParseInstruction(pInst);
            return true;
        }
        return false;
    }
    
    boolean HLSL_PeekShaderInstructionOpCode(D3D10_SB_OPCODE_TYPE *pOpCode)
    {
        assert(pOpCode);
        if (!HLSLParser.EndOfShader())
        {
            *pOpCode = HLSLParser.PeekNextInstructionOpCode();
            return true;
        }
        return false;
    }

    HRESULT Translate_VS(); // vertex shader
    HRESULT Translate_PS(); // Fragmaent shader

    void HLSL_ParseDecl();

    void Emit_Prologue_VS();
    void Emit_Prologue_PS();

    void Emit_ShaderOutput_VS(boolean bVS, Vc4ShaderStorage *Storage);

    void Emit_Epilogue(Vc4ShaderStorage *Storage);

    void Emit_Mov(CInstruction &Inst);
    void Emit_Mul(CInstruction &Inst);
    void Emit_Sample(CInstruction &Inst);

    Vc4Register Find_Vc4Register(COperandBase c, uint8_t swizzleMask)
    {
        Vc4Register ret;
        assert(c.m_IndexDimension == D3D10_SB_OPERAND_INDEX_1D);
        
        if (c.m_Type == D3D10_SB_OPERAND_TYPE_INPUT)
        {
            for (uint8_t i = 0; i < 4; i++)
            {
                if (this->InputRegister[c.m_Index[0].m_RegIndex][i].flags.valid && 
                    (this->InputRegister[c.m_Index[0].m_RegIndex][i].swizzleMask & swizzleMask))
                {
                    return this->InputRegister[c.m_Index[0].m_RegIndex][i];
                }
            }
        }
        else if (c.m_Type == D3D10_SB_OPERAND_TYPE_OUTPUT)
        {
            for (uint8_t i = 0; i < 4; i++)
            {
                if (this->OutputRegister[c.m_Index[0].m_RegIndex][i].flags.valid && 
                    (this->OutputRegister[c.m_Index[0].m_RegIndex][i].swizzleMask & swizzleMask))
                {
                    return this->OutputRegister[c.m_Index[0].m_RegIndex][i];
                }
            }
        }
        else if (c.m_Type == D3D10_SB_OPERAND_TYPE_TEMP)
        {
            for (uint8_t i = 0; i < 4; i++)
            {
                if (this->TempRegister[c.m_Index[0].m_RegIndex][i].flags.valid && 
                    (this->TempRegister[c.m_Index[0].m_RegIndex][i].swizzleMask & swizzleMask))
                {
                    return this->TempRegister[c.m_Index[0].m_RegIndex][i];
                }
            }
        }
        assert(ret.flags.valid);
        return ret;
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

private:

    D3D10_SB_TOKENIZED_PROGRAM_TYPE uShaderType;

    CShaderCodeParser HLSLParser;
    Vc4ShaderStorage ShaderStorage; // Used for vertex/pixel shader.
    Vc4ShaderStorage ShaderStorageAux; // Used for coordinate shader.

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