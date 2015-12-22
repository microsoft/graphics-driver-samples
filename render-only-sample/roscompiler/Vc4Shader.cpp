#include "roscompiler.h"

HRESULT Vc4ShaderStorage::Initialize()
{
    this->pStorage = new BYTE[VC4_DEFAULT_STORAGE_SIZE];
    if (this->pStorage == NULL)
    {
        return E_OUTOFMEMORY;
    }
    this->cStorage = VC4_DEFAULT_STORAGE_SIZE;
    this->pCurrent = this->pStorage;
    this->cUsed = 0;

    return S_OK;
}

HRESULT Vc4ShaderStorage::Grow(uint32_t size)
{
    uint32_t NewSize = this->cStorage + max(size, VC4_DEFAULT_STORAGE_INCREMENT);
    BYTE *pNew = new BYTE[NewSize];
    if (pNew == NULL)
    {
        return E_OUTOFMEMORY;
    }
    memcpy(pNew, this->pStorage, this->cStorage);
    delete[] this->pStorage;

    this->pStorage = pNew;
    this->pCurrent = this->pStorage + this->cUsed;
    this->cStorage = NewSize;

    return S_OK;
}

void Vc4Shader::Emit_Prologue_VS()
{
    assert(this->uShaderType == D3D10_SB_VERTEX_SHADER);
    
    CurrentStorage->Ensure<VC4_QPU_INSTRUCTION>(this->cInput + 3); // +2 for vr_setup/vw_setup and NOP.

    assert(cInput < 16); // VR_SETUP:NUM limitation, vpm only can read up to 16 values.

    {
        Vc4Instruction Vc4Inst(vc4_load_immediate_32);
        Vc4Register vr_setup(VC4_QPU_ALU_REG_A, VC4_QPU_WADDR_VPMVCD_RD_SETUP);
        Vc4Register value; value.SetImmediateI(MAKE_VR_SETUP(cInput, 1, true, false, VC4_QPU_32BIT_VECTOR, 0));
        Vc4Inst.Vc4_a_LOAD32(vr_setup, value);
        Vc4Inst.Emit(CurrentStorage);
    }

    {
        Vc4Instruction Vc4Inst(vc4_load_immediate_32);
        Vc4Register vw_setup(VC4_QPU_ALU_REG_B, VC4_QPU_WADDR_VPMVCD_WR_SETUP);
        Vc4Register value; value.SetImmediateI(MAKE_VW_SETUP(1, true, false, VC4_QPU_32BIT_VECTOR, 0));
        Vc4Inst.Vc4_a_LOAD32(vw_setup, value);
        Vc4Inst.Emit(CurrentStorage);
    }

    for (uint8_t iRegUsed = 0, iRegIndex = 0; iRegUsed < this->cInput; iRegIndex++)
    {
        Vc4Instruction Vc4Inst;
        Vc4Register raX = this->InputRegister[iRegIndex / 4][iRegIndex % 4];
        if (raX.GetFlags().valid)
        {
            assert(raX.GetMux() == VC4_QPU_ALU_REG_A || raX.GetMux() == VC4_QPU_ALU_REG_B);
            Vc4Register vpm(raX.GetMux(), VC4_QPU_RADDR_VPM);
            Vc4Inst.Vc4_a_MOV(raX, vpm);
            Vc4Inst.Emit(CurrentStorage);
            iRegUsed++;
        }
    }

    { // Emit a NOP
        Vc4Instruction Vc4Inst;
        Vc4Inst.Emit(CurrentStorage);
    }
}

void Vc4Shader::Emit_Prologue_PS()
{
    assert(this->uShaderType == D3D10_SB_PIXEL_SHADER);
    
    // assume all requires interpolation. 1 instruction for load, 1 instruction for interpolation, and 1 instruction to store back.
    // +1 for NOP (after read all varying).
    // +1 for sbwait (after interporation).
    CurrentStorage->Ensure<VC4_QPU_INSTRUCTION>((this->cInput * 3) + 1 + 1);

    // Issue mul inputs (from varying) with ra15 (W).
    for (uint8_t i = 0, iRegUsed = 0; iRegUsed < this->cInput; i++)
    {
        Vc4Register raX = this->InputRegister[i / 4][i % 4];
        if (raX.GetFlags().valid)
        {
            Vc4Instruction Vc4Inst;
            assert(raX.GetMux() == VC4_QPU_ALU_REG_A || raX.GetMux() == VC4_QPU_ALU_REG_B);
            Vc4Register varying(VC4_QPU_ALU_REG_B, VC4_QPU_RADDR_VERYING);
            Vc4Register ra15(VC4_QPU_ALU_REG_A, 15);
            Vc4Inst.Vc4_m_FMUL(raX, varying, ra15);
            Vc4Inst.Emit(CurrentStorage);
            iRegUsed++;
        }
    }

    // Issue NOP.
    {
        Vc4Instruction Vc4Inst;
        Vc4Inst.Emit(CurrentStorage);
    }

    // Issue add r5 to each input data to complete interpolation.
    for (uint8_t iRegUsed = 0, iRegIndex = 0; iRegUsed < this->cInput; iRegIndex++)
    {
        Vc4Register raX = this->InputRegister[iRegIndex / 4][iRegIndex % 4];
        if (raX.GetFlags().valid && raX.GetFlags().require_linear_conversion)
        {
            {
                Vc4Instruction Vc4Inst;
                Vc4Register r1(VC4_QPU_ALU_REG_A, VC4_QPU_WADDR_ACC1);
                Vc4Register r5(VC4_QPU_ALU_R5);
                Vc4Inst.Vc4_a_FADD(r1, raX, r5);
                Vc4Inst.Emit(CurrentStorage);
            }
        
            {
                Vc4Instruction Vc4Inst;
                Vc4Register r1(VC4_QPU_ALU_R1);
                Vc4Inst.Vc4_a_MOV(raX, r1);
                Vc4Inst.Emit(CurrentStorage);
            }

            iRegUsed++;
        }
    }
    
    { // Emit a NOP with 'sbwait'
        Vc4Instruction Vc4Inst;
        Vc4Inst.Vc4_Sig(VC4_QPU_SIG_WAIT_FOR_SCOREBOARD);
        Vc4Inst.Emit(CurrentStorage);
    }
}

void Vc4Shader::Emit_ShaderOutput_VS(boolean bVS)
{
    assert(this->uShaderType == D3D10_SB_VERTEX_SHADER);
    
    CurrentStorage->Ensure<VC4_QPU_INSTRUCTION>((bVS ? 0 : 4)                        // raw coordinate outputs X/Y/Z/W.
                                                      + 8                                 // 2 fmul, 2 ftoi, 1 NOP and 1 mov.
                                                      + (bVS ? (this->cOutput - 4) : 0)); // varying output
    
    Vc4Register vpm(VC4_QPU_ALU_REG_B, VC4_QPU_WADDR_VPM);

    Vc4Register pos[4]; // X/Y/Z/W.
    for (uint8_t iPos = 0; iPos < this->cOutput; iPos++)
    {
        if (this->OutputRegister[iPos / 4][iPos % 4].GetFlags().position)
        {
            for (uint8_t i = iPos; i < iPos + 4; i++)
            {
                Vc4Register reg = this->OutputRegister[i / 4][i % 4];
                assert(reg.GetFlags().position);
                pos[this->SwizzleMaskToIndex(reg.GetSwizzleMask())] = reg;
            }
            break;
        }
    }

    // Only CS emits raw coordinates.
    if (!bVS)
    {
        // Xc, Yc, Zc, Wc
        for (uint8_t i = 0; i < 4; i++)
        {
            Vc4Register src = pos[i];
            Vc4Instruction Vc4Inst;
            Vc4Inst.Vc4_m_MOV(vpm, src);
            Vc4Inst.Emit(CurrentStorage);
        }
    }

    // Ys/Xs
    {
        // scale by RT dimension (read from uniform). Result in r0/r1.
        {
            for (uint8_t i = 0; i < 2; i++)
            {
                Vc4Register rX(VC4_QPU_ALU_REG_B, VC4_QPU_WADDR_ACC0 + i); // r0 and r1.
                Vc4Register unif((pos[i].GetMux() == VC4_QPU_ALU_REG_A ? VC4_QPU_ALU_REG_B : VC4_QPU_ALU_REG_A), VC4_QPU_RADDR_UNIFORM); // use opossit register file.
                Vc4Instruction Vc4Inst;
                Vc4Inst.Vc4_m_FMUL(rX, pos[i], unif);
                Vc4Inst.Emit(CurrentStorage);
                {
                    VC4_UNIFORM_FORMAT u;
                    u.Type = (i == 0 ? VC4_UNIFORM_TYPE_VIEWPORT_SCALE_X : VC4_UNIFORM_TYPE_VIEWPORT_SCALE_Y);
                    this->AddUniformReference(u);
                }
            }
        }

        // Convert r0/r1 to 16bits float and packed into pos[0]
        {
            Vc4Register ra16(VC4_QPU_ALU_REG_A, 16);
            for (uint8_t i = 0; i < 2; i++)
            {
                Vc4Register rX(VC4_QPU_ALU_R0 + i); // r0 and r1.
                Vc4Instruction Vc4Inst;
                Vc4Inst.Vc4_a_FTOI(ra16, rX); // REUSE ra16 as temp is no longer needed.
                Vc4Inst.Vc4_a_Pack(VC4_QPU_PACK_A_16a + i);
                Vc4Inst.Emit(CurrentStorage);
            }

            // Issue NOP as ra16 is just written.
            {
                Vc4Instruction Vc4Inst;
                Vc4Inst.Emit(CurrentStorage);
            }

            // Output to vpm.
            {
                Vc4Instruction Vc4Inst;
                Vc4Inst.Vc4_m_MOV(vpm, ra16);
                Vc4Inst.Emit(CurrentStorage);
            }
        }
    }

    // Zc
    {
        Vc4Instruction Vc4Inst;
        Vc4Inst.Vc4_m_MOV(vpm, pos[2]);
        Vc4Inst.Emit(CurrentStorage);
    }

    // 1/W // TODO: this must be 1/Wc
    {
        Vc4Register w;
        Vc4Instruction Vc4Inst;
        Vc4Inst.Vc4_m_MOV(vpm, pos[3]);
        Vc4Inst.Emit(CurrentStorage);
    }

    // Only VS emits "varying" (everything read from vpm except position data).
    if (bVS)
    {
        for (uint8_t i = 0; i < this->cOutput; i++)
        {
            if (this->OutputRegister[i / 4][i % 4].GetFlags().position == false)
            {
                Vc4Register src = this->OutputRegister[i / 4][i % 4];
                Vc4Instruction Vc4Inst;
                Vc4Inst.Vc4_m_MOV(vpm, src);
                Vc4Inst.Emit(CurrentStorage);
            }
        }
    }
}

void Vc4Shader::Emit_Epilogue()
{
    assert(this->uShaderType == D3D10_SB_PIXEL_SHADER ||
           this->uShaderType == D3D10_SB_VERTEX_SHADER);

    // +1 for thrend with color output.
    // +2 for NOP(s) and unblock.
    CurrentStorage->Ensure<VC4_QPU_INSTRUCTION>(3);
    
    // output 'Z'.
    if (D3D10_SB_PIXEL_SHADER == this->uShaderType)
    {
        if (true) // TODO: check if Z is enabled.
        {
            Vc4Register tlb_z(VC4_QPU_ALU_REG_B, VC4_QPU_WADDR_TLB_Z);
            Vc4Register rb15(VC4_QPU_ALU_REG_B, 15); // reserved for Z
            Vc4Instruction Vc4Inst;
            Vc4Inst.Vc4_m_MOV(tlb_z, rb15);
            Vc4Inst.Emit(CurrentStorage);
        }
    }

    // output 'thrend', and output pixel data is PS.
    {
        Vc4Instruction Vc4Inst;
        Vc4Inst.Vc4_Sig(VC4_QPU_SIG_PROGRAM_END);
        if (D3D10_SB_PIXEL_SHADER == this->uShaderType)
        {
            // Output pixel data to 'tlb_c'.
            Vc4Register tlb_c(VC4_QPU_ALU_REG_A, VC4_QPU_WADDR_TLB_COLOUR_ALL);
            Vc4Inst.Vc4_a_MOV(tlb_c, this->OutputRegister[0][0]);
        }
        Vc4Inst.Emit(CurrentStorage);
    }

    // output 2 NOP(s) with 'sbdone' for last nop for pixel shader.
    for (UINT i = 0; i < 2; i++)
    {
        Vc4Instruction Vc4Inst;
        if ((D3D10_SB_PIXEL_SHADER == this->uShaderType) && (i == 1))
        {
            Vc4Inst.Vc4_Sig(VC4_QPU_SIG_SCOREBOARD_UNBLOCK);
        }
        Vc4Inst.Emit(CurrentStorage);
    }
}

void Vc4Shader::Emit_Mov(CInstruction &Inst)
{
    assert(this->uShaderType == D3D10_SB_PIXEL_SHADER ||
        this->uShaderType == D3D10_SB_VERTEX_SHADER);

    assert(Inst.m_NumOperands == 2);

    // max 4 mov(s) + 1 NOP.
    CurrentStorage->Ensure<VC4_QPU_INSTRUCTION>(5);

    {
        for (uint8_t i = 0, aCurrent = D3D10_SB_OPERAND_4_COMPONENT_MASK_X; i < 4; i++)
        {
            if (Inst.m_Operands[0].m_WriteMask & aCurrent)
            {
                Vc4Register dst = Find_Vc4Register_M(Inst.m_Operands[0], aCurrent);
                uint8_t pack = VC4_QPU_PACK_A_32;
                if (dst.GetFlags().packed)
                {
                    pack = VC4_QPU_PACK_MUL_8a + i;
                }

                Vc4Register src;
                Setup_SourceRegister(Inst, 1, i, src);

                if (src.GetFlags().immediate)
                {
                    Vc4Instruction Vc4Inst(vc4_load_immediate_32);
                    Vc4Inst.Vc4_m_LOAD32(dst, src);
                    Vc4Inst.Vc4_m_Pack(pack);
                    Vc4Inst.Emit(CurrentStorage);
                }
                else
                {
                    Vc4Instruction Vc4Inst;
                    Vc4Inst.Vc4_m_MOV(dst, src);
                    Vc4Inst.Vc4_m_Pack(pack);
                    Vc4Inst.Emit(CurrentStorage);
                }
            }

            aCurrent <<= 1;
        }
    }

    { // Emit a NOP
        Vc4Instruction Vc4Inst;
        Vc4Inst.Emit(CurrentStorage);
    }
}

void Vc4Shader::Emit_Add(CInstruction &Inst)
{
    assert(this->uShaderType == D3D10_SB_PIXEL_SHADER ||
           this->uShaderType == D3D10_SB_VERTEX_SHADER);

    assert(Inst.m_NumOperands == 3);

    // max 4 add, includes 1 add + 3 moves(s) for each, + 1 NOP.
    CurrentStorage->Ensure<VC4_QPU_INSTRUCTION>(17);

    {
        for (uint8_t i = 0, aCurrent = D3D10_SB_OPERAND_4_COMPONENT_MASK_X; i < 4; i++)
        {
            if (Inst.m_Operands[0].m_WriteMask & aCurrent)
            {
                Vc4Register dst = Find_Vc4Register_M(Inst.m_Operands[0], aCurrent);
                uint8_t pack = VC4_QPU_PACK_A_32;
                if (dst.GetFlags().packed)
                {
                    pack = VC4_QPU_PACK_MUL_8a + i;
                }

                Vc4Register src[2];
                this->Setup_SourceRegisters(Inst, i, src, ARRAYSIZE(src));

                if (dst.GetFlags().packed)
                {
                    // pack has to be done at mul pipe, so move result to r3, 
                    // then use mul pipe to move to final dst (with pack).
                    Vc4Register r3(VC4_QPU_ALU_R3, VC4_QPU_WADDR_ACC3);

                    {
                        Vc4Instruction Vc4Inst;
                        Vc4Inst.Vc4_a_FADD(r3, src[0], src[1]);
                        Vc4Inst.Emit(CurrentStorage);
                    }

                    {
                        Vc4Instruction Vc4Inst;
                        Vc4Inst.Vc4_m_MOV(dst, r3);
                        Vc4Inst.Vc4_m_Pack(pack);
                        Vc4Inst.Emit(CurrentStorage);
                    }
                }
                else
                {
                    Vc4Instruction Vc4Inst;
                    Vc4Inst.Vc4_a_FADD(dst, src[0], src[1]);
                    Vc4Inst.Emit(CurrentStorage);
                }
            }

            aCurrent <<= 1;
        }
    }

    { // Emit a NOP
        Vc4Instruction Vc4Inst;
        Vc4Inst.Emit(CurrentStorage);
    }
}

void Vc4Shader::Emit_Mul(CInstruction &Inst)
{
    assert(this->uShaderType == D3D10_SB_PIXEL_SHADER ||
           this->uShaderType == D3D10_SB_VERTEX_SHADER);
    
    assert(Inst.m_NumOperands == 3);

    // max 4 mul, includes 1 mul + 2 moves(s) for each, + 1 NOP.
    CurrentStorage->Ensure<VC4_QPU_INSTRUCTION>(13);

    {
        for (uint8_t i = 0, aCurrent = D3D10_SB_OPERAND_4_COMPONENT_MASK_X; i < 4; i++)
        {
            if (Inst.m_Operands[0].m_WriteMask & aCurrent)
            {
                Vc4Register dst = Find_Vc4Register_M(Inst.m_Operands[0], aCurrent);
                uint8_t pack = VC4_QPU_PACK_A_32;
                if (dst.GetFlags().packed)
                {
                    pack = VC4_QPU_PACK_MUL_8a + i;
                }

                Vc4Register src[2];
                this->Setup_SourceRegisters(Inst, i, src, ARRAYSIZE(src));

                {
                    Vc4Instruction Vc4Inst;
                    Vc4Inst.Vc4_m_FMUL(dst, src[0], src[1]);
                    Vc4Inst.Vc4_m_Pack(pack);
                    Vc4Inst.Emit(CurrentStorage);
                }
            }

            aCurrent <<= 1;
        }
    }

    { // Emit a NOP
        Vc4Instruction Vc4Inst;
        Vc4Inst.Emit(CurrentStorage);
    }
}

void Vc4Shader::Emit_Sample(CInstruction &Inst)
{
    assert(this->uShaderType == D3D10_SB_PIXEL_SHADER);
    
    assert(Inst.m_NumOperands == 4);

    boolean bUnpack = false;

    Vc4Register o[4];

    assert(Inst.m_Operands[0].m_NumComponents == D3D10_SB_OPERAND_4_COMPONENT);
    assert(Inst.m_Operands[0].m_IndexDimension == D3D10_SB_OPERAND_INDEX_1D);
    assert(Inst.m_Operands[0].m_IndexType[0] == D3D10_SB_OPERAND_INDEX_IMMEDIATE32);
    assert(Inst.m_Operands[0].m_ComponentSelection == D3D10_SB_OPERAND_4_COMPONENT_MASK_MODE);
    assert(Inst.m_Operands[0].m_WriteMask == (D3D10_SB_OPERAND_4_COMPONENT_MASK_R | D3D10_SB_OPERAND_4_COMPONENT_MASK_G | D3D10_SB_OPERAND_4_COMPONENT_MASK_B | D3D10_SB_OPERAND_4_COMPONENT_MASK_A));
    switch (Inst.m_Operands[0].m_Type)
    {
    case D3D10_SB_OPERAND_TYPE_OUTPUT:
        o[0] = Find_Vc4Register_M(Inst.m_Operands[0], (Inst.m_Operands[0].m_WriteMask & D3D10_SB_OPERAND_4_COMPONENT_MASK_MASK));
        assert(o[0].GetFlags().packed);
        break;
    case D3D10_SB_OPERAND_TYPE_TEMP:
        for (uint8_t i = 0, aCurrent = D3D10_SB_OPERAND_4_COMPONENT_MASK_X; i < 4; i++)
        {
            if (Inst.m_Operands[0].m_WriteMask & aCurrent)
            {
                o[i] = Find_Vc4Register_M(Inst.m_Operands[0], aCurrent);
            }
            aCurrent <<= 1;
        }
        bUnpack = true;
        break;
    default:
        assert(false);
    }

    // Resource
    assert(Inst.m_Operands[2].m_Type == D3D10_SB_OPERAND_TYPE_RESOURCE);
    assert(Inst.m_Operands[2].m_NumComponents == D3D10_SB_OPERAND_4_COMPONENT);
    assert(Inst.m_Operands[2].m_IndexDimension == D3D10_SB_OPERAND_INDEX_1D);
    assert(Inst.m_Operands[2].m_IndexType[0] == D3D10_SB_OPERAND_INDEX_IMMEDIATE32);
    uint32_t resourceIndex = Inst.m_Operands[2].m_Index[0].m_RegIndex;
    uint32_t texDimension = this->ResourceDimension[resourceIndex];

    // Texture coordinate
    assert(Inst.m_Operands[1].m_NumComponents == D3D10_SB_OPERAND_4_COMPONENT);
    assert(Inst.m_Operands[1].m_IndexDimension == D3D10_SB_OPERAND_INDEX_1D);
    assert(Inst.m_Operands[1].m_IndexType[0] == D3D10_SB_OPERAND_INDEX_IMMEDIATE32);
    assert(Inst.m_Operands[1].m_ComponentSelection == D3D10_SB_OPERAND_4_COMPONENT_SWIZZLE_MODE);

    Vc4Register s;
    Vc4Register t;
    Vc4Register r;

    switch (texDimension)
    {
    case D3D10_SB_RESOURCE_DIMENSION_TEXTURECUBE:
        r = Find_Vc4Register_M(Inst.m_Operands[1], D3D10_SB_OPERAND_4_COMPONENT_MASK(Inst.m_Operands[1].m_Swizzle[2]));
        __fallthrough;
    case D3D10_SB_RESOURCE_DIMENSION_TEXTURE2D:
        t = Find_Vc4Register_M(Inst.m_Operands[1], D3D10_SB_OPERAND_4_COMPONENT_MASK(Inst.m_Operands[1].m_Swizzle[1]));
        __fallthrough;
    case D3D10_SB_RESOURCE_DIMENSION_TEXTURE1D:
        s = Find_Vc4Register_M(Inst.m_Operands[1], D3D10_SB_OPERAND_4_COMPONENT_MASK(Inst.m_Operands[1].m_Swizzle[0]));
        break;
    case D3D10_SB_RESOURCE_DIMENSION_BUFFER:
    case D3D10_SB_RESOURCE_DIMENSION_TEXTURE3D:
    case D3D10_SB_RESOURCE_DIMENSION_TEXTURE2DMS:
    case D3D10_SB_RESOURCE_DIMENSION_TEXTURE1DARRAY:
    case D3D10_SB_RESOURCE_DIMENSION_TEXTURE2DARRAY:
    case D3D10_SB_RESOURCE_DIMENSION_TEXTURE2DMSARRAY:
    default:
        assert(false);
    }
    
    // Sampler
    assert(Inst.m_Operands[3].m_Type == D3D10_SB_OPERAND_TYPE_SAMPLER);
    assert(Inst.m_Operands[3].m_IndexDimension == D3D10_SB_OPERAND_INDEX_1D);
    assert(Inst.m_Operands[3].m_IndexType[0] == D3D10_SB_OPERAND_INDEX_IMMEDIATE32);
    uint32_t samplerIndex = Inst.m_Operands[3].m_Index[0].m_RegIndex;
               
    // + 3 or 2 or 1 to set texture coordinate.
    // + 1 for sample instruction, 'ldtmu0', 
    // + 1 or up to 4 for moving r4 to output register or unpacking r4 to other register(s).
    // + 1 for NOP.
    {
        uint8_t texDem = (r.GetFlags().valid && t.GetFlags().valid ? 3 : t.GetFlags().valid ? 2 : 1);
        CurrentStorage->Ensure<VC4_QPU_INSTRUCTION>(texDem + 1 + (bUnpack ? 4 : 1) + 1);
    }

    // texture address : z
    if (r.GetFlags().valid)
    {
        Vc4Instruction Vc4Inst;
        Vc4Register tmu0_r(VC4_QPU_ALU_REG_A, VC4_QPU_WADDR_TMU0_R);
        Vc4Inst.Vc4_a_MOV(tmu0_r, r);
        Vc4Inst.Emit(CurrentStorage);
    }

    // texture address : y
    if (t.GetFlags().valid)
    {
        Vc4Instruction Vc4Inst;
        Vc4Register tmu0_t(VC4_QPU_ALU_REG_A, VC4_QPU_WADDR_TMU0_T);
        Vc4Inst.Vc4_a_MOV(tmu0_t, t);
        Vc4Inst.Emit(CurrentStorage);
    }

    // texture address : x and must write 's' at last.
    assert(s.GetFlags().valid);
    {
        Vc4Instruction Vc4Inst;
        Vc4Register tmu0_s(VC4_QPU_ALU_REG_A, VC4_QPU_WADDR_TMU0_S);
        Vc4Inst.Vc4_a_MOV(tmu0_s, s);
        Vc4Inst.Emit(CurrentStorage);
    }

    // add uniform references.
    {
        {
            VC4_UNIFORM_FORMAT u;
            u.Type = VC4_UNIFORM_TYPE_SAMPLER_CONFIG_P0;
            u.sampilerConfiguration.samplerIndex = samplerIndex;
            u.sampilerConfiguration.resourceIndex = resourceIndex;
            this->AddUniformReference(u);
        }

        {
            VC4_UNIFORM_FORMAT u;
            u.Type = VC4_UNIFORM_TYPE_SAMPLER_CONFIG_P1;
            u.sampilerConfiguration.samplerIndex = samplerIndex;
            u.sampilerConfiguration.resourceIndex = resourceIndex;
            this->AddUniformReference(u);
        }

        if (r.GetFlags().valid) // only cube needs P2 config.
        {
            VC4_UNIFORM_FORMAT u;
            u.Type = VC4_UNIFORM_TYPE_SAMPLER_CONFIG_P2;
            u.sampilerConfiguration.samplerIndex = samplerIndex;
            u.sampilerConfiguration.resourceIndex = resourceIndex;
            this->AddUniformReference(u);
        }
    }

    // Sample texture, result come up in r4.
    {
        Vc4Instruction Vc4Inst;
        Vc4Inst.Vc4_Sig(VC4_QPU_SIG_LOAD_TMU0);
        Vc4Inst.Emit(CurrentStorage);
    }

    // TODO: Swizzle texture channel when texture format and RT format are different.
    Vc4Register r4(VC4_QPU_ALU_R4);

    // Move result at r4 to output register.
    if (Inst.m_Operands[0].m_Type == D3D10_SB_OPERAND_TYPE_OUTPUT)
    {
        Vc4Instruction Vc4Inst;
        Vc4Inst.Vc4_a_MOV(o[0], r4);
        Vc4Inst.Emit(CurrentStorage);
    }
    else if (Inst.m_Operands[0].m_ComponentSelection == D3D10_SB_OPERAND_4_COMPONENT_MASK_MODE)
    {
        assert(bUnpack);
        // Move each color channel at r4 to o[4].
        for (uint8_t i = 0; i < 4; i++)
        {
            if (o[i].GetFlags().valid)
            {
                Vc4Instruction Vc4Inst;
                Vc4Inst.Vc4_m_MOV(o[i], r4);
                Vc4Inst.Vc4_m_Unpack(VC4_QPU_UNPACK_8a + i, true); // Use R4 unpack.
                Vc4Inst.Emit(CurrentStorage);
            }
        }
    }
    else
    {
        assert(false);
    }

    { // Emit a NOP
        Vc4Instruction Vc4Inst;
        Vc4Inst.Emit(CurrentStorage);
    }
}

void Vc4Shader::HLSL_ParseDecl()
{
    assert(this->uShaderType == D3D10_SB_PIXEL_SHADER ||
           this->uShaderType == D3D10_SB_VERTEX_SHADER);

    boolean bDone = false;
    D3D10_SB_OPCODE_TYPE OpCode;
    while (HLSL_PeekShaderInstructionOpCode(OpCode) && !bDone)
    { 
        CInstruction Inst;
        switch (OpCode)
        {
        case D3D10_SB_OPCODE_DCL_RESOURCE:
            HLSL_GetShaderInstruction(Inst);
            assert(cResource < 8);
            this->ResourceDimension[cResource++] = Inst.m_ResourceDecl.SRVInfo.Dimension;
            break;
        case D3D10_SB_OPCODE_DCL_CONSTANT_BUFFER:
            HLSL_GetShaderInstruction(Inst);
            cConstants++;
            break;
        case D3D10_SB_OPCODE_DCL_SAMPLER:
            HLSL_GetShaderInstruction(Inst);
            cSampler++;
            break;
        case D3D10_SB_OPCODE_DCL_INPUT:
        case D3D10_SB_OPCODE_DCL_INPUT_PS:
            HLSL_GetShaderInstruction(Inst);
            if (Inst.m_OpCode == D3D10_SB_OPCODE_DCL_INPUT_PS)
            {
                assert(Inst.m_InputPSDecl.InterpolationMode == D3D10_SB_INTERPOLATION_LINEAR); // PS input must be linear.
            }
            assert(Inst.m_NumOperands == 1);
            assert(Inst.m_Operands[0].m_ComponentSelection == D3D10_SB_OPERAND_4_COMPONENT_MASK_MODE);
            assert(Inst.m_Operands[0].m_IndexDimension == D3D10_SB_OPERAND_INDEX_1D);
            assert(Inst.m_Operands[0].m_IndexType[0] == D3D10_SB_OPERAND_INDEX_IMMEDIATE32);
            assert(Inst.m_Operands[0].m_Index[0].m_RegIndex < 8);
            assert(Inst.m_Operands[0].m_WriteMask & D3D10_SB_OPERAND_4_COMPONENT_MASK_MASK);
            for (uint8_t i = 0, aCurrent = D3D10_SB_OPERAND_4_COMPONENT_MASK_X; i < 4; i++)
            {
                if (Inst.m_Operands[0].m_WriteMask & aCurrent)
                {
                    assert((ROS_VC4_INPUT_REGISTER_FILE_START + cInput) <= ROS_VC4_INPUT_REGISTER_FILE_END);
                    this->InputRegister[Inst.m_Operands[0].m_Index[0].m_RegIndex][i].flags.valid = true;
                    this->InputRegister[Inst.m_Operands[0].m_Index[0].m_RegIndex][i].flags.require_linear_conversion = (Inst.m_OpCode == D3D10_SB_OPCODE_DCL_INPUT_PS ? true : false);
                    this->InputRegister[Inst.m_Operands[0].m_Index[0].m_RegIndex][i].addr = ROS_VC4_INPUT_REGISTER_FILE_START + cInput++;
                    this->InputRegister[Inst.m_Operands[0].m_Index[0].m_RegIndex][i].mux = ROS_VC4_INPUT_REGISTER_FILE;
                    this->InputRegister[Inst.m_Operands[0].m_Index[0].m_RegIndex][i].swizzleMask = aCurrent;
                }
                aCurrent <<= 1;
            }
            break;
        case D3D10_SB_OPCODE_DCL_OUTPUT:
        case D3D10_SB_OPCODE_DCL_OUTPUT_SIV:
            HLSL_GetShaderInstruction(Inst);
            assert(Inst.m_NumOperands == 1);
            assert(Inst.m_Operands[0].m_ComponentSelection == D3D10_SB_OPERAND_4_COMPONENT_MASK_MODE);
            assert(Inst.m_Operands[0].m_IndexDimension == D3D10_SB_OPERAND_INDEX_1D);
            assert(Inst.m_Operands[0].m_IndexType[0] == D3D10_SB_OPERAND_INDEX_IMMEDIATE32);
            if (this->uShaderType == D3D10_SB_PIXEL_SHADER)
            {
                // only support single 8888RGBA colour output from pixel shader.
                assert(Inst.m_Operands[0].m_WriteMask == (D3D10_SB_OPERAND_4_COMPONENT_MASK_R | D3D10_SB_OPERAND_4_COMPONENT_MASK_G | D3D10_SB_OPERAND_4_COMPONENT_MASK_B | D3D10_SB_OPERAND_4_COMPONENT_MASK_A));
                assert(Inst.m_Operands[0].m_Index[0].m_RegIndex == 0);
                assert(cOutput == 0);
                this->OutputRegister[0][0].flags.valid = true;
                this->OutputRegister[0][0].flags.color = true;
                this->OutputRegister[0][0].flags.packed = true; // RGBA components are packed in single register (see above WriteMask assert).
                this->OutputRegister[0][0].addr = VC4_QPU_WADDR_ACC0;
                this->OutputRegister[0][0].mux = VC4_QPU_ALU_R0; // Pixel output is fixed at R0.
                this->OutputRegister[0][0].swizzleMask = (uint8_t)(Inst.m_Operands[0].m_WriteMask & D3D10_SB_OPERAND_4_COMPONENT_MASK_MASK);
                cOutput++;
            }
            else
            {
                assert(this->uShaderType == D3D10_SB_VERTEX_SHADER);
                assert(Inst.m_Operands[0].m_Index[0].m_RegIndex < 8);
                bool bPos;
                uint8_t aMask;
                if ((Inst.m_OpCode == D3D10_SB_OPCODE_DCL_OUTPUT_SIV) && (Inst.m_InputDeclSIV.Name == D3D10_SB_NAME_POSITION))
                {
                    bPos = true;
                    aMask = D3D10_SB_OPERAND_4_COMPONENT_MASK_MASK;
                }
                else
                {
                    bPos = false;
                    aMask = (Inst.m_Operands[0].m_WriteMask & D3D10_SB_OPERAND_4_COMPONENT_MASK_MASK);
                }
                assert(aMask);

                for (uint8_t i = 0, aCurrent = D3D10_SB_OPERAND_4_COMPONENT_MASK_X; i < 4; i++)
                {
                    if (aMask & aCurrent)
                    {
                        assert((ROS_VC4_OUTPUT_REGISTER_FILE_START + cOutput) <= ROS_VC4_OUTPUT_REGISTER_FILE_END);
                        this->OutputRegister[Inst.m_Operands[0].m_Index[0].m_RegIndex][i].flags.valid = true;
                        this->OutputRegister[Inst.m_Operands[0].m_Index[0].m_RegIndex][i].flags.position = bPos;
                        this->OutputRegister[Inst.m_Operands[0].m_Index[0].m_RegIndex][i].addr = ROS_VC4_OUTPUT_REGISTER_FILE_START + cOutput++;
                        this->OutputRegister[Inst.m_Operands[0].m_Index[0].m_RegIndex][i].mux = ROS_VC4_OUTPUT_REGISTER_FILE;
                        this->OutputRegister[Inst.m_Operands[0].m_Index[0].m_RegIndex][i].swizzleMask = aCurrent;
                    }
                    aCurrent <<= 1;
                }
            }
            break;
        case D3D10_SB_OPCODE_DCL_TEMPS:
            HLSL_GetShaderInstruction(Inst);
            // Temp register doesn't have swizzle mask, so assume all 4 components to be used.
            // TODO: AllocateRegister(); Currently temps are allocated at ra16~ra31.
            //       since currently reserve temp to ra16~31, so only upto 4 temps are allowed.
            assert(Inst.m_TempsDecl.NumTemps <= 4);
            for (uint8_t i = 0; i < Inst.m_TempsDecl.NumTemps * 4; i++)
            {
                assert((ROS_VC4_INPUT_REGISTER_FILE_START + cTemp) <= ROS_VC4_TEMP_REGISTER_FILE_END);
                this->TempRegister[i / 4][i % 4].flags.valid = true;
                this->TempRegister[i / 4][i % 4].flags.temp = true;
                this->TempRegister[i / 4][i % 4].addr = ROS_VC4_TEMP_REGISTER_FILE_START + cTemp++;
                this->TempRegister[i / 4][i % 4].mux = ROS_VC4_TEMP_REGISTER_FILE;
                this->TempRegister[i / 4][i % 4].swizzleMask = D3D10_SB_OPERAND_4_COMPONENT_MASK_X << (i % 4);
            }
            break;
        case D3D10_SB_OPCODE_DCL_INDEX_RANGE:
        case D3D10_SB_OPCODE_DCL_GS_OUTPUT_PRIMITIVE_TOPOLOGY:
        case D3D10_SB_OPCODE_DCL_GS_INPUT_PRIMITIVE:
        case D3D10_SB_OPCODE_DCL_MAX_OUTPUT_VERTEX_COUNT:
        case D3D10_SB_OPCODE_DCL_INPUT_SGV:
        case D3D10_SB_OPCODE_DCL_INPUT_SIV:
        case D3D10_SB_OPCODE_DCL_INPUT_PS_SGV:
        case D3D10_SB_OPCODE_DCL_INPUT_PS_SIV:
        case D3D10_SB_OPCODE_DCL_OUTPUT_SGV:
        case D3D10_SB_OPCODE_DCL_INDEXABLE_TEMP:
        case D3D10_SB_OPCODE_DCL_GLOBAL_FLAGS:
            HLSL_GetShaderInstruction(Inst);
            assert(false); // TODO
            __fallthrough;
        default:
            if (OpCode >= D3D10_SB_OPCODE_RESERVED0)
            {
                assert(false); // only 10.0 opcode is supported.
            }
            bDone = true;
        }
    }
}

HRESULT Vc4Shader::Translate_VS()
{
    assert(this->uShaderType == D3D10_SB_VERTEX_SHADER);

    this->SetCurrentStorage(this->ShaderStorage, this->ShaderUniform);
    this->HLSL_ParseDecl();
    this->Emit_Prologue_VS();

    {
        CInstruction Inst;
        while (HLSL_GetShaderInstruction(Inst))
        {
            switch (Inst.m_OpCode)
            {
            case D3D10_SB_OPCODE_ADD:
                this->Emit_Add(Inst);
                break;
            case D3D10_SB_OPCODE_MOV:
                this->Emit_Mov(Inst);
                break;
            case D3D10_SB_OPCODE_MUL:
                this->Emit_Mul(Inst);
                break;
            case D3D10_SB_OPCODE_RET:
                break;
            default:
                assert(false);
            }
        }
    }

    this->ShaderStorageAux->CopyFrom(*this->ShaderStorage); // Copy VS to CS.
    this->Emit_ShaderOutput_VS(true);  // VS
    this->Emit_Epilogue(); // VS

    this->SetCurrentStorage(this->ShaderStorageAux, this->ShaderUniformAux); // switch to CS storage.
    this->Emit_ShaderOutput_VS(false); // CS
    this->Emit_Epilogue(); // CS
    
    return S_OK;
}

HRESULT Vc4Shader::Translate_PS()
{
    assert(this->uShaderType == D3D10_SB_PIXEL_SHADER);

    this->SetCurrentStorage(this->ShaderStorage, this->ShaderUniform);
    this->HLSL_ParseDecl();
    this->Emit_Prologue_PS();

    {
        CInstruction Inst;
        while (HLSL_GetShaderInstruction(Inst))
        {
            switch (Inst.m_OpCode)
            {
            case D3D10_SB_OPCODE_ADD:
                this->Emit_Add(Inst);
                break;
            case D3D10_SB_OPCODE_MOV:
                this->Emit_Mov(Inst);
                break;
            case D3D10_SB_OPCODE_MUL:
                this->Emit_Mul(Inst);
                break;
            case D3D10_SB_OPCODE_RET:
                break;
            case D3D10_SB_OPCODE_SAMPLE:
                this->Emit_Sample(Inst);
                break;
            default:
                assert(false);
            }
        }
    }
        
    this->Emit_Epilogue();

    return S_OK;
}

