#include "precomp.h"
#include "roscompiler.h"

#if VC4

TCHAR *Vc4_QPU_LookUp_String(VC4QPU_TOKENLOOKUP_TABLE *pTable, INT Value)
{
    for (INT i = 0; pTable[i].Value != VC4_QPU_END_OF_LOOKUPTABLE; i++)
    {
        if (pTable[i].Value == Value)
        {
            return (pTable[i].Token ? pTable[i].Token : _TEXT("Invalid"));
        }
    }
    return _TEXT("Invalid");
}

TCHAR *Vc4_QPU_LookUp_Addr_String(VC4QPU_TOKENLOOKUP_ADDR_TABLE *pTable, INT Regfile, INT Value)
{
    for (INT i = 0; pTable[i].LookUp[Regfile].Value != VC4_QPU_END_OF_LOOKUPTABLE; i++)
    {
        if (pTable[i].LookUp[Regfile].Value == Value)
        {
            return (pTable[i].LookUp[Regfile].Token ? pTable[i].LookUp[Regfile].Token : _TEXT("Invalid"));
        }
    }
    return _TEXT("Invalid");
}

#define VC4_QPU_LOOKUP_STRING(Type,Value) Vc4_QPU_LookUp_String(VC4_QPU_##Type##_LOOKUP, Value)
#define VC4_QPU_LOOKUP_ADDR_STRING(Type,Regfile,Value) Vc4_QPU_LookUp_Addr_String(VC4_QPU_##Type##_LOOKUP, Regfile, Value)

HRESULT Vc4Disasm::ParseSignature(VC4_QPU_INSTRUCTION Instruction)
{
    this->xprintf(TEXT("%s\t; "), VC4_QPU_LOOKUP_STRING(SIG, VC4_QPU_GET_SIG(Instruction)));
    return S_OK;
}

HRESULT Vc4Disasm::ParseSmallImmediate(VC4_QPU_INSTRUCTION Instruction)
{
    DWORD dwSmallImmediate = VC4_QPU_GET_SMALL_IMMEDIATE(Instruction);
    if (dwSmallImmediate < 16)
    {
        this->xprintf(TEXT("%d"), dwSmallImmediate);
    }
    else if (dwSmallImmediate < 32)
    {
        this->xprintf(TEXT("%d"), -16 + (dwSmallImmediate - 16));
    }
    else if (dwSmallImmediate < 40)
    {
        this->xprintf(TEXT("%.1f"), (float)(1 << (dwSmallImmediate - 32)));
    }
    else if (dwSmallImmediate < 48)
    {
        this->xprintf(TEXT("%f"), 1.0f / (1 << (48 - dwSmallImmediate)));
    }
    else if (dwSmallImmediate == 48)
    {
        this->xprintf(TEXT("%s << r5[3:0]"), VC4_QPU_LOOKUP_STRING(ALU, VC4_QPU_GET_MUL_B(Instruction)));
    }
    else if (dwSmallImmediate < 64)
    {
        this->xprintf(TEXT("%s << %d"), VC4_QPU_LOOKUP_STRING(ALU, VC4_QPU_GET_MUL_B(Instruction)), dwSmallImmediate - 48);
    }
    else
    {
        this->xprintf(TEXT("Invalid"));
    }
    return S_OK;
 }

HRESULT Vc4Disasm::ParseWriteAddr(DWORD waddr, bool bRegfile_A)
{
    this->xprintf(TEXT("%s"), VC4_QPU_LOOKUP_ADDR_STRING(WADDR, bRegfile_A ? 0 : 1, waddr));
    return S_OK;
}

HRESULT Vc4Disasm::ParseWrite(VC4_QPU_INSTRUCTION Instruction, bool bAddOp, bool bShowPack)
{
    bool bRegfile_A = ((bAddOp == true) == (VC4_QPU_IS_WRITESWAP_SET(Instruction) == 0)) ? true : false;
    DWORD waddr = bAddOp ? VC4_QPU_GET_WADDR_ADD(Instruction) : VC4_QPU_GET_WADDR_MUL(Instruction);
    ParseWriteAddr(waddr, bRegfile_A);
    if (VC4_QPU_WADDR_TMU0_S <= waddr && waddr <= VC4_QPU_WADDR_TMU1_B)
    {
        uniformIndex++;
    }
    if (bShowPack)
    {
        if (bRegfile_A && !VC4_QPU_IS_PM_SET(Instruction))
        {
            this->xprintf(TEXT("%s"), VC4_QPU_LOOKUP_STRING(PACK_A, VC4_QPU_GET_PACK(Instruction)));
        }
        else if (!bAddOp && VC4_QPU_IS_PM_SET(Instruction))
        {
            this->xprintf(TEXT("%s"), VC4_QPU_LOOKUP_STRING(PACK_MUL, VC4_QPU_GET_PACK(Instruction)));
        }
    }
    return S_OK;
}

HRESULT Vc4Disasm::ParseReadAddr(DWORD raddr, bool bRegfile_A)
{
    this->xprintf(TEXT("%s"), VC4_QPU_LOOKUP_ADDR_STRING(RADDR, bRegfile_A ? 0 : 1, raddr));
    if (raddr == VC4_QPU_RADDR_UNIFORM)
    {
        this->xprintf(TEXT("[%d]"), this->uniformIndex++);
    }
    return S_OK;
}

HRESULT Vc4Disasm::ParseRead(VC4_QPU_INSTRUCTION Instruction, DWORD mux)
{
    if (mux < VC4_QPU_ALU_REG_A)
    {
        this->xprintf(TEXT("%s"), VC4_QPU_LOOKUP_STRING(ALU, mux));
        if (mux == VC4_QPU_ALU_R4 && VC4_QPU_IS_PM_SET(Instruction))
        {
            this->xprintf(TEXT("%s"), VC4_QPU_LOOKUP_STRING(UNPACK, VC4_QPU_GET_UNPACK(Instruction)));
        }
    }
    else if (mux == VC4_QPU_ALU_REG_A)
    {
        ParseReadAddr(VC4_QPU_GET_RADDR_A(Instruction), true);
        if (!VC4_QPU_IS_PM_SET(Instruction))
        {
            this->xprintf(TEXT("%s"), VC4_QPU_LOOKUP_STRING(UNPACK, VC4_QPU_GET_UNPACK(Instruction)));
        }
    } 
    else if (mux == VC4_QPU_ALU_REG_B)
    {
        ParseReadAddr(VC4_QPU_GET_RADDR_B(Instruction), false);
    }
    else
    {
        this->xprintf(TEXT("Invalid"));
    }
    return S_OK;
}

HRESULT Vc4Disasm::ParseAddOp(VC4_QPU_INSTRUCTION Instruction)
{
    this->xprintf(TEXT("%s%s%s "),
        (VC4_QPU_IS_OPCODE_ADD_MOV(Instruction) ? VC4_QPU_Name_Op_Move : VC4_QPU_LOOKUP_STRING(OPCODE_ADD, VC4_QPU_GET_OPCODE_ADD(Instruction))),
        (VC4_QPU_IS_SETFLAGS_SET(Instruction) ? VC4_QPU_Name_SetFlag : VC4_QPU_Name_Empty),
        (VC4_QPU_IS_OPCODE_ADD_NOP(Instruction) ? VC4_QPU_Name_Empty : VC4_QPU_LOOKUP_STRING(COND, VC4_QPU_GET_COND_ADD(Instruction))));
    if (!VC4_QPU_IS_OPCODE_ADD_NOP(Instruction))
    {
        ParseWrite(Instruction, true);
        this->xprintf(TEXT(", "));
        if (!VC4_QPU_IS_OPCODE_ADD_MOV(Instruction))
        {
            ParseRead(Instruction, VC4_QPU_GET_ADD_A(Instruction));
            this->xprintf(TEXT(", "));
        }
        if (VC4_QPU_IS_OPCODE_LOAD_SM(Instruction) && (VC4_QPU_GET_ADD_B(Instruction) == VC4_QPU_ALU_REG_B))
        {
            ParseSmallImmediate(Instruction);
        }
        else
        {
            ParseRead(Instruction, VC4_QPU_GET_ADD_B(Instruction));
        }
    }
    return S_OK;
}

HRESULT Vc4Disasm::ParseMulOp(VC4_QPU_INSTRUCTION Instruction)
{
    this->xprintf(TEXT("%s%s%s "),
        (VC4_QPU_IS_OPCODE_MUL_MOV(Instruction) ? VC4_QPU_Name_Op_Move : VC4_QPU_LOOKUP_STRING(OPCODE_MUL, VC4_QPU_GET_OPCODE_MUL(Instruction))),
        (VC4_QPU_IS_SETFLAGS_SET(Instruction) ? VC4_QPU_Name_SetFlag : VC4_QPU_Name_Empty),
        (VC4_QPU_IS_OPCODE_MUL_NOP(Instruction) ? VC4_QPU_Name_Empty : VC4_QPU_LOOKUP_STRING(COND, VC4_QPU_GET_COND_MUL(Instruction))));
    if (!VC4_QPU_IS_OPCODE_MUL_NOP(Instruction))
    {
        ParseWrite(Instruction, false);
        this->xprintf(TEXT(", "));
        if (!VC4_QPU_IS_OPCODE_MUL_MOV(Instruction))
        {
            ParseRead(Instruction, VC4_QPU_GET_MUL_A(Instruction));
            this->xprintf(TEXT(", "));
        }
        if (VC4_QPU_IS_OPCODE_LOAD_SM(Instruction) && (VC4_QPU_GET_MUL_B(Instruction) == VC4_QPU_ALU_REG_B))
        {
            ParseSmallImmediate(Instruction);
        }
        else
        {
            ParseRead(Instruction, VC4_QPU_GET_MUL_B(Instruction));
        }
    }
    return S_OK;
}

HRESULT Vc4Disasm::ParseALUInstruction(VC4_QPU_INSTRUCTION Instruction)
{
    ParseAddOp(Instruction);
    this->xprintf(TEXT(" ; "));
    ParseMulOp(Instruction);
    return S_OK;
}

HRESULT Vc4Disasm::ParseLoadImmInstruction(VC4_QPU_INSTRUCTION Instruction)
{
    if (VC4_QPU_GET_WADDR_ADD(Instruction) != VC4_QPU_WADDR_NOP)
    {
        this->xprintf(TEXT("%s%s%s "),
            VC4_QPU_Name_Op_Move,
            ((VC4_QPU_GET_IMMEDIATE_TYPE(Instruction) == VC4_QPU_IMMEDIATE_TYPE_32) ? TEXT("") : (VC4_QPU_GET_IMMEDIATE_TYPE(Instruction) == VC4_QPU_IMMEDIATE_TYPE_PER_ELEMENT_SIGNED) ? TEXT("_per_element_signed") : (VC4_QPU_GET_IMMEDIATE_TYPE(Instruction) == VC4_QPU_IMMEDIATE_TYPE_PER_ELEMENT_UNSIGNED) ? TEXT("per_element_unsigned") : TEXT("Invalid")),
            (VC4_QPU_IS_SETFLAGS_SET(Instruction) ? VC4_QPU_Name_SetFlag : VC4_QPU_Name_Empty));
        ParseWrite(Instruction, true);
        this->xprintf(TEXT("%s"), VC4_QPU_LOOKUP_STRING(COND, VC4_QPU_GET_COND_ADD(Instruction)));
        this->xprintf(TEXT(", "));
        this->xprintf(TEXT("0x%08x"), VC4_QPU_GET_IMMEDIATE_32(Instruction));
    }
    else
    {
        this->xprintf(TEXT("%s"), VC4_QPU_LOOKUP_STRING(OPCODE_ADD, VC4_QPU_OPCODE_ADD_NOP));
    }
    this->xprintf(TEXT(" ; "));
    if (VC4_QPU_GET_WADDR_MUL(Instruction) != VC4_QPU_WADDR_NOP)
    {
        this->xprintf(TEXT("%s%s%s "),
            VC4_QPU_Name_Op_Move,
            ((VC4_QPU_GET_IMMEDIATE_TYPE(Instruction) == VC4_QPU_IMMEDIATE_TYPE_32) ? TEXT("") : (VC4_QPU_GET_IMMEDIATE_TYPE(Instruction) == VC4_QPU_IMMEDIATE_TYPE_PER_ELEMENT_SIGNED) ? TEXT("_per_element_signed") : (VC4_QPU_GET_IMMEDIATE_TYPE(Instruction) == VC4_QPU_IMMEDIATE_TYPE_PER_ELEMENT_UNSIGNED) ? TEXT("per_element_unsigned") : TEXT("Invalid")),
            (VC4_QPU_IS_SETFLAGS_SET(Instruction) ? VC4_QPU_Name_SetFlag : VC4_QPU_Name_Empty));
        ParseWrite(Instruction, false);
        this->xprintf(TEXT("%s"), VC4_QPU_LOOKUP_STRING(COND, VC4_QPU_GET_COND_MUL(Instruction)));
        this->xprintf(TEXT(", "));
        this->xprintf(TEXT("0x%08x"), VC4_QPU_GET_IMMEDIATE_32(Instruction));
    }
    else
    {
        this->xprintf(TEXT("%s"), VC4_QPU_LOOKUP_STRING(OPCODE_MUL, VC4_QPU_OPCODE_MUL_NOP));
    }
    return S_OK;
}

HRESULT Vc4Disasm::ParseSemaphoreInstruction(VC4_QPU_INSTRUCTION Instruction)
{
    this->xprintf(TEXT("Semaphore instruction is not supported yet"));
    return S_OK;
    Instruction;
}
        
HRESULT Vc4Disasm::ParseBranchInstruction(VC4_QPU_INSTRUCTION Instruction)
{
    this->xprintf(TEXT("%s%s "),
        (VC4_QPU_IS_BRANCH_RELATIVE(Instruction) ? TEXT("brr") : TEXT("bra")),
        (VC4_QPU_LOOKUP_STRING(BRANCH_COND, VC4_QPU_GET_BRANCH_COND(Instruction))));
    if (VC4_QPU_IS_BRANCH_USE_RADDR_A(Instruction))
    {
        ParseReadAddr(VC4_QPU_GET_BRANCH_RADDR_A(Instruction), true);
    }
    if (VC4_QPU_GET_IMMEDIATE_32(Instruction) != 0)
    {
        this->xprintf(TEXT("+0x%x"), VC4_QPU_GET_IMMEDIATE_32(Instruction));
    }
    if ((VC4_QPU_GET_WADDR_ADD(Instruction) != VC4_QPU_WADDR_NOP) ||
        (VC4_QPU_GET_WADDR_MUL(Instruction) != VC4_QPU_WADDR_NOP))
    {
        this->xprintf(TEXT("; ret_addr saved to "));
        ParseWrite(Instruction, true, false);
        this->xprintf(TEXT(" "));
        ParseWrite(Instruction, false, false);
    }
    return S_OK;
}

HRESULT Vc4Disasm::ParseFlags(VC4_QPU_INSTRUCTION Instruction)
{
    switch (VC4_QPU_GET_SIG(Instruction))
    {
    case VC4_QPU_SIG_BREAK:
    case VC4_QPU_SIG_NO_SIGNAL:
    case VC4_QPU_SIG_THREAD_SWITCH:
    case VC4_QPU_SIG_PROGRAM_END:
    case VC4_QPU_SIG_WAIT_FOR_SCOREBOARD:
    case VC4_QPU_SIG_SCOREBOARD_UNBLOCK:
    case VC4_QPU_SIG_LAST_THREAD_SWITCH:
    case VC4_QPU_SIG_COVERAGE_LOAD:
    case VC4_QPU_SIG_COLOR_LOAD:
    case VC4_QPU_SIG_COLOR_LOAD_AND_PROGRAM_END:
    case VC4_QPU_SIG_LOAD_TMU0:
    case VC4_QPU_SIG_LOAD_TMU1:
    case VC4_QPU_SIG_ALPAH_MASK_LOAD:
    case VC4_QPU_SIG_ALU_WITH_RADDR_B:
    case VC4_QPU_SIG_LOAD_IMMEDIATE:
        if (!VC4_QPU_IS_OPCODE_NOP(Instruction) || 
            (VC4_QPU_GET_SIG(Instruction) == VC4_QPU_SIG_LOAD_IMMEDIATE))
        {
            this->xprintf(TEXT("\t // pm = %d, sf = %d, ws = %d"),
                VC4_QPU_IS_PM_SET(Instruction) ? 1 : 0,
                VC4_QPU_IS_SETFLAGS_SET(Instruction) ? 1 : 0,
                VC4_QPU_IS_WRITESWAP_SET(Instruction) ? 1 : 0);
        }
        break;
    case VC4_QPU_SIG_BRANCH:
        // TODO:
        break;
    }
    return S_OK;
}

HRESULT
Vc4Disasm::Run(const VC4_QPU_INSTRUCTION* pShader, ULONG ShaderSize, TCHAR *pTitle)
{
    if (pTitle) this->xprintf(TEXT("---------- %s ----------\n"), pTitle);
    ULONG cInstruction = ShaderSize / sizeof(VC4_QPU_INSTRUCTION);
    for (ULONG i = 0; i < cInstruction; i++)
    {
        VC4_QPU_INSTRUCTION Instruction = pShader[i];
        ParseSignature(Instruction);
        switch (VC4_QPU_GET_SIG(Instruction))
        {
        case VC4_QPU_SIG_BREAK:
        case VC4_QPU_SIG_NO_SIGNAL:
        case VC4_QPU_SIG_THREAD_SWITCH:
        case VC4_QPU_SIG_PROGRAM_END:
        case VC4_QPU_SIG_WAIT_FOR_SCOREBOARD:
        case VC4_QPU_SIG_SCOREBOARD_UNBLOCK:
        case VC4_QPU_SIG_LAST_THREAD_SWITCH:
        case VC4_QPU_SIG_COVERAGE_LOAD:
        case VC4_QPU_SIG_COLOR_LOAD:
        case VC4_QPU_SIG_COLOR_LOAD_AND_PROGRAM_END:
        case VC4_QPU_SIG_LOAD_TMU0:
        case VC4_QPU_SIG_LOAD_TMU1:
        case VC4_QPU_SIG_ALPAH_MASK_LOAD:
        case VC4_QPU_SIG_ALU_WITH_RADDR_B:
            ParseALUInstruction(Instruction);
            break;
        case VC4_QPU_SIG_LOAD_IMMEDIATE:
            if (VC4_QPU_IS_OPCODE_SEMAPHORE(Instruction))
            {
                ParseSemaphoreInstruction(Instruction);
            }
            else
            {
                ParseLoadImmInstruction(Instruction);
            }
            break;
        case VC4_QPU_SIG_BRANCH:
            ParseBranchInstruction(Instruction);
            break;
        default:
            this->xprintf(TEXT("Invalid signature"));
            break;
        }
        ParseFlags(Instruction);
        Flush(0);
    }
    return S_OK;
}

EXTERN_C void Vc4Disassemble(VC4_QPU_INSTRUCTION *pHwCode, UINT HwCodeSize, fnPrinter Printer)
{
    Vc4Disasm Disasm;
    Disasm.SetPrinter(Printer);
    Disasm.Run((const VC4_QPU_INSTRUCTION*)pHwCode, HwCodeSize, NULL);
}

#endif // VC4
