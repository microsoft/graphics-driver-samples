#include "precomp.h"
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

    VC4_ASSERT(cInput < 16); // VR_SETUP:NUM limitation, vpm only can read up to 16 values.

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
    
    for (uint8_t i = 0, iRegUsed = 0; iRegUsed < this->cInput; i++)
    {
        Vc4Register raX = this->InputRegister[i / 4][i % 4];

        if (raX.GetFlags().valid)
        {
            assert(raX.GetMux() == VC4_QPU_ALU_REG_A || raX.GetMux() == VC4_QPU_ALU_REG_B);

            Vc4Register r0(VC4_QPU_ALU_R0, VC4_QPU_WADDR_ACC0);

            // Issue mul inputs (from varying) with ra15 (W).
            {
                Vc4Instruction Vc4Inst;
                Vc4Register varying(VC4_QPU_ALU_REG_B, VC4_QPU_RADDR_VERYING);
                Vc4Register ra15(VC4_QPU_ALU_REG_A, 15);
                Vc4Inst.Vc4_m_FMUL(r0, varying, ra15);
                Vc4Inst.Emit(CurrentStorage);
            }

            // Issue add r5 to each input data to complete interpolation.
            {
                Vc4Instruction Vc4Inst;
                Vc4Register r5(VC4_QPU_ALU_R5);
                Vc4Inst.Vc4_a_FADD(raX, r0, r5);
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

    // calculate 1/W
    {
        // send W to sfu_recip.
        {
            Vc4Register sfu_recip((pos[3].GetMux() == VC4_QPU_ALU_REG_A ? VC4_QPU_ALU_REG_B : VC4_QPU_ALU_REG_A), VC4_QPU_WADDR_SFU_RECIP);
            Vc4Instruction Vc4Inst;
            Vc4Inst.Vc4_a_MOV(sfu_recip, pos[3]);
            Vc4Inst.Emit(CurrentStorage);
        }

        // Issue 2 NOPs to wait result of sfu_recip.
        for (uint8_t i = 0; i < 2; i++)
        {
            Vc4Instruction Vc4Inst;
            Vc4Inst.Emit(CurrentStorage);
        }
    }

    // Now, r4 is 1/W.
    Vc4Register r4(VC4_QPU_ALU_R4);
    
    // Ys/Xs
    {
        // scale by RT dimension (read from uniform). Result in r0/r1.
        {
            for (uint8_t i = 0; i < 2; i++)
            {
                Vc4Register rX(VC4_QPU_ALU_R0 + i, VC4_QPU_WADDR_ACC0 + i); // r0 and r1.
                
                { // divide Xc/Yc by W.
                    Vc4Instruction Vc4Inst;
                    Vc4Inst.Vc4_m_FMUL(rX, pos[i], r4);
                    Vc4Inst.Emit(CurrentStorage);
                }

                { // Scale Xc/Yc with viewport.
                    Vc4Instruction Vc4Inst;
                    Vc4Register unif((rX.GetMux() == VC4_QPU_ALU_REG_A ? VC4_QPU_ALU_REG_B : VC4_QPU_ALU_REG_A), VC4_QPU_RADDR_UNIFORM); // use opossit register file.
                    Vc4Inst.Vc4_m_FMUL(rX, rX, unif);
                    Vc4Inst.Emit(CurrentStorage);
                    {
                        VC4_UNIFORM_FORMAT u;
                        u.Type = (i == 0 ? VC4_UNIFORM_TYPE_VIEWPORT_SCALE_X : VC4_UNIFORM_TYPE_VIEWPORT_SCALE_Y);
                        this->AddUniformReference(u);
                    }
                }
            }
        }

        // Convert r0/r1 to 16bits float and pack them into ra16, then output to vpm.
        {
            Vc4Register ra16(VC4_QPU_ALU_REG_A, 16);

            for (uint8_t i = 0; i < 2; i++)
            {
                Vc4Register rX(VC4_QPU_ALU_R0 + i); // r0 and r1.
                Vc4Instruction Vc4Inst;
                Vc4Inst.Vc4_a_FTOI(ra16, rX); // REUSE ra16 as temp is no longer needed.
                Vc4Inst.Vc4_a_Pack(VC4_QPU_PACK_A_16a + i); // Pack to 16a or 16b.
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

    // Zs
    { // Zs = Zc / W // TODO: Z offset
        Vc4Instruction Vc4Inst;
        Vc4Inst.Vc4_m_FMUL(vpm, pos[2], r4);
        Vc4Inst.Emit(CurrentStorage);
    }

    // 1/Wc
    {
        // Move result of sfu_recip (come up at r4) to vpm.
        {
            Vc4Instruction Vc4Inst;
            Vc4Inst.Vc4_m_MOV(vpm, r4);
            Vc4Inst.Emit(CurrentStorage);
        }
    }

    // Only VS emits "varying" (everything read from vpm except position data).
    if (bVS)
    {
        for (uint8_t i = 0, iRegUsed = 0; iRegUsed < this->cOutput; i++ )
        {
            Vc4Register src = this->OutputRegister[i / 4][i % 4];
            if (src.GetFlags().valid)
            {
                if (src.GetFlags().linkage)
                {
                    Vc4Instruction Vc4Inst;
                    Vc4Inst.Vc4_m_MOV(vpm, src);
                    // Vc4Inst.Vc4_m_FMUL(vpm, src, r4);
                    Vc4Inst.Emit(CurrentStorage);
                }
                iRegUsed++;
            }
        }
    }
}

void Vc4Shader::Emit_Blending_PS()
{
    // Issue #29
    assert(false);
}

void Vc4Shader::Emit_Epilogue()
{
    assert(this->uShaderType == D3D10_SB_PIXEL_SHADER ||
           this->uShaderType == D3D10_SB_VERTEX_SHADER);

    if (D3D10_SB_PIXEL_SHADER == this->uShaderType)
    {
        // output 'Z'.
        if (UmdCompiler->GetDepthState()->DepthEnable)
        {
            Vc4Register tlb_z(VC4_QPU_ALU_REG_B, VC4_QPU_WADDR_TLB_Z);
            Vc4Register rb15(VC4_QPU_ALU_REG_B, 15); // reserved for Z
            Vc4Instruction Vc4Inst;
            Vc4Inst.Vc4_m_MOV(tlb_z, rb15);
            Vc4Inst.Emit(CurrentStorage);
        }

        // output blending instructions.
        if (UmdCompiler->GetBlendState()->RenderTarget[0].BlendEnable)
        {
            this->Emit_Blending_PS();
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

void Vc4Shader::Emit_Mad(CInstruction &Inst)
{
    assert(this->uShaderType == D3D10_SB_PIXEL_SHADER ||
           this->uShaderType == D3D10_SB_VERTEX_SHADER);

    VC4_ASSERT(Inst.m_NumOperands == 4);

    {
        for (uint8_t i = 0, aCurrent = D3D10_SB_OPERAND_4_COMPONENT_MASK_X; i < 4; i++)
        {
            if (Inst.m_Operands[0].m_WriteMask & aCurrent)
            {
                Vc4Register accum(VC4_QPU_ALU_R3, VC4_QPU_WADDR_ACC3);
                Vc4Register dst = Find_Vc4Register_M(Inst.m_Operands[0], aCurrent);

                // perform mul first 2 operands
                {
                    Vc4Register src[2];
                    this->Setup_SourceRegisters(Inst, 1, ARRAYSIZE(src), i, src);

                    {
                        Vc4Instruction Vc4Inst;
                        Vc4Inst.Vc4_m_FMUL(accum, src[0], src[1]);
                        Vc4Inst.Emit(CurrentStorage);
                    }
                }

                Vc4Register _dst;
                if (dst.GetFlags().packed)
                {
                    // pack has to be done at mul pipe, so result to r3, 
                    // then use mul pipe to move to final dst (with pack).
                    _dst = accum;
                }
                else
                {
                    _dst = dst;
                }

                // perform add with 3rd operand.
                {
                    Vc4Register src[1];
                    this->Setup_SourceRegisters(Inst, 3, ARRAYSIZE(src), i, src);

                    {
                        Vc4Instruction Vc4Inst;
                        Vc4Inst.Vc4_a_FADD(_dst, accum, src[0]);
                        Vc4Inst.Emit(CurrentStorage);
                    }
                }

                // move to destination (with packing).
                if (dst.GetFlags().packed)
                {
                    Vc4Instruction Vc4Inst;
                    Vc4Inst.Vc4_m_MOV(dst, accum);
                    Vc4Inst.Vc4_m_Pack(dst.GetPack(i));
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

void Vc4Shader::Emit_Mov(CInstruction &Inst)
{
    assert(this->uShaderType == D3D10_SB_PIXEL_SHADER ||
           this->uShaderType == D3D10_SB_VERTEX_SHADER);

    VC4_ASSERT(Inst.m_NumOperands == 2);

    {
        for (uint8_t i = 0, aCurrent = D3D10_SB_OPERAND_4_COMPONENT_MASK_X; i < 4; i++)
        {
            if (Inst.m_Operands[0].m_WriteMask & aCurrent)
            {
                Vc4Register dst = Find_Vc4Register_M(Inst.m_Operands[0], aCurrent);
                Vc4Register src[1];
                Setup_SourceRegisters(Inst, 1, ARRAYSIZE(src), i , src);

                {
                    Vc4Instruction Vc4Inst;
                    Vc4Inst.Vc4_m_MOV(dst, src[0]);
                    Vc4Inst.Vc4_m_Pack(dst.GetPack(i));
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

void Vc4Shader::Emit_DPx(CInstruction &Inst)
{
    assert(this->uShaderType == D3D10_SB_PIXEL_SHADER ||
           this->uShaderType == D3D10_SB_VERTEX_SHADER); 

    VC4_ASSERT(Inst.m_NumOperands == 3);

    {
        // DP2 loop 2 times.
        // DP3 loop 3 times.
        // DP4 loop 4 times.
        uint8_t c = (uint8_t)(Inst.m_OpCode - 13);

        // where to accumulate result of mul.
        Vc4Register accum(VC4_QPU_ALU_R3, VC4_QPU_WADDR_ACC3);

        {
            Vc4Register zero(VC4_QPU_ALU_REG_B, 0); // 0 as small immediate in raddr_b
            Vc4Instruction Vc4Inst(vc4_alu_small_immediate);
            Vc4Inst.Vc4_m_MOV(accum, zero);
            Vc4Inst.Emit(CurrentStorage);
        }
           
        for(uint8_t i = 0; i < c; i++)
        {
            Vc4Register temp(VC4_QPU_ALU_R1, VC4_QPU_WADDR_ACC1);
            Vc4Register src[2];
            Setup_SourceRegisters(Inst, 1, ARRAYSIZE(src), i, src);

            {
                Vc4Instruction Vc4Inst;
                Vc4Inst.Vc4_m_FMUL(temp, src[0], src[1]);
                if (i > 0)
                {
                    Vc4Inst.Vc4_a_FADD(accum, accum, temp);
                }
                Vc4Inst.Emit(CurrentStorage);
            }

            if (i+1 == c)
            {
                Vc4Instruction Vc4Inst;
                Vc4Inst.Vc4_a_FADD(accum, accum, temp);
                Vc4Inst.Emit(CurrentStorage);
            }
        }

        // replicate ouput where specified.
        for (uint8_t i = 0, aCurrent = D3D10_SB_OPERAND_4_COMPONENT_MASK_X; i < 4; i++)
        {
            if (Inst.m_Operands[0].m_WriteMask & aCurrent)
            {
                Vc4Register dst = Find_Vc4Register_M(Inst.m_Operands[0], aCurrent);
                Vc4Instruction Vc4Inst;
                Vc4Inst.Vc4_m_MOV(dst, accum);
                Vc4Inst.Vc4_m_Pack(dst.GetPack(i));
                Vc4Inst.Emit(CurrentStorage);
            }
     
            aCurrent <<= 1;
        }
    }

    { // Emit a NOP
        Vc4Instruction Vc4Inst;
        Vc4Inst.Emit(CurrentStorage);
    }
}

void Vc4Shader::Emit_with_Add_pipe(CInstruction &Inst)
{
    assert(this->uShaderType == D3D10_SB_PIXEL_SHADER ||
           this->uShaderType == D3D10_SB_VERTEX_SHADER);

    VC4_ASSERT(Inst.m_NumOperands == 3);

    {
        for (uint8_t i = 0, aCurrent = D3D10_SB_OPERAND_4_COMPONENT_MASK_X; i < 4; i++)
        {
            if (Inst.m_Operands[0].m_WriteMask & aCurrent)
            {
                Vc4Register dst = Find_Vc4Register_M(Inst.m_Operands[0], aCurrent);

                Vc4Register src[2];
                this->Setup_SourceRegisters(Inst, 1, ARRAYSIZE(src), i, src);

                Vc4Register _dst;
                if (dst.GetFlags().packed)
                {
                    // pack has to be done at mul pipe, so result to r3, 
                    // then use mul pipe to move to final dst (with pack).
                    Vc4Register r3(VC4_QPU_ALU_R3, VC4_QPU_WADDR_ACC3);
                    _dst = r3;
                }
                else
                {
                    _dst = dst;
                }
                
                {
                    Vc4Instruction Vc4Inst;
                    switch (Inst.m_OpCode)
                    {
                    case D3D10_SB_OPCODE_ADD:
                        Vc4Inst.Vc4_a_FADD(_dst, src[0], src[1]);
                        break;
                    case D3D10_SB_OPCODE_MAX:
                        Vc4Inst.Vc4_a_FMAX(_dst, src[0], src[1]);
                        break;
                    case D3D10_SB_OPCODE_MIN:
                        Vc4Inst.Vc4_a_FMIN(_dst, src[0], src[1]);
                        break;
                    case D3D10_SB_OPCODE_IADD:
                        Vc4Inst.Vc4_a_IADD(_dst, src[0], src[1]);
                        break;
                    default:
                        VC4_ASSERT(false);
                    }
                    Vc4Inst.Emit(CurrentStorage);
                }

                if (dst.GetFlags().packed)
                {
                    Vc4Instruction Vc4Inst;
                    Vc4Inst.Vc4_m_MOV(dst, _dst);
                    Vc4Inst.Vc4_m_Pack(dst.GetPack(i));
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

void Vc4Shader::Emit_with_Mul_pipe(CInstruction &Inst)
{
    assert(this->uShaderType == D3D10_SB_PIXEL_SHADER ||
           this->uShaderType == D3D10_SB_VERTEX_SHADER);
    
    VC4_ASSERT(Inst.m_NumOperands == 3);

    {
        for (uint8_t i = 0, aCurrent = D3D10_SB_OPERAND_4_COMPONENT_MASK_X; i < 4; i++)
        {
            if (Inst.m_Operands[0].m_WriteMask & aCurrent)
            {
                Vc4Register dst = Find_Vc4Register_M(Inst.m_Operands[0], aCurrent);

                Vc4Register src[2];
                this->Setup_SourceRegisters(Inst, 1, ARRAYSIZE(src), i, src);

                {
                    Vc4Instruction Vc4Inst;
                    switch (Inst.m_OpCode)
                    {
                    case D3D10_SB_OPCODE_MUL:
                        Vc4Inst.Vc4_m_FMUL(dst, src[0], src[1]);
                        break;
                    default:
                        VC4_ASSERT(false);
                    }
                    Vc4Inst.Vc4_m_Pack(dst.GetPack(i));
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
    
    VC4_ASSERT(Inst.m_NumOperands == 4);

    boolean bUnpack = false;

    Vc4Register o[4];

    VC4_ASSERT(Inst.m_Operands[0].m_NumComponents == D3D10_SB_OPERAND_4_COMPONENT);
    VC4_ASSERT(Inst.m_Operands[0].m_IndexDimension == D3D10_SB_OPERAND_INDEX_1D);
    VC4_ASSERT(Inst.m_Operands[0].m_IndexType[0] == D3D10_SB_OPERAND_INDEX_IMMEDIATE32);
    VC4_ASSERT(Inst.m_Operands[0].m_ComponentSelection == D3D10_SB_OPERAND_4_COMPONENT_MASK_MODE);
    VC4_ASSERT(Inst.m_Operands[0].m_WriteMask == (D3D10_SB_OPERAND_4_COMPONENT_MASK_R | D3D10_SB_OPERAND_4_COMPONENT_MASK_G | D3D10_SB_OPERAND_4_COMPONENT_MASK_B | D3D10_SB_OPERAND_4_COMPONENT_MASK_A));
    switch (Inst.m_Operands[0].m_Type)
    {
    case D3D10_SB_OPERAND_TYPE_OUTPUT:
        o[0] = Find_Vc4Register_M(Inst.m_Operands[0], (Inst.m_Operands[0].m_WriteMask & D3D10_SB_OPERAND_4_COMPONENT_MASK_MASK));
        VC4_ASSERT(o[0].GetFlags().packed);
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
        VC4_ASSERT(false);
    }

    // Resource
    VC4_ASSERT(Inst.m_Operands[2].m_Type == D3D10_SB_OPERAND_TYPE_RESOURCE);
    VC4_ASSERT(Inst.m_Operands[2].m_NumComponents == D3D10_SB_OPERAND_4_COMPONENT);
    VC4_ASSERT(Inst.m_Operands[2].m_IndexDimension == D3D10_SB_OPERAND_INDEX_1D);
    VC4_ASSERT(Inst.m_Operands[2].m_IndexType[0] == D3D10_SB_OPERAND_INDEX_IMMEDIATE32);
    uint32_t resourceIndex = Inst.m_Operands[2].m_Index[0].m_RegIndex;
    uint32_t texDimension = this->ResourceDimension[resourceIndex];

    DXGI_FORMAT texFormat = UmdCompiler->GetShaderResourceFormat((uint8_t)resourceIndex);
    VC4_ASSERT((texFormat == DXGI_FORMAT_B8G8R8A8_UNORM) || (texFormat == DXGI_FORMAT_R8G8B8A8_UNORM));
        
    // TODO: more generic color channel swizzle support.
    boolean bSwapColorChannel = (texFormat != DXGI_FORMAT_R8G8B8A8_UNORM);
    
    // Texture coordinate
    VC4_ASSERT(Inst.m_Operands[1].m_NumComponents == D3D10_SB_OPERAND_4_COMPONENT);
    VC4_ASSERT(Inst.m_Operands[1].m_IndexDimension == D3D10_SB_OPERAND_INDEX_1D);
    VC4_ASSERT(Inst.m_Operands[1].m_IndexType[0] == D3D10_SB_OPERAND_INDEX_IMMEDIATE32);
    VC4_ASSERT(Inst.m_Operands[1].m_ComponentSelection == D3D10_SB_OPERAND_4_COMPONENT_SWIZZLE_MODE);

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
    VC4_ASSERT(Inst.m_Operands[3].m_Type == D3D10_SB_OPERAND_TYPE_SAMPLER);
    VC4_ASSERT(Inst.m_Operands[3].m_IndexDimension == D3D10_SB_OPERAND_INDEX_1D);
    VC4_ASSERT(Inst.m_Operands[3].m_IndexType[0] == D3D10_SB_OPERAND_INDEX_IMMEDIATE32);
    uint32_t samplerIndex = Inst.m_Operands[3].m_Index[0].m_RegIndex;
               
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
            u.samplerConfiguration.samplerIndex = samplerIndex;
            u.samplerConfiguration.resourceIndex = resourceIndex;
            this->AddUniformReference(u);
        }

        {
            VC4_UNIFORM_FORMAT u;
            u.Type = VC4_UNIFORM_TYPE_SAMPLER_CONFIG_P1;
            u.samplerConfiguration.samplerIndex = samplerIndex;
            u.samplerConfiguration.resourceIndex = resourceIndex;
            this->AddUniformReference(u);
        }

        if (r.GetFlags().valid) // only cube needs P2 config.
        {
            VC4_UNIFORM_FORMAT u;
            u.Type = VC4_UNIFORM_TYPE_SAMPLER_CONFIG_P2;
            u.samplerConfiguration.samplerIndex = samplerIndex;
            u.samplerConfiguration.resourceIndex = resourceIndex;
            this->AddUniformReference(u);
        }
    }

    // Sample texture, result come up in r4.
    {
        Vc4Instruction Vc4Inst;
        Vc4Inst.Vc4_Sig(VC4_QPU_SIG_LOAD_TMU0);
        Vc4Inst.Emit(CurrentStorage);
    }

    // Sample result is now at r4.
    Vc4Register r4(VC4_QPU_ALU_R4);

    // Move result at r4 to output register.
    if (Inst.m_Operands[0].m_Type == D3D10_SB_OPERAND_TYPE_OUTPUT)
    {
        if (bSwapColorChannel == o[0].GetFlags().swap_color_channel)
        {
            Vc4Instruction Vc4Inst;
            Vc4Inst.Vc4_a_MOV(o[0], r4);
            Vc4Inst.Emit(CurrentStorage);
        }
        else
        {
            // R, G, B channel
            for (uint8_t i = 0; i < 3; i++)
            {
                Vc4Instruction Vc4Inst;
                Vc4Inst.Vc4_m_MOV(o[0], r4);
                Vc4Inst.Vc4_m_Pack(VC4_QPU_PACK_MUL_8c - i);
                Vc4Inst.Vc4_m_Unpack(VC4_QPU_UNPACK_8a + i, true); // Use R4 unpack.
                Vc4Inst.Emit(CurrentStorage);
            }

            // A channel
            {
                Vc4Instruction Vc4Inst;
                Vc4Inst.Vc4_m_MOV(o[0], r4);
                Vc4Inst.Vc4_m_Pack(VC4_QPU_PACK_MUL_8d);
                Vc4Inst.Vc4_m_Unpack(VC4_QPU_UNPACK_8d, true); // Use R4 unpack.
                Vc4Inst.Emit(CurrentStorage);
            }
        }
    }
    else
    {
        // Move each color channel at r4 to o[i].
        // R, G, B channel
        for (uint8_t i = 0; i < 3; i++)
        {
            Vc4Register out = bSwapColorChannel ? o[2 - i] : o[i];
            if (out.GetFlags().valid)
            {
                Vc4Instruction Vc4Inst;
                Vc4Inst.Vc4_m_MOV(out, r4);
                Vc4Inst.Vc4_m_Unpack(VC4_QPU_UNPACK_8a + i, true); // Use R4 unpack.
                Vc4Inst.Emit(CurrentStorage);
            }
        }

        // A channel
        if (o[3].GetFlags().valid)
        {
            Vc4Instruction Vc4Inst;
            Vc4Inst.Vc4_m_MOV(o[3], r4);
            Vc4Inst.Vc4_m_Unpack(VC4_QPU_UNPACK_8d, true); // Use R4 unpack.
            Vc4Inst.Emit(CurrentStorage);
        }
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
    assert(this->HLSLParser.IsValid());
    
    boolean bDone = false;
    D3D10_SB_OPCODE_TYPE OpCode;
    while (HLSL_PeekShaderInstructionOpCode(this->HLSLParser, OpCode) && !bDone)
    { 
        CInstruction Inst;
        switch (OpCode)
        {
        case D3D10_SB_OPCODE_DCL_RESOURCE:
            HLSL_GetShaderInstruction(this->HLSLParser, Inst);
            VC4_ASSERT(Inst.m_Operands[0].m_Index[0].m_RegIndex < ARRAYSIZE(this->ResourceDimension));
            this->ResourceDimension[Inst.m_Operands[0].m_Index[0].m_RegIndex] = Inst.m_ResourceDecl.SRVInfo.Dimension;
            cResources++;
            break;
        case D3D10_SB_OPCODE_DCL_CONSTANT_BUFFER:
            HLSL_GetShaderInstruction(this->HLSLParser, Inst);
            // Issue 37: VC4 Find_Vc4Register_I needs to implement dynamic index support for constant buffers
            VC4_ASSERT(Inst.m_ResourceDecl.CBInfo.AccessPattern == D3D10_SB_CONSTANT_BUFFER_IMMEDIATE_INDEXED);
            cConstants++;
            break;
        case D3D10_SB_OPCODE_DCL_SAMPLER:
            HLSL_GetShaderInstruction(this->HLSLParser, Inst);
            cSampler++;
            break;
        case D3D10_SB_OPCODE_DCL_INPUT:
        case D3D10_SB_OPCODE_DCL_INPUT_PS:
            HLSL_GetShaderInstruction(this->HLSLParser, Inst);
            if (Inst.m_OpCode == D3D10_SB_OPCODE_DCL_INPUT_PS)
            {
                VC4_ASSERT(Inst.m_InputPSDecl.InterpolationMode == D3D10_SB_INTERPOLATION_LINEAR); // PS input must be linear.
            }
            VC4_ASSERT(Inst.m_NumOperands == 1);
            VC4_ASSERT(Inst.m_Operands[0].m_ComponentSelection == D3D10_SB_OPERAND_4_COMPONENT_MASK_MODE);
            VC4_ASSERT(Inst.m_Operands[0].m_IndexDimension == D3D10_SB_OPERAND_INDEX_1D);
            VC4_ASSERT(Inst.m_Operands[0].m_IndexType[0] == D3D10_SB_OPERAND_INDEX_IMMEDIATE32);
            VC4_ASSERT(Inst.m_Operands[0].m_Index[0].m_RegIndex < 8);
            VC4_ASSERT(Inst.m_Operands[0].m_WriteMask & D3D10_SB_OPERAND_4_COMPONENT_MASK_MASK);

            for (uint8_t i = 0, aCurrent = D3D10_SB_OPERAND_4_COMPONENT_MASK_X; i < 4; i++)
            {
                if (Inst.m_Operands[0].m_WriteMask & aCurrent)
                {
                    VC4_ASSERT((ROS_VC4_INPUT_REGISTER_FILE_START + cInput) <= ROS_VC4_INPUT_REGISTER_FILE_END);
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
            HLSL_GetShaderInstruction(this->HLSLParser, Inst);
            VC4_ASSERT(Inst.m_NumOperands == 1);
            VC4_ASSERT(Inst.m_Operands[0].m_ComponentSelection == D3D10_SB_OPERAND_4_COMPONENT_MASK_MODE);
            VC4_ASSERT(Inst.m_Operands[0].m_IndexDimension == D3D10_SB_OPERAND_INDEX_1D);
            VC4_ASSERT(Inst.m_Operands[0].m_IndexType[0] == D3D10_SB_OPERAND_INDEX_IMMEDIATE32);
            if (this->uShaderType == D3D10_SB_PIXEL_SHADER)
            {
                // only support single 8888RGBA colour output from pixel shader.
                VC4_ASSERT(Inst.m_Operands[0].m_WriteMask == (D3D10_SB_OPERAND_4_COMPONENT_MASK_R | D3D10_SB_OPERAND_4_COMPONENT_MASK_G | D3D10_SB_OPERAND_4_COMPONENT_MASK_B | D3D10_SB_OPERAND_4_COMPONENT_MASK_A));
                VC4_ASSERT(Inst.m_Operands[0].m_Index[0].m_RegIndex == 0);
                VC4_ASSERT(cOutput == 0);
                this->OutputRegister[0][0].flags.valid = true;
                this->OutputRegister[0][0].flags.color = true;
                this->OutputRegister[0][0].flags.packed = true; // RGBA components are packed in single register (see above WriteMask assert).
                this->OutputRegister[0][0].addr = ROS_VC4_OUTPUT_REGISTER_FILE_START;
                this->OutputRegister[0][0].mux = ROS_VC4_OUTPUT_REGISTER_FILE;
                this->OutputRegister[0][0].swizzleMask = (uint8_t)(Inst.m_Operands[0].m_WriteMask & D3D10_SB_OPERAND_4_COMPONENT_MASK_MASK);
                // TODO: more generic color channel swizzle support.
                DXGI_FORMAT texFormat = UmdCompiler->GetRenderTargetFormat(0);
                VC4_ASSERT((texFormat == DXGI_FORMAT_B8G8R8A8_UNORM) || (texFormat == DXGI_FORMAT_R8G8B8A8_UNORM));
                this->OutputRegister[0][0].flags.swap_color_channel = (texFormat != DXGI_FORMAT_R8G8B8A8_UNORM);
                cOutput++;
            }
            else
            {
                VC4_ASSERT(this->uShaderType == D3D10_SB_VERTEX_SHADER);
                VC4_ASSERT(Inst.m_Operands[0].m_Index[0].m_RegIndex < 8);
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
                    VC4_ASSERT(aMask);
                }

                for (uint8_t i = 0, aCurrent = D3D10_SB_OPERAND_4_COMPONENT_MASK_X; i < 4; i++)
                {
                    if (aMask & aCurrent)
                    {
                        VC4_ASSERT((ROS_VC4_OUTPUT_REGISTER_FILE_START + cOutput) <= ROS_VC4_OUTPUT_REGISTER_FILE_END);
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
            HLSL_GetShaderInstruction(this->HLSLParser, Inst);
            // Temp register doesn't have swizzle mask, so assume all 4 components to be used.
            // TODO: AllocateRegister(); Currently temps are allocated at ra16~ra31.
            //       since currently reserve temp to ra16~31, so only upto 4 temps are allowed.
            VC4_ASSERT(Inst.m_TempsDecl.NumTemps <= 4);
            for (uint8_t i = 0; i < Inst.m_TempsDecl.NumTemps * 4; i++)
            {
                VC4_ASSERT((ROS_VC4_TEMP_REGISTER_FILE_START + cTemp) <= ROS_VC4_TEMP_REGISTER_FILE_END);
                this->TempRegister[i / 4][i % 4].flags.valid = true;
                this->TempRegister[i / 4][i % 4].flags.temp = true;
                this->TempRegister[i / 4][i % 4].addr = ROS_VC4_TEMP_REGISTER_FILE_START + cTemp++;
                this->TempRegister[i / 4][i % 4].mux = ROS_VC4_TEMP_REGISTER_FILE;
                this->TempRegister[i / 4][i % 4].swizzleMask = D3D10_SB_OPERAND_4_COMPONENT_MASK_X << (i % 4);
            }
            break;
        case D3D10_SB_OPCODE_DCL_GLOBAL_FLAGS:
            HLSL_GetShaderInstruction(this->HLSLParser, Inst);
            // TODO:
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
            HLSL_GetShaderInstruction(this->HLSLParser, Inst);
            VC4_ASSERT(false); // TODO
            __fallthrough;
        default:
            if (OpCode >= D3D10_SB_OPCODE_RESERVED0)
            {
                VC4_ASSERT(false); // only 10.0 opcode is supported.
            }
            bDone = true;
        }
    }
}

void Vc4Shader::HLSL_Link_PS()
{
    assert(this->uShaderType == D3D10_SB_VERTEX_SHADER);
    assert(this->HLSLDownstreamParser.IsValid());

    boolean bDone = false, bFound = false;
    CInstruction Inst;
    while (HLSL_GetShaderInstruction(this->HLSLDownstreamParser, Inst) && !bDone)
    {
        switch (Inst.m_OpCode)
        {
        case D3D10_SB_OPCODE_DCL_INPUT_PS:
            VC4_ASSERT(Inst.m_InputPSDecl.InterpolationMode == D3D10_SB_INTERPOLATION_LINEAR); // PS input must be linear.
            VC4_ASSERT(Inst.m_NumOperands == 1);
            VC4_ASSERT(Inst.m_Operands[0].m_ComponentSelection == D3D10_SB_OPERAND_4_COMPONENT_MASK_MODE);
            VC4_ASSERT(Inst.m_Operands[0].m_IndexDimension == D3D10_SB_OPERAND_INDEX_1D);
            VC4_ASSERT(Inst.m_Operands[0].m_IndexType[0] == D3D10_SB_OPERAND_INDEX_IMMEDIATE32);
            VC4_ASSERT(Inst.m_Operands[0].m_Index[0].m_RegIndex < 8);
            VC4_ASSERT(Inst.m_Operands[0].m_WriteMask & D3D10_SB_OPERAND_4_COMPONENT_MASK_MASK);

            for (uint8_t i = 0, aCurrent = D3D10_SB_OPERAND_4_COMPONENT_MASK_X; i < 4; i++)
            {
                if (Inst.m_Operands[0].m_WriteMask & aCurrent)
                {
                    VC4_ASSERT(this->OutputRegister[Inst.m_Operands[0].m_Index[0].m_RegIndex][i].flags.valid);
                    this->OutputRegister[Inst.m_Operands[0].m_Index[0].m_RegIndex][i].flags.linkage = true;
                }
                aCurrent <<= 1;
            }
            bFound = true;

            break;
        default:
            if (bFound)
            {
                bDone = true;
            }
        }
    }
}

HRESULT Vc4Shader::Translate_VS()
{
    assert(this->uShaderType == D3D10_SB_VERTEX_SHADER);

    this->SetCurrentStorage(this->ShaderStorage, this->ShaderUniform);
    this->HLSL_ParseDecl();
    this->HLSL_Link_PS();  
    this->Emit_Prologue_VS();

    {
        CInstruction Inst;
        assert(Inst.m_bSaturate == false); // saturate is not supported.
        while (HLSL_GetShaderInstruction(this->HLSLParser, Inst))
        {
            // Need to add support for D3D10_SB_OPCODE_IADD - Issue #38
            switch (Inst.m_OpCode)
            {
            case D3D10_SB_OPCODE_ADD:
            case D3D10_SB_OPCODE_MAX:
            case D3D10_SB_OPCODE_MIN:
            case D3D10_SB_OPCODE_IADD:
                this->Emit_with_Add_pipe(Inst);
                break;
            case D3D10_SB_OPCODE_DP2:
            case D3D10_SB_OPCODE_DP3:
            case D3D10_SB_OPCODE_DP4:
                this->Emit_DPx(Inst);
                break;
            case D3D10_SB_OPCODE_MAD:
                this->Emit_Mad(Inst);
                break;
            case D3D10_SB_OPCODE_MOV:
                this->Emit_Mov(Inst);
                break;
            case D3D10_SB_OPCODE_MUL:
                this->Emit_with_Mul_pipe(Inst);
                break;
            case D3D10_SB_OPCODE_RET:
                break;
            default:
                VC4_ASSERT(false);
            }
        }
    }

    this->ShaderStorageAux->CopyFrom(*this->ShaderStorage); // Copy VS to CS.
    this->ShaderUniformAux->CopyFrom(*this->ShaderUniform); // Copy VS uniform to CS.

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
        assert(Inst.m_bSaturate == false); // saturate is not supported.
        while (HLSL_GetShaderInstruction(this->HLSLParser, Inst))
        {
            switch (Inst.m_OpCode)
            {
            case D3D10_SB_OPCODE_ADD:
            case D3D10_SB_OPCODE_MAX:
            case D3D10_SB_OPCODE_MIN:
                this->Emit_with_Add_pipe(Inst);
                break;
            case D3D10_SB_OPCODE_DP2:
            case D3D10_SB_OPCODE_DP3:
            case D3D10_SB_OPCODE_DP4:
                this->Emit_DPx(Inst);
                break;
            case D3D10_SB_OPCODE_MAD:
                this->Emit_Mad(Inst);
                break;
            case D3D10_SB_OPCODE_MOV:
                this->Emit_Mov(Inst);
                break;
            case D3D10_SB_OPCODE_MUL:
                this->Emit_with_Mul_pipe(Inst);
                break;
            case D3D10_SB_OPCODE_RET:
                break;
            case D3D10_SB_OPCODE_SAMPLE:
                this->Emit_Sample(Inst);
                break;
            default:
                VC4_ASSERT(false);
            }
        }
    }
        
    this->Emit_Epilogue();

    return S_OK;
}

