#include "roscompiler.h"

HRESULT Vc4ShaderStorage::Initialize()
{
    this->pStream = new VC4_QPU_INSTRUCTION[VC4_DEFAULT_STORAGE_SIZE];
    if (this->pStream == NULL)
    {
        return E_OUTOFMEMORY;
    }
    this->cStream = VC4_DEFAULT_STORAGE_SIZE;
    this->pCurrent = this->pStream;
    this->cUsed = 0;

    return S_OK;
}

HRESULT Vc4ShaderStorage::Grow(uint32_t size)
{
    uint32_t NewSize = this->cStream + max(size, VC4_DEFAULT_STORAGE_INCREMENT);
    VC4_QPU_INSTRUCTION *pNew = new VC4_QPU_INSTRUCTION[NewSize];
    if (pNew == NULL)
    {
        return E_OUTOFMEMORY;
    }
    memcpy(pNew, this->pStream, this->cStream * sizeof(VC4_QPU_INSTRUCTION));
    delete[] this->pStream;

    this->pStream = pNew;
    this->pCurrent = this->pStream + this->cUsed;
    this->cStream = NewSize;

    return S_OK;
}

HRESULT Vc4ShaderStorage::Emit(VC4_QPU_INSTRUCTION Instruction)
{
    HRESULT hr = this->Ensure(1);
    if (SUCCEEDED(hr))
    {
        this->Store(Instruction);
    }
    return hr;
}

void Vc4Shader::Emit_Prologue_VS()
{
    this->ShaderStorage.Ensure(this->cInput + 3); // +2 for vr_setup/vw_setup and NOP.

    assert(cInput < 16); // VR_SETUP:NUM limitation, vpm only can read up to 16 values.

    {
        Vc4Instruction Vc4Inst(vc4_load_immediate_32);
        Vc4Register vr_setup(VC4_QPU_ALU_REG_A, VC4_QPU_WADDR_VPMVCD_RD_SETUP);
        Vc4Value value; value.i = MAKE_VR_SETUP(cInput, 1, true, false, VC4_QPU_32BIT_VECTOR, 0);
        Vc4Inst.Vc4_a_LOAD32(vr_setup, value);
        ShaderStorage.Emit(Vc4Inst.GetInstruction());
    }

    {
        Vc4Instruction Vc4Inst(vc4_load_immediate_32);
        Vc4Register vw_setup(VC4_QPU_ALU_REG_B, VC4_QPU_WADDR_VPMVCD_WR_SETUP);
        Vc4Value value; value.i = MAKE_VW_SETUP(1, true, false, VC4_QPU_32BIT_VECTOR, 0);
        Vc4Inst.Vc4_a_LOAD32(vw_setup, value);
        ShaderStorage.Emit(Vc4Inst.GetInstruction());
    }

    for (uint8_t iRegUsed = 0, iRegIndex = 0; iRegUsed < this->cInput; iRegIndex++)
    {
        Vc4Instruction Vc4Inst;
        Vc4Register raX = this->InputRegister[iRegIndex / 4][iRegIndex % 4];
        if (raX.flags.valid)
        {
            assert(raX.mux == VC4_QPU_ALU_REG_A || raX.mux == VC4_QPU_ALU_REG_B);
            Vc4Register vpm(raX.mux, VC4_QPU_RADDR_VPM);
            Vc4Inst.Vc4_a_MOV(raX, vpm);
            ShaderStorage.Emit(Vc4Inst.GetInstruction());
            iRegUsed++;
        }
    }

    { // Emit a NOP
        Vc4Instruction Vc4Inst;
        ShaderStorage.Emit(Vc4Inst.GetInstruction());
    }
}

void Vc4Shader::Emit_Prologue_PS()
{
    // assume all requires interpolation. 1 instruction for load, 1 instruction for interpolation, and 1 instruction to store back.
    // +1 for NOP (after read all varying).
    // +1 for sbwait (after interporation).
    this->ShaderStorage.Ensure((this->cInput * 3) + 1 + 1);

    // Issue load inputs from varying.
    for (uint8_t iRegUsed = 0, iRegIndex = 0; iRegUsed < this->cInput; iRegIndex++)
    {
        Vc4Register raX = this->InputRegister[iRegIndex / 4][iRegIndex % 4];

        if (raX.flags.valid)
        {
            Vc4Instruction Vc4Inst;
            assert(raX.mux == VC4_QPU_ALU_REG_A || raX.mux == VC4_QPU_ALU_REG_B);
            Vc4Register varying(raX.mux, VC4_QPU_RADDR_VERYING);
            Vc4Inst.Vc4_a_MOV(raX, varying);
            ShaderStorage.Emit(Vc4Inst.GetInstruction());
            iRegUsed++;
        }
    }

    // Issue NOP.
    {
        Vc4Instruction Vc4Inst;
        ShaderStorage.Emit(Vc4Inst.GetInstruction());
    }

    // Issue interporation to each data.
    for (uint8_t iRegUsed = 0, iRegIndex = 0; iRegUsed < this->cInput; iRegIndex++)
    {
        Vc4Register raX = this->InputRegister[iRegIndex / 4][iRegIndex % 4];

        if (raX.flags.valid && raX.flags.require_linear_conversion)
        {
            {
                Vc4Instruction Vc4Inst;
                Vc4Register r1(VC4_QPU_ALU_REG_A, VC4_QPU_WADDR_ACC1);
                Vc4Register r5(VC4_QPU_ALU_R5);
                Vc4Inst.Vc4_a_FADD(r1, raX, r5);
                ShaderStorage.Emit(Vc4Inst.GetInstruction());
            }
        
            {
                Vc4Instruction Vc4Inst;
                Vc4Register r1(VC4_QPU_ALU_R1);
                Vc4Inst.Vc4_a_MOV(raX, r1);
                ShaderStorage.Emit(Vc4Inst.GetInstruction());
            }

            iRegUsed++;
        }
    }
    
    { // Emit a NOP with 'sbwait'
        Vc4Instruction Vc4Inst;
        Vc4Inst.Vc4_Sig(VC4_QPU_SIG_WAIT_FOR_SCOREBOARD);
        ShaderStorage.Emit(Vc4Inst.GetInstruction());
    }
}

void Vc4Shader::Emit_ShaderOutput_VS(boolean bVS, Vc4ShaderStorage *Storage)
{
    Storage->Ensure((bVS ? 0 : 4)                        // raw coordinate outputs X/Y/Z/W.
                     + 8                                 // 2 fmul, 2 ftoi, 1 NOP and 1 mov.
                     + (bVS ? (this->cOutput - 4) : 0)); // varying output
    
    Vc4Register vpm(VC4_QPU_ALU_REG_B, VC4_QPU_WADDR_VPM);

    Vc4Register pos[4]; // X/Y/Z/W.
    for (uint8_t iPos = 0; iPos < this->cOutput; iPos++)
    {
        if (this->OutputRegister[iPos / 4][iPos % 4].flags.position)
        {
            for (uint8_t i = iPos; i < iPos + 4; i++)
            {
                Vc4Register reg = this->OutputRegister[i / 4][i % 4];
                assert(reg.flags.position);
                pos[this->SwizzleMaskToIndex(reg.swizzleMask)] = reg;
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
            Storage->Emit(Vc4Inst.GetInstruction());
        }
    }

    // Ys/Xs
    {
        // scale by RT dimension (read from uniform). Result in r0/r1.
        {
            for (uint8_t i = 0; i < 2; i++)
            {
                Vc4Register rX(VC4_QPU_ALU_REG_B, VC4_QPU_WADDR_ACC0 + i); // r0 and r1.
                Vc4Register unif((pos[i].mux == VC4_QPU_ALU_REG_A ? VC4_QPU_ALU_REG_B : VC4_QPU_ALU_REG_A), VC4_QPU_RADDR_UNIFORM); // use opossit register file.
                Vc4Instruction Vc4Inst;
                 Vc4Inst.Vc4_m_FMUL(rX, pos[i], unif);
                Storage->Emit(Vc4Inst.GetInstruction());
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
                Storage->Emit(Vc4Inst.GetInstruction());
            }

            // Issue NOP as ra16 is just written.
            {
                Vc4Instruction Vc4Inst;
                ShaderStorage.Emit(Vc4Inst.GetInstruction());
            }

            // Output to vpm.
            {
                Vc4Instruction Vc4Inst;
                Vc4Inst.Vc4_m_MOV(vpm, ra16);
                Storage->Emit(Vc4Inst.GetInstruction());
            }
        }
    }

    // Zc
    {
        Vc4Instruction Vc4Inst;
        Vc4Inst.Vc4_m_MOV(vpm, pos[2]);
        Storage->Emit(Vc4Inst.GetInstruction());
    }

    // 1/W // TODO: this must be 1/Wc
    {
        Vc4Register w;
        Vc4Instruction Vc4Inst;
        Vc4Inst.Vc4_m_MOV(vpm, pos[3]);
        Storage->Emit(Vc4Inst.GetInstruction());
    }

    // Only VS emits "varying" (everything read from vpm except position data).
    if (bVS)
    {
        for (uint8_t i = 0; i < this->cOutput; i++)
        {
            if (this->OutputRegister[i / 4][i % 4].flags.position == false)
            {
                Vc4Register src = this->OutputRegister[i / 4][i % 4];
                Vc4Instruction Vc4Inst;
                Vc4Inst.Vc4_m_MOV(vpm, src);
                Storage->Emit(Vc4Inst.GetInstruction());
            }
        }
    }
}

void Vc4Shader::Emit_Epilogue(Vc4ShaderStorage *Storage)
{
    if (Storage == NULL)
    {
        Storage = &(this->ShaderStorage);
    }

    // +1 for thrend with color output.
    // +2 for NOP(s) and unblock.
    Storage->Ensure(3);

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
        Storage->Emit(Vc4Inst.GetInstruction());
    }

    // output 2 NOP(s).
    for (UINT i = 0; i < 2; i++)
    {
        Vc4Instruction Vc4Inst;
        if ((D3D10_SB_PIXEL_SHADER == this->uShaderType) && (i == 1))
        {
            Vc4Inst.Vc4_Sig(VC4_QPU_SIG_SCOREBOARD_UNBLOCK);
        }
        Storage->Emit(Vc4Inst.GetInstruction());
    }
}

void Vc4Shader::Emit_Mov(CInstruction &Inst)
{
    // max 4 mov(s) + 1 NOP.
    this->ShaderStorage.Ensure(5);

    assert(Inst.m_NumOperands == 2);

    assert(Inst.m_Operands[0].m_NumComponents == D3D10_SB_OPERAND_4_COMPONENT);
    assert(Inst.m_Operands[0].m_IndexDimension == D3D10_SB_OPERAND_INDEX_1D);
    assert(Inst.m_Operands[0].m_ComponentSelection == D3D10_SB_OPERAND_4_COMPONENT_MASK_MODE);

    {
        uint8_t aCurrent = D3D10_SB_OPERAND_4_COMPONENT_MASK_X;
        for (uint8_t i = 0; i < 4; i++)
        {
            if (Inst.m_Operands[0].m_WriteMask & aCurrent)
            {
                Vc4Register dst = Find_Vc4Register(Inst.m_Operands[0], aCurrent);
                uint8_t pack = VC4_QPU_PACK_A_32;
                if (dst.flags.packed)
                {
                    pack = VC4_QPU_PACK_MUL_8a + SwizzleMaskToIndex(aCurrent);
                }

                switch (Inst.m_Operands[1].m_Type)
                {
                case D3D10_SB_OPERAND_TYPE_IMMEDIATE32:
                {
                    Vc4Instruction Vc4Inst(vc4_load_immediate_32);
                    Vc4Value value;
                    switch (Inst.m_Operands[1].m_NumComponents)
                    {
                    case D3D10_SB_OPERAND_1_COMPONENT:
                        value.i = Inst.m_Operands[1].m_Value[0];
                        break;
                    case D3D10_SB_OPERAND_4_COMPONENT:
                        value.i = Inst.m_Operands[1].m_Value[i];
                        break;
                    default:
                        assert(false);
                    }
                    Vc4Inst.Vc4_m_LOAD32(dst, value);
                    Vc4Inst.Vc4_m_Pack(pack);
                    ShaderStorage.Emit(Vc4Inst.GetInstruction());
                    break;
                }
                case D3D10_SB_OPERAND_TYPE_IMMEDIATE64:
                {
                    assert(false);
                    break;
                }
                default:
                {
                    assert(Inst.m_Operands[1].m_NumComponents == D3D10_SB_OPERAND_4_COMPONENT);
                    assert(Inst.m_Operands[1].m_ComponentSelection == D3D10_SB_OPERAND_4_COMPONENT_SWIZZLE_MODE);
                    Vc4Register src = Find_Vc4Register(Inst.m_Operands[1], D3D10_SB_OPERAND_4_COMPONENT_MASK(Inst.m_Operands[1].m_Swizzle[i]));
                    Vc4Instruction Vc4Inst;
                    Vc4Inst.Vc4_m_MOV(dst, src);
                    Vc4Inst.Vc4_m_Pack(pack);
                    ShaderStorage.Emit(Vc4Inst.GetInstruction());
                    break;
                }
                }
            }
            aCurrent <<= 1;
        }
    }

    { // Emit a NOP
        Vc4Instruction Vc4Inst;
        ShaderStorage.Emit(Vc4Inst.GetInstruction());
    }
}

void Vc4Shader::Emit_Sample(CInstruction &Inst)
{
    assert(this->uShaderType == D3D10_SB_PIXEL_SHADER);
    
    assert(Inst.m_NumOperands == 4);

    // Output : only supports 8888RGBA output for now.
    Vc4Register o;

    assert(Inst.m_Operands[0].m_NumComponents == D3D10_SB_OPERAND_4_COMPONENT);
    assert(Inst.m_Operands[0].m_IndexDimension == D3D10_SB_OPERAND_INDEX_1D);
    if (Inst.m_Operands[0].m_Type == D3D10_SB_OPERAND_TYPE_OUTPUT)
    {
        assert(Inst.m_Operands[0].m_ComponentSelection == D3D10_SB_OPERAND_4_COMPONENT_MASK_MODE);
        assert(Inst.m_Operands[0].m_WriteMask == (D3D10_SB_OPERAND_4_COMPONENT_MASK_R | D3D10_SB_OPERAND_4_COMPONENT_MASK_G | D3D10_SB_OPERAND_4_COMPONENT_MASK_B | D3D10_SB_OPERAND_4_COMPONENT_MASK_A));
        o = Find_Vc4Register(Inst.m_Operands[0], (Inst.m_Operands[0].m_WriteMask & D3D10_SB_OPERAND_4_COMPONENT_MASK_MASK));
    }
    else
    {
        assert(false);
    }

    // Resource
    assert(Inst.m_Operands[2].m_Type == D3D10_SB_OPERAND_TYPE_RESOURCE);
    assert(Inst.m_Operands[2].m_NumComponents == D3D10_SB_OPERAND_4_COMPONENT);
    assert(Inst.m_Operands[2].m_IndexDimension == D3D10_SB_OPERAND_INDEX_1D);

    uint32_t texDimension = this->ResourceDimension[Inst.m_Operands[2].m_Index[0].m_RegIndex];

    // Sampler
    assert(Inst.m_Operands[3].m_Type == D3D10_SB_OPERAND_TYPE_SAMPLER);

    // Texture coordinate
    assert(Inst.m_Operands[1].m_NumComponents == D3D10_SB_OPERAND_4_COMPONENT);
    assert(Inst.m_Operands[1].m_IndexDimension == D3D10_SB_OPERAND_INDEX_1D);
    assert(Inst.m_Operands[1].m_ComponentSelection == D3D10_SB_OPERAND_4_COMPONENT_SWIZZLE_MODE);

    Vc4Register s;
    Vc4Register t;
    Vc4Register r;

    switch (texDimension)
    {
    case D3D10_SB_RESOURCE_DIMENSION_TEXTURECUBE:
        r = Find_Vc4Register(Inst.m_Operands[1], D3D10_SB_OPERAND_4_COMPONENT_MASK(Inst.m_Operands[1].m_Swizzle[2]));
        __fallthrough;
    case D3D10_SB_RESOURCE_DIMENSION_TEXTURE2D:
        t = Find_Vc4Register(Inst.m_Operands[1], D3D10_SB_OPERAND_4_COMPONENT_MASK(Inst.m_Operands[1].m_Swizzle[1]));
        __fallthrough;
    case D3D10_SB_RESOURCE_DIMENSION_TEXTURE1D:
        s = Find_Vc4Register(Inst.m_Operands[1], D3D10_SB_OPERAND_4_COMPONENT_MASK(Inst.m_Operands[1].m_Swizzle[0]));
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
            
    // + 1 for sample instruction, 'ldtmu0', and + 1 for moving r4 to output register, and + 1 for NOP.
    this->ShaderStorage.Ensure((r.flags.valid && t.flags.valid ? 3 : t.flags.valid ? 2 : 1) + 3); 

    // texture address : z
    if (r.flags.valid)
    {
        Vc4Instruction Vc4Inst;
        Vc4Register tmu0_r(VC4_QPU_ALU_REG_A, VC4_QPU_WADDR_TMU0_R);
        Vc4Inst.Vc4_a_MOV(tmu0_r, r);
        ShaderStorage.Emit(Vc4Inst.GetInstruction());
    }

    // texture address : y
    if (t.flags.valid)
    {
        Vc4Instruction Vc4Inst;
        Vc4Register tmu0_t(VC4_QPU_ALU_REG_A, VC4_QPU_WADDR_TMU0_T);
        Vc4Inst.Vc4_a_MOV(tmu0_t, t);
        ShaderStorage.Emit(Vc4Inst.GetInstruction());
    }

    // texture address : x and must write 's' at last.
    assert(s.flags.valid);
    {
        Vc4Instruction Vc4Inst;
        Vc4Register tmu0_s(VC4_QPU_ALU_REG_A, VC4_QPU_WADDR_TMU0_S);
        Vc4Inst.Vc4_a_MOV(tmu0_s, s);
        ShaderStorage.Emit(Vc4Inst.GetInstruction());
    }

    // Sample texture, result come up in r4.
    {
        Vc4Instruction Vc4Inst;
        Vc4Inst.Vc4_Sig(VC4_QPU_SIG_LOAD_TMU0);
        ShaderStorage.Emit(Vc4Inst.GetInstruction());
    }

    // TODO: Swizzle texture channel when texture format and RT format are different.
    
    // Move result to r4 to output register.
    if (Inst.m_Operands[0].m_Type == D3D10_SB_OPERAND_TYPE_OUTPUT)
    {
        Vc4Instruction Vc4Inst;
        Vc4Register r4(VC4_QPU_ALU_R4);
        Vc4Inst.Vc4_a_MOV(o, r4);
        ShaderStorage.Emit(Vc4Inst.GetInstruction());
    }
    else
    {
        assert(false);
    }

    { // Emit a NOP
        Vc4Instruction Vc4Inst;
        ShaderStorage.Emit(Vc4Inst.GetInstruction());
    }
}

void Vc4Shader::HLSL_ParseDecl()
{
    boolean bDone = false;
    D3D10_SB_OPCODE_TYPE OpCode;
    while (HLSL_PeekShaderInstructionOpCode(&OpCode) && !bDone)
    { 
        CInstruction Inst;
        switch (OpCode)
        {
        case D3D10_SB_OPCODE_DCL_RESOURCE:
            HLSL_GetShaderInstruction(&Inst);
            assert(cResource < 8);
            this->ResourceDimension[cResource++] = Inst.m_ResourceDecl.SRVInfo.Dimension;
            break;
        case D3D10_SB_OPCODE_DCL_CONSTANT_BUFFER:
            HLSL_GetShaderInstruction(&Inst);
            break;
        case D3D10_SB_OPCODE_DCL_SAMPLER:
            HLSL_GetShaderInstruction(&Inst);
            cSampler++;
            break;
        case D3D10_SB_OPCODE_DCL_INPUT:
        case D3D10_SB_OPCODE_DCL_INPUT_PS:
        {
            HLSL_GetShaderInstruction(&Inst);
            if (Inst.m_OpCode == D3D10_SB_OPCODE_DCL_INPUT_PS)
            {
                assert(Inst.m_InputPSDecl.InterpolationMode == D3D10_SB_INTERPOLATION_LINEAR); // PS input must be linear.
            }
            assert(Inst.m_NumOperands == 1);
            assert(Inst.m_Operands[0].m_ComponentSelection == D3D10_SB_OPERAND_4_COMPONENT_MASK_MODE);
            assert(Inst.m_Operands[0].m_IndexDimension == D3D10_SB_OPERAND_INDEX_1D);
            auto iRegIndex = Inst.m_Operands[0].m_Index[0].m_RegIndex;
            assert(iRegIndex < 8);
            uint8_t aMask = (Inst.m_Operands[0].m_WriteMask & D3D10_SB_OPERAND_4_COMPONENT_MASK_MASK);
            assert(aMask);

            uint8_t aCurrent = D3D10_SB_OPERAND_4_COMPONENT_MASK_X;
            for (uint8_t i = 0; i < 4; i++)
            {
                if (aMask & aCurrent)
                {
                    assert(cInput < 16);
                    this->InputRegister[iRegIndex][i].flags.valid = true;
                    this->InputRegister[iRegIndex][i].flags.require_linear_conversion = (Inst.m_OpCode == D3D10_SB_OPCODE_DCL_INPUT_PS ? true : false);
                    this->InputRegister[iRegIndex][i].addr = cInput++;
                    this->InputRegister[iRegIndex][i].mux = VC4_QPU_ALU_REG_A; // TODO: allocate register. currently Input are assigned to RA0~7.
                    this->InputRegister[iRegIndex][i].swizzleMask = aCurrent;
                }
                aCurrent <<= 1;
            }
            break;
        }
        case D3D10_SB_OPCODE_DCL_OUTPUT:
        case D3D10_SB_OPCODE_DCL_OUTPUT_SIV:
        {
            HLSL_GetShaderInstruction(&Inst);
            assert(Inst.m_NumOperands == 1);
            assert(Inst.m_Operands[0].m_ComponentSelection == D3D10_SB_OPERAND_4_COMPONENT_MASK_MODE);
            assert(Inst.m_Operands[0].m_IndexDimension == D3D10_SB_OPERAND_INDEX_1D);
            if (this->uShaderType == D3D10_SB_PIXEL_SHADER)
            {
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
                // only support single 8888RGBA colour output from pixel shader.
                // TODO: z output if requested.
            }
            else
            {
                assert(this->uShaderType == D3D10_SB_VERTEX_SHADER);
                auto iRegIndex = Inst.m_Operands[0].m_Index[0].m_RegIndex;
                assert(iRegIndex < 8);
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

                uint8_t aCurrent = D3D10_SB_OPERAND_4_COMPONENT_MASK_X;
                for (uint8_t i = 0; i < 4; i++)
                {
                    if (aMask & aCurrent)
                    {
                        assert(cOutput < 16);
                        this->OutputRegister[iRegIndex][i].flags.valid = true;
                        this->OutputRegister[iRegIndex][i].flags.position = bPos;
                        this->OutputRegister[iRegIndex][i].addr = cOutput++;
                        this->OutputRegister[iRegIndex][i].mux = VC4_QPU_ALU_REG_B; // TODO: allocate register. currently Input are assigned to RA0~7.
                        this->OutputRegister[iRegIndex][i].swizzleMask = aCurrent;
                    }
                    aCurrent <<= 1;
                }
            }
            break;
        }
        case D3D10_SB_OPCODE_DCL_TEMPS:
        {
            HLSL_GetShaderInstruction(&Inst);
            // Temp register doesn't have swizzle mask, so assume all 4 components to be used.
            // TODO: AllocateRegister(); Currently temps are allocated at ra16~ra31.
            //       since currently reserve temp to ra16~31, so only upto 4 temps are allowed.
            assert(Inst.m_TempsDecl.NumTemps <= 4);
            for (uint8_t i = 0; i < Inst.m_TempsDecl.NumTemps * 4; i++)
            {
                assert(cTemp < 16);
                this->TempRegister[i / 4][i % 4].flags.valid = true;
                this->TempRegister[i / 4][i % 4].flags.temp = true;
                this->TempRegister[i / 4][i % 4].addr = 16 + cTemp++;  // TODO: don't hardcode 16, allocate register.
                this->TempRegister[i / 4][i % 4].mux = VC4_QPU_ALU_REG_A;
                this->TempRegister[i / 4][i % 4].swizzleMask = D3D10_SB_OPERAND_4_COMPONENT_MASK_X << (i % 4);
            }
            break;
        }
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
            HLSL_GetShaderInstruction(&Inst);
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
    HRESULT hr;
    hr = this->ShaderStorage.Initialize(); // VS
    if (FAILED(hr))
    {
        return hr;
    }

    hr = this->ShaderStorageAux.Initialize(); // CS
    if (FAILED(hr))
    {
        return hr;
    }

    this->HLSL_ParseDecl();
    this->Emit_Prologue_VS();

    {
        CInstruction Inst;
        while (HLSL_GetShaderInstruction(&Inst))
        {
            switch (Inst.m_OpCode)
            {
            case D3D10_SB_OPCODE_MOV:
                this->Emit_Mov(Inst);
                break;
            case D3D10_SB_OPCODE_RET:
                break;
            default:
                assert(false);
            }
        }
    }

    this->ShaderStorageAux.CopyFrom(&(this->ShaderStorage)); // Copy VS to CS.
    
    this->Emit_ShaderOutput_VS(true, &(this->ShaderStorage)); // VS
    this->Emit_ShaderOutput_VS(false, &(this->ShaderStorageAux)); // CS

    this->Emit_Epilogue(&(this->ShaderStorage)); // VS
    this->Emit_Epilogue(&(this->ShaderStorageAux)); // CS
    
#if DBG
    Vc4Disasm().Run((const VC4_QPU_INSTRUCTION*)this->ShaderStorage.GetStorage(), this->ShaderStorage.GetStorageUsedSize() * sizeof(VC4_QPU_INSTRUCTION), TEXT("VC4 Vertex shader"));
    Vc4Disasm().Run((const VC4_QPU_INSTRUCTION*)this->ShaderStorageAux.GetStorage(), this->ShaderStorageAux.GetStorageUsedSize() * sizeof(VC4_QPU_INSTRUCTION), TEXT("VC4 Coordinate shader"));
#endif   

    return S_OK;
}

HRESULT Vc4Shader::Translate_PS()
{
    HRESULT hr;
    hr = this->ShaderStorage.Initialize();
    if (FAILED(hr))
    {
        return hr;
    }
    
    this->HLSL_ParseDecl();
    this->Emit_Prologue_PS();

    {
        CInstruction Inst;
        while (HLSL_GetShaderInstruction(&Inst))
        {
            switch (Inst.m_OpCode)
            {
            case D3D10_SB_OPCODE_SAMPLE:
                this->Emit_Sample(Inst);
                break;
            case D3D10_SB_OPCODE_MOV:
                this->Emit_Mov(Inst);
                break;
            case D3D10_SB_OPCODE_RET:
                break;
            default:
                assert(false);
            }
        }
    }
        
    this->Emit_Epilogue(&(this->ShaderStorage));

#if DBG
    Vc4Disasm().Run((const VC4_QPU_INSTRUCTION*)this->ShaderStorage.GetStorage(), this->ShaderStorage.GetStorageUsedSize() * sizeof(VC4_QPU_INSTRUCTION), TEXT("VC4 Fragment shader"));
#endif

    return S_OK;
}

