// Vc4Asm.cpp : Defines the entry point for the console application.
//
// Really simple VC4 QPU Assembler (just enough for verification)

#include "stdafx.h"

EXTERN_C void Vc4Disassemble(VC4_QPU_INSTRUCTION *pHwCode, UINT HwCodeSize, fnPrinter Printer);

TCHAR szAsmFile[MAX_PATH] = { 0 };

TCHAR szBuff[4096] = { 0 };
TCHAR szOriginal[4096] = { 0 };

const TCHAR szDelimiters[] = _TEXT(" ,;\n\t");
const TCHAR szPeriod[] = _TEXT(".");

#if _DEBUG
#define RETURN_WHEN_INVALID(x,msg,arg,line) if ((x) == VC4_QPU_INVALID_VALUE) { _ftprintf_s(stderr, msg, arg, Line); __debugbreak(); return E_FAIL; }
#define RETURN_ERROR(err,msg,param) _ftprintf_s(stderr, msg, param); __debugbreak(); return err;
#else
#define RETURN_WHEN_INVALID(x,msg,arg,line) if ((x) == VC4_QPU_INVALID_VALUE) { _ftprintf_s(stderr, msg, arg, Line); return E_FAIL; }
#define RETURN_ERROR(err,msg,param) _ftprintf_s(stderr, msg, param); return err;
#endif // _DBG

INT Vc4_QPU_LookUp_Value(VC4QPU_TOKENLOOKUP_TABLE *pTable, TCHAR *pToken)
{
    for (INT i = 0; pTable[i].Value != VC4_QPU_END_OF_LOOKUPTABLE; i++)
    {
        TCHAR *pTableToken = pTable[i].Token;
        if (pToken[0] != '.' && pTableToken[0] == '.')
        {
            pTableToken++;
        }
        if (_tcsicmp(pTableToken, pToken) == 0)
        {
            return (pTable[i].Value);
        }
    }
    return VC4_QPU_INVALID_VALUE;
}

INT Vc4_QPU_LookUp_Addr_Value(VC4QPU_TOKENLOOKUP_ADDR_TABLE *pTable, INT Regfile, TCHAR *pToken, boolean *pExchangeable)
{
    for (INT i = 0; pTable[i].LookUp[Regfile].Value != VC4_QPU_END_OF_LOOKUPTABLE; i++)
    {
        TCHAR *pTableToken = pTable[i].LookUp[Regfile].Token;
        if (_tcsicmp(pTableToken, pToken) == 0)
        {
            if (pExchangeable)
            {
                *pExchangeable = pTable[i].Exchangeable;
            }
            return (pTable[i].LookUp[Regfile].Value);
        }
    }
    return VC4_QPU_INVALID_VALUE;
}

#define VC4_QPU_LOOKUP_VALUE(Type,pToken) Vc4_QPU_LookUp_Value(VC4_QPU_##Type##_LOOKUP, pToken)
#define VC4_QPU_LOOKUP_ADDR_VALUE(Type,Regfile,pToken,pExchangeable) Vc4_QPU_LookUp_Addr_Value(VC4_QPU_##Type##_LOOKUP, Regfile, pToken, pExchangeable)

INT Vc4_QPU_LookUp_AddOp_ArgCount(INT AddOp)
{
    switch (AddOp)
    {
    case VC4_QPU_OPCODE_ADD_NOP:
        return 0;
    case VC4_QPU_OPCODE_ADD_FADD:
    case VC4_QPU_OPCODE_ADD_FSUB:
    case VC4_QPU_OPCODE_ADD_FMIN:
    case VC4_QPU_OPCODE_ADD_FMAX:
    case VC4_QPU_OPCODE_ADD_FMIN_ABS:
    case VC4_QPU_OPCODE_ADD_FMAX_ABS:
    case VC4_QPU_OPCODE_ADD_ADD:
    case VC4_QPU_OPCODE_ADD_SUB:
    case VC4_QPU_OPCODE_ADD_SHR:
    case VC4_QPU_OPCODE_ADD_ASR:
    case VC4_QPU_OPCODE_ADD_ROR:
    case VC4_QPU_OPCODE_ADD_SHL:
    case VC4_QPU_OPCODE_ADD_MIN:
    case VC4_QPU_OPCODE_ADD_MAX:
    case VC4_QPU_OPCODE_ADD_AND:
    case VC4_QPU_OPCODE_ADD_OR:
    case VC4_QPU_OPCODE_ADD_XOR:
    case VC4_QPU_OPCODE_ADD_V8ADDS:
    case VC4_QPU_OPCODE_ADD_V8SUBS:
        return 3; // dest, src0, src1
    case VC4_QPU_OPCODE_ADD_FTOI:
    case VC4_QPU_OPCODE_ADD_ITOF:
    case VC4_QPU_OPCODE_ADD_NOT:
    case VC4_QPU_OPCODE_ADD_CLZ:
    case VC4_QPU_OPCODE_ADD_MOV:
        return 2; // dest, src
    default:
        assert(false);
    }
    return -1;
}

INT Vc4_QPU_LookUp_MulOp_ArgCount(INT MulOp)
{
    switch (MulOp)
    {
    case VC4_QPU_OPCODE_MUL_NOP:
        return 0;
    case VC4_QPU_OPCODE_MUL_FMUL:
    case VC4_QPU_OPCODE_MUL_MUL24:
    case VC4_QPU_OPCODE_MUL_V8MULD:
    case VC4_QPU_OPCODE_MUL_V8MIN:
    case VC4_QPU_OPCODE_MUL_V8MAX:
    case VC4_QPU_OPCODE_MUL_V8ADDS:
    case VC4_QPU_OPCODE_MUL_V8SUBS:
        return 3; // dest, src0, src1
    case VC4_QPU_OPCODE_MUL_MOV:
        return 2; // dest, src
    default:
        assert(false);
    }
    return -1;
}

boolean IsOpEmpty(TCHAR*p, TCHAR **ppNext)
{
    assert(p && ppNext);

    // skip space
    while (*p == _TEXT(' ') || *p == _TEXT('\t'))
    {
        p++;
    }

    if (*p == _TEXT(';'))
    {
        *ppNext = p + 1;
        return true; // no opcode found.
    }
    else
    {
        TCHAR *pNext = _tcschr(p, _TEXT(';'));
        if (pNext)
        {
            *ppNext = pNext + 1;
        }
        else
        {
            pNext = p + _tcslen(p);
        }
        return false;
    }
}

INT IsPowerOfTwo(UINT n)
{
    return (n && ((n & (n - 1)) == 0));
}

INT FindBitPosition(UINT n)
{
    UINT i = 1, pos = 1;
    while (!(i & n))
    {
        i = i << 1;
        ++pos;
    }
    return pos;
}

INT ParseSmallImmediate(TCHAR *p)
{
    // TODO: horizontal vetor rotation is not supported.
    if (_tcscmp(p, _TEXT("1.0/")) == 0)
    {
        p += 4;
        double value = _tstof(p);
        if (value >= 2.0f && value <= 128.0f)
        {
            UINT IntValue = (UINT)value;
            if (value == (double)value)
            {
                if (IsPowerOfTwo(IntValue))
                {
                    return 40 + 9 - FindBitPosition(IntValue); // 40 - 47
                }
            }
        }
    }
    else if (_tcschr(p, TEXT('.')))
    {
        double value = _tstof(p);
        if (value >= 1.0f && value <= 128.0f)
        {
            UINT IntValue = (UINT)value;
            if (value == (double)value)
            {
                if (IsPowerOfTwo(IntValue))
                {
                    return 31 + FindBitPosition(IntValue); // 32 - 39
                }
            }
        }
    }
    else
    {
        INT value = _tstoi(p);
        if (value >= 0 && value <= 15)
        {
            return value; // 0 - 15
        }
        else if (value < 0 && value >= -16)
        {
            return 32 + value; // 16 - 31
        }
    }

    RETURN_ERROR(0, TEXT("Invalid or unsupported small immediate value %s\n"), p);
}

INT32 ParseImmediate(TCHAR *p)
{
    if (p[0] == NULL)
    {
        RETURN_ERROR(E_INVALIDARG, TEXT("Missing immediate value, %s\n"), TEXT("NULL"));
    }
    if ((p[0] == TEXT('0')) && (p[1] == TEXT('x')))
    {
        return (INT32)_tcstoul(p, NULL, 16);
    }
    else if ((p[_tcslen(p)] == TEXT('f')) || (_tcschr(p, TEXT('.')) != NULL))
    {
        union { int i; float f; } iandf;
        iandf.f = _tcstof(p, NULL);
        return (INT32)iandf.i;
    }
    else
    {
        return (INT32)_tcstoul(p, NULL, 10);
    }
}

HRESULT ParseALUInstruction(VC4_QPU_INSTRUCTION &QpuInst, TCHAR *pOpCode, UINT Line)
{
    boolean bPackedRegfileA = false;

    boolean bSetFlags = false;
    boolean bWriteSwap = false;
    boolean bPackUnpackSelect = false;

    INT AddOp = VC4_QPU_OPCODE_ADD_NOP;
    INT AddCond = VC4_QPU_COND_ALWAYS;
    INT AddArgc = 0;

    INT AddMuxDest = 0;
    INT AddWaddr = VC4_QPU_WADDR_NOP;
    INT AddPack = 0;

    INT AddMuxSrc[2] = { 0 };
    INT AddRaddr[2] = { VC4_QPU_RADDR_NOP };
    INT AddUnpack[2] = { 0 };

    boolean AddRegfileExchangeable[3] = { false };

    INT MulOp = VC4_QPU_OPCODE_MUL_NOP;
    INT MulCond = VC4_QPU_COND_ALWAYS;
    INT MulArgc = 0;

    INT MulMuxDest = 0;
    INT MulWaddr = VC4_QPU_WADDR_NOP;
    INT MulPack = 0;

    INT MulMuxSrc[2] = { 0 };
    INT MulRaddr[2] = { VC4_QPU_RADDR_NOP };
    INT MulUnpack[2] = { 0 };

    boolean MulRegfileExchangeable[3] = { false };

    boolean bAddReadRegfile_A = false;
    boolean bAddReadRegfile_B = false;

    boolean bMulUsePack_A = false;
    boolean bAddUsePack_A = false;

    boolean bMulUseUnPack_R4 = false;
    boolean bAddUseUnPack_R4 = false;

    INT Pack = 0; // NOP for any packing
    INT Unpack = 0; // NOP for any unpacking

    INT RaddrA = VC4_QPU_INVALID_VALUE;
    INT RaddrB = VC4_QPU_INVALID_VALUE;

    boolean bSetImmediateValue1 = false;
    boolean bSetImmediateValue2 = false;

    INT32 ImmediateValue1 = 0;
    INT32 ImmediateValue2 = 0;

    TCHAR *pOpCodeNext;
    if (!IsOpEmpty(pOpCode,&pOpCodeNext))
    {
        TCHAR *pTokenNext;
        TCHAR *pToken = _tcstok_s(pOpCode, szDelimiters, &pTokenNext);

        // Parse instruction
        {
            TCHAR* _AddCond = NULL;
            TCHAR* _AddOp = _tcstok_s(pToken, szPeriod, &_AddCond);
            AddOp = VC4_QPU_LOOKUP_VALUE(OPCODE_ADD, _AddOp);
            RETURN_WHEN_INVALID(AddOp, TEXT("Invalid AddOp %s, line %d\n"), _AddOp, Line);
            AddCond = VC4_QPU_LOOKUP_VALUE(COND, _AddCond);
            RETURN_WHEN_INVALID(AddCond, TEXT("Invalid AddOp Cond %s, line %d\n"), _AddCond, Line);
            if (VC4_QPU_GET_SIG(QpuInst) == VC4_QPU_SIG_LOAD_IMMEDIATE)
            {
                if (AddOp != VC4_QPU_OPCODE_ADD_MOV && AddOp != VC4_QPU_OPCODE_ADD_NOP)
                {
                    RETURN_ERROR(E_INVALIDARG, TEXT("Load immediate must be mov instruction, line %d\n"), Line);
                }
            }
        }

        // Parse parameters.
        {
            AddArgc = Vc4_QPU_LookUp_AddOp_ArgCount(AddOp);
            assert(AddArgc <= 3);
            TCHAR* _AddArgv[3] = { NULL };
            for (INT i = 0; i < AddArgc; i++)
            {
                _AddArgv[i] = _tcstok_s(NULL, szDelimiters, &pTokenNext);
            }

            // Encode destination and sources
            for (INT i = 0; i < AddArgc; i++)
            {
                TCHAR* _Pack = NULL;
                if (i == 0)
                {
                    TCHAR* _Reg = _tcstok_s(_AddArgv[i], szPeriod, &_Pack);
                    AddWaddr = VC4_QPU_LOOKUP_ADDR_VALUE(WADDR, 0, _Reg, &AddRegfileExchangeable[i]);
                    if (AddWaddr == VC4_QPU_INVALID_VALUE)
                    {
                        // if can't be found on a file, look for b file.
                        AddWaddr = VC4_QPU_LOOKUP_ADDR_VALUE(WADDR, 1, _Reg, &AddRegfileExchangeable[i]);
                        RETURN_WHEN_INVALID(AddWaddr, TEXT("Invalid AddOp destination %s, line %d\n"), _Reg, Line);
                        // AddOp is writing to regfile B.
                        AddMuxDest = VC4_QPU_ALU_REG_B;
                    }
                    else
                    {
                        AddMuxDest = VC4_QPU_ALU_REG_A;
                    }
                }
                else if (VC4_QPU_GET_SIG(QpuInst) == VC4_QPU_SIG_LOAD_IMMEDIATE)
                {
                    assert(i == 1);
                    ImmediateValue1 = ParseImmediate(_AddArgv[i]);
                    bSetImmediateValue1 = true;
                }
                else
                {
                    TCHAR* _Reg = _tcstok_s(_AddArgv[i], szPeriod, &_Pack);
                    AddMuxSrc[i-1] = VC4_QPU_LOOKUP_VALUE(ALU, _Reg);
                    if (AddMuxSrc[i-1] == VC4_QPU_INVALID_VALUE)
                    {
                        AddRaddr[i-1] = VC4_QPU_LOOKUP_ADDR_VALUE(RADDR, 0, _Reg, &AddRegfileExchangeable[i]);
                        if (AddRaddr[i-1] == VC4_QPU_INVALID_VALUE)
                        {
                            AddRaddr[i-1] = VC4_QPU_LOOKUP_ADDR_VALUE(RADDR, 1, _Reg, &AddRegfileExchangeable[i]);
                            RETURN_WHEN_INVALID(AddRaddr[i-1], TEXT("Invalid AddOp source %s, line %d\n"), _Reg, Line);
                            // AddOp is reading from Regfile B
                            AddMuxSrc[i-1] = VC4_QPU_ALU_REG_B;
                            bAddReadRegfile_B = true;
                        }
                        else
                        {
                            // AddOp is reading from Regfile A
                            AddMuxSrc[i-1] = VC4_QPU_ALU_REG_A;
                            bAddReadRegfile_A = true;
                        }
                    }
                }

                if (_Pack && _Pack[0])
                {
                    if (i == 0) // Pack
                    {
                        if (AddMuxDest != VC4_QPU_ALU_REG_A)
                        {
                            RETURN_ERROR(E_FAIL, TEXT("Pack can be only used to write regfile A at AddOps, line %d\n"), Line);
                        }
                        AddPack = VC4_QPU_LOOKUP_VALUE(PACK_A, _Pack);
                        RETURN_WHEN_INVALID(AddPack, TEXT("Invalid pack at AddOps %s, line %d\n"), _Pack, Line);
                    }
                    else // Unpack
                    {
                        AddUnpack[i-1] = VC4_QPU_LOOKUP_VALUE(PACK_A, _Pack);
                        RETURN_WHEN_INVALID(AddUnpack[i-1], TEXT("Invalid pack at AddOps %s, line %d\n"), _Pack, Line);
                        bAddUseUnPack_R4 = (AddMuxSrc[i-1] == VC4_QPU_ALU_R4) ? true : false;
                    }
                }
            }
        }
    }

    pOpCode = pOpCodeNext;
    if (!IsOpEmpty(pOpCode, &pOpCodeNext))
    {
        TCHAR *pTokenNext;
        TCHAR *pToken = _tcstok_s(pOpCode, szDelimiters, &pTokenNext);

        // Parse instruction
        {
            TCHAR* _MulCond = NULL;
            TCHAR* _MulOp = _tcstok_s(pToken, szPeriod, &_MulCond);
            MulOp = VC4_QPU_LOOKUP_VALUE(OPCODE_MUL, _MulOp);
            RETURN_WHEN_INVALID(MulOp, TEXT("Invalid MulOp %s, line %d\n"), _MulOp, Line);
            MulCond = VC4_QPU_LOOKUP_VALUE(COND, _MulCond);
            RETURN_WHEN_INVALID(MulCond, TEXT("Invalid AddOp Cond %s, line %d\n"), _MulCond, Line);
            if (VC4_QPU_GET_SIG(QpuInst) == VC4_QPU_SIG_LOAD_IMMEDIATE)
            {
                if (MulOp != VC4_QPU_OPCODE_MUL_MOV && MulOp != VC4_QPU_OPCODE_MUL_NOP)
                {
                    RETURN_ERROR(E_INVALIDARG, TEXT("Load immediate must be mov instruction, line %d\n"), Line);
                }
            }
        }
        
        // Parse parameters.
        {
            MulArgc = (VC4_QPU_GET_SIG(QpuInst) == VC4_QPU_SIG_ALU_WITH_RADDR_B) ? 2 : Vc4_QPU_LookUp_MulOp_ArgCount(MulOp);
            assert(MulArgc <= 3);
            TCHAR* _MulArgv[3] = { NULL };
            for (INT i = 0; i < MulArgc; i++)
            {
                _MulArgv[i] = _tcstok_s(NULL, szDelimiters, &pTokenNext);
            }

            // Encode destination and sources
            for (INT i = 0; i < MulArgc; i++)
            {
                TCHAR* _Pack = NULL;
                if (i == 0)
                {
                    TCHAR* _Reg = _tcstok_s(_MulArgv[i], szPeriod, &_Pack);
                    MulWaddr = VC4_QPU_LOOKUP_ADDR_VALUE(WADDR, 1, _Reg, &MulRegfileExchangeable[i]);
                    if (MulWaddr == VC4_QPU_INVALID_VALUE)
                    {
                        MulWaddr = VC4_QPU_LOOKUP_ADDR_VALUE(WADDR, 0, _Reg, &MulRegfileExchangeable[i]);
                        RETURN_WHEN_INVALID(MulWaddr, TEXT("Invalid MulOp destination %s, line %d\n"), _Reg, Line);
                        MulMuxDest = VC4_QPU_ALU_REG_A;
                    }
                    else
                    {
                        MulMuxDest = VC4_QPU_ALU_REG_B;
                    }
                }
                else if (VC4_QPU_GET_SIG(QpuInst) == VC4_QPU_SIG_ALU_WITH_RADDR_B)
                {
                    assert(i == 1);
                    MulMuxSrc[i-1] = VC4_QPU_ALU_REG_B;
                    MulRaddr[i-1] = ParseSmallImmediate(_MulArgv[i]);
                }
                else if (VC4_QPU_GET_SIG(QpuInst) == VC4_QPU_SIG_LOAD_IMMEDIATE)
                {
                    assert(i == 1);
                    ImmediateValue2 = ParseImmediate(_MulArgv[i]);
                    bSetImmediateValue2 = true;
                }
                else
                {
                    TCHAR* _Reg = _tcstok_s(_MulArgv[i], szPeriod, &_Pack);
                    MulMuxSrc[i-1] = VC4_QPU_LOOKUP_VALUE(ALU, _Reg);
                    if (MulMuxSrc[i-1] == VC4_QPU_INVALID_VALUE)
                    {
                        // If AddOps already took regfile A, try B first, otherwise A first.
                        MulRaddr[i-1] = VC4_QPU_LOOKUP_ADDR_VALUE(RADDR, bAddReadRegfile_A ? 1 : 0, _Reg, &MulRegfileExchangeable[i]);
                        if (MulRaddr[i-1] == VC4_QPU_INVALID_VALUE)
                        {
                            MulRaddr[i-1] = VC4_QPU_LOOKUP_ADDR_VALUE(RADDR, bAddReadRegfile_A ? 0 : 1, _Reg, &MulRegfileExchangeable[i]);
                            RETURN_WHEN_INVALID(MulRaddr[i-1], TEXT("Invalid MulOp source %s, line %d\n"), _Reg, Line);
                            MulMuxSrc[i-1] = bAddReadRegfile_A ? VC4_QPU_ALU_REG_A : VC4_QPU_ALU_REG_B;
                        }
                        else
                        {
                            MulMuxSrc[i-1] = bAddReadRegfile_A ? VC4_QPU_ALU_REG_B : VC4_QPU_ALU_REG_A;
                        }
                    }
                }

                if (_Pack && _Pack[0])
                {
                    if (i == 0) // Pack
                    {
                        // Try MulOp unpack first.
                        MulPack = VC4_QPU_LOOKUP_VALUE(PACK_MUL, _Pack);
                        if (MulPack == VC4_QPU_INVALID_VALUE)
                        {
                            // if not, then see regfile A pack, only if writing to regfile A.
                            if (MulMuxDest == VC4_QPU_ALU_REG_A)
                            {
                                MulPack = VC4_QPU_LOOKUP_VALUE(PACK_A, _Pack);
                                bMulUsePack_A = true;
                            }
                        }
                        RETURN_WHEN_INVALID(MulPack, TEXT("Invalid pack at MulOps %s, line %d\n"), _Pack, Line);
                    }
                    else // Unpack
                    {
                        MulUnpack[i-1] = VC4_QPU_LOOKUP_VALUE(PACK_A, _Pack);
                        RETURN_WHEN_INVALID(MulUnpack[i-1], TEXT("Invalid pack at AddOps %s, line %d\n"), _Pack, Line);
                        bMulUseUnPack_R4 = (MulMuxSrc[i-1] == VC4_QPU_ALU_R4) ? true : false;
                    }
                }
            }
        }
    }

    // Remap "mov" to proper opcode.
    if (AddOp == VC4_QPU_OPCODE_ADD_MOV)
    {
        AddOp = VC4_QPU_OPCODE_ADD_OR;
    }
    if (MulOp == VC4_QPU_OPCODE_MUL_MOV)
    {
        MulOp = VC4_QPU_OPCODE_MUL_V8MIN;
    }

    // If OpCode is NOP, condition is changed to never.
    if (AddOp == VC4_QPU_OPCODE_ADD_NOP)
    {
        AddCond = VC4_QPU_COND_NEVER;
    }
    if (MulOp == VC4_QPU_OPCODE_MUL_NOP)
    {
        MulCond = VC4_QPU_COND_NEVER;
    }
    
    // Any instruction only with 2 parameters, copy [1] to [2].
    if (AddArgc <= 2)
    {
        AddMuxSrc[1] = AddMuxSrc[0];
        AddRaddr[1] = AddRaddr[0];
        AddUnpack[1] = AddUnpack[0];
    }
    if (MulArgc <= 2)
    {
        MulMuxSrc[1] = MulMuxSrc[0];
        MulRaddr[1] = MulRaddr[0];
        MulUnpack[1] = MulUnpack[0];
    }

    // If Signature is load_sm, then MulOp must be 'mov'
    if ((VC4_QPU_GET_SIG(QpuInst) == VC4_QPU_SIG_ALU_WITH_RADDR_B) &&
        (MulOp != VC4_QPU_OPCODE_MUL_V8MIN))
    {
        RETURN_ERROR(E_FAIL, _TEXT("load_sm sig must have 'mov' or 'or' at MulOp, line %d\n"), Line);
    }

    // AddOp and MulOp can't write to same location.
    if ((AddMuxDest == MulMuxDest) && 
        (AddWaddr == MulWaddr) && AddRegfileExchangeable[0])
    {
        RETURN_ERROR(E_FAIL, _TEXT("Both of AddOp and MulOp writes to same destination, line %d\n"), Line);
    }

    // Determine who to use regfile A destination.
    // First, check who uses regfile A pack.
    if (AddPack && bMulUsePack_A)
    {
        RETURN_ERROR(E_FAIL, _TEXT("Both of AddOp and MulOp use regfile A pack, line %d\n"), Line);
    }
    else if (AddPack == 0 && bMulUsePack_A)
    {
        // MulOp wants to write regfile A pack
    }
    else if (AddPack)
    {
        assert(bMulUsePack_A == false);
        // AddOp wants to write regfile A pack.
        bAddUsePack_A = true;
    }
    assert((bAddUsePack_A && bMulUsePack_A) == false);

    if ((MulPack && (bMulUsePack_A == false)) && bAddUsePack_A)
    {
        RETURN_ERROR(E_FAIL, _TEXT("Mul pack and Regfile A pack can't be use at same time, line %d\n"), Line);
    }
    
    if ((bMulUseUnPack_R4 || bAddUseUnPack_R4 || bAddUsePack_A || bMulUsePack_A) &&
        (bMulUseUnPack_R4 || bAddUseUnPack_R4) == (bAddUsePack_A || bMulUsePack_A))
    {
        RETURN_ERROR(E_FAIL, _TEXT("R4 unpack and Regfile A pack can't be use at same time, line %d\n"), Line);
    }
        
    if (bAddUsePack_A && (AddMuxDest != VC4_QPU_ALU_REG_A))
    {
        RETURN_ERROR(E_FAIL, _TEXT("AddOp wants to use regfile A pack, but it's not writing to regfile A, line %d\n"), Line);
    }

    if (bMulUsePack_A && (MulMuxDest != VC4_QPU_ALU_REG_A))
    {
        RETURN_ERROR(E_FAIL, _TEXT("AddOp wants to use regfile A pack, but it's not writing to regfile A, line %d\n"), Line);

// TODO: regfile reassignment.
// if (!MulRegfileExchangeable[0])
// {
//    RETURN_ERROR(E_FAIL, _TEXT("MulOp wants to use regfile A pack, but it's writing to regfile only available on B, line %d\n"), Line);
// }
    }

    if (MulPack && (bMulUsePack_A == false))
    {
        // MulPack is used, set PM bit.
        bPackUnpackSelect = true;
        Pack = MulPack;
    }
    else if (bMulUseUnPack_R4 || bAddUseUnPack_R4)
    {
        bPackUnpackSelect = true;
    }
    else if (AddPack)
    {
        assert(bPackUnpackSelect == false);
        Pack = AddPack;
    }

    // Validate Unpack.
    if (VC4_QPU_GET_SIG(QpuInst) != VC4_QPU_SIG_LOAD_IMMEDIATE)
    {
        INT _Mux[4] = { AddMuxSrc[0], AddMuxSrc[1], MulMuxSrc[0], MulMuxSrc[1] };
        INT _Unpack[4] = { AddUnpack[0], AddUnpack[1], MulUnpack[0], MulUnpack[1] };
        for (INT _i = 0; _i < 4; _i++)
        {
            if (Unpack != 0 && Unpack != _Unpack[_i])
            {
                RETURN_ERROR(E_FAIL, _TEXT("Unpack is not consistent, line %d\n"), Line);
            }
            if (_Unpack[_i])
            {
                if (bPackUnpackSelect && _Mux[_i] != VC4_QPU_ALU_R4)
                {
                    // when PM bit set, only r4 unpack can be performed.
                    RETURN_ERROR(E_FAIL, _TEXT("Only r4 can perform unpack with PM bit set, line %d\n"), Line);
                }
                Unpack = _Unpack[_i];
            }
        }
    }

    // Set and validate RaddrA/B.
    if (VC4_QPU_GET_SIG(QpuInst) != VC4_QPU_SIG_LOAD_IMMEDIATE)
    {
        INT _Mux[4] = { AddMuxSrc[0], AddMuxSrc[1], MulMuxSrc[0], MulMuxSrc[1] };
        INT _Raddr[4] = { AddRaddr[0], AddRaddr[1], MulRaddr[0], MulRaddr[1] };
        for (INT _i = 0; _i < 4; _i++)
        {
            if (_Mux[_i] == VC4_QPU_ALU_REG_A)
            {
                if (_Raddr[_i] != VC4_QPU_RADDR_NOP && (RaddrA != _Raddr[_i]))
                {
                    if (RaddrA == VC4_QPU_INVALID_VALUE)
                    {
                        RaddrA = _Raddr[_i];
                    }
                    else
                    {
                        RETURN_ERROR(E_FAIL, _TEXT("Conflit usage for RaddrA, line %d"), Line);
                    }
                }
            }
            else if (_Mux[_i] == VC4_QPU_ALU_REG_B)
            {
                if (_Raddr[_i] != VC4_QPU_RADDR_NOP && (RaddrB != _Raddr[_i]))
                {
                    if (RaddrB == VC4_QPU_INVALID_VALUE)
                    {
                        RaddrB = _Raddr[_i];
                    }
                    else
                    {
                        RETURN_ERROR(E_FAIL, _TEXT("Conflit usage for RaddrB, line %d"), Line);
                    }
                }
            }
        }

        if (RaddrA == VC4_QPU_INVALID_VALUE)
        {
            RaddrA = VC4_QPU_RADDR_NOP;
        }
        if (RaddrB == VC4_QPU_INVALID_VALUE)
        {
            RaddrB = VC4_QPU_RADDR_NOP;
        }
    }

    // Set write swap,
    bWriteSwap = (AddMuxDest == VC4_QPU_ALU_REG_B) || (MulMuxDest == VC4_QPU_ALU_REG_A);

    // TODO:
    bSetFlags = false;

    // Build instruction
    if (VC4_QPU_GET_SIG(QpuInst) == VC4_QPU_SIG_LOAD_IMMEDIATE)
    {
        if (bSetImmediateValue1 && bSetImmediateValue2 && (ImmediateValue1 != ImmediateValue2))
        {
            RETURN_ERROR(E_INVALIDARG, TEXT("ImmedicateValue is not consistent, Line %d\n"), Line);
        }
        VC4_QPU_SET_IMMEDIATE_TYPE(QpuInst, VC4_QPU_IMMEDIATE_TYPE_32);
        VC4_QPU_SET_PM(QpuInst, bPackUnpackSelect);
        VC4_QPU_SET_PACK(QpuInst, Pack);
        VC4_QPU_SET_COND_ADD(QpuInst, AddCond);
        VC4_QPU_SET_COND_MUL(QpuInst, MulCond);
        VC4_QPU_SET_SETFLAGS(QpuInst, bSetFlags);
        VC4_QPU_SET_WRITESWAP(QpuInst, bWriteSwap);
        VC4_QPU_SET_WADDR_ADD(QpuInst, AddWaddr);
        VC4_QPU_SET_WADDR_MUL(QpuInst, MulWaddr);
        VC4_QPU_SET_IMMEDIATE_32(QpuInst, ImmediateValue1 ? ImmediateValue1 : ImmediateValue2);
    }   
    else
    {
        VC4_QPU_SET_UNPACK(QpuInst, Unpack);
        VC4_QPU_SET_PM(QpuInst, bPackUnpackSelect);
        VC4_QPU_SET_PACK(QpuInst, Pack);
        VC4_QPU_SET_COND_ADD(QpuInst, AddCond);
        VC4_QPU_SET_COND_MUL(QpuInst, MulCond);
        VC4_QPU_SET_SETFLAGS(QpuInst, bSetFlags);
        VC4_QPU_SET_WRITESWAP(QpuInst, bWriteSwap);
        VC4_QPU_SET_WADDR_ADD(QpuInst, AddWaddr);
        VC4_QPU_SET_WADDR_MUL(QpuInst, MulWaddr);
        VC4_QPU_SET_OPCODE_ADD(QpuInst, AddOp);
        VC4_QPU_SET_OPCODE_MUL(QpuInst, MulOp);
        VC4_QPU_SET_RADDR_A(QpuInst, RaddrA);
        VC4_QPU_SET_RADDR_B(QpuInst, RaddrB);
        VC4_QPU_SET_ADD_A(QpuInst, AddMuxSrc[0]);
        VC4_QPU_SET_ADD_B(QpuInst, AddMuxSrc[1]);
        VC4_QPU_SET_MUL_A(QpuInst, MulMuxSrc[0]);
        VC4_QPU_SET_MUL_B(QpuInst, MulMuxSrc[1]);
    }

    return S_OK;
}

void Printer(void *pFile, const TCHAR* szStr, int Line, void* m_pCustomCtx)
{
    _ftprintf_s(stderr, TEXT("// %s"), szStr);
}

int _tmain(int argc, TCHAR *argv[])
{
    boolean ShowDisassemble = false;
    boolean ShowOriginal = false;
    boolean Disassemble = false;

    if (argc <= 1)
    {
        RETURN_ERROR(E_INVALIDARG, _TEXT("%s\n"), TEXT("Usage: Vc4Asm ASMFile [OUTFile]\n"));
    }

    for (int i = 1; i < argc; i++)
    {
        if (*argv[i] == TEXT('-'))
        {
            if (_tcsicmp(argv[i], TEXT("-ShowDisassemble")) == 0)
            {
                ShowDisassemble = true;
            }
            else if (_tcsicmp(argv[i], TEXT("-ShowOriginal")) == 0)
            {
                ShowOriginal = true;
            }
            else if (_tcsicmp(argv[i], TEXT("-Disassemble")) == 0)
            {
                Disassemble = true;
            }
            else
            {
                RETURN_ERROR(E_INVALIDARG, _TEXT("Invalid Option %s\n"), argv[i]);
            }
        }
        else if (szAsmFile[0] == NULL)
        {
            _tcscpy_s(szAsmFile, argv[i]);
        }
        else
        {
            RETURN_ERROR(E_INVALIDARG, TEXT("Invalid argument - %s\n"), argv[i]);
        }
    }

    FILE* fpASM = NULL;

    errno_t err = _tfopen_s(&fpASM, szAsmFile, _TEXT("r"));
    if (err)
    {
        RETURN_ERROR(E_INVALIDARG, TEXT("Invalid Asm file - %s\n"), szAsmFile);
    }

    UINT Line = 0;
    HRESULT hr = S_OK;
    while (_fgetts(szBuff, _countof(szBuff), fpASM) && (hr == S_OK))
    {
        VC4_QPU_INSTRUCTION QpuInst = 0;
        Line++;

        if (Disassemble)
        {
            TCHAR *pTokenNext = NULL;
            TCHAR *pValue = _tcstok_s(szBuff, szDelimiters, &pTokenNext);
            do
            {
                INT c = 0;
                if (pValue[0] == '0' && pValue[1] == 'x')
                {
                    c += 2; // Skip 0x.
                }
                
                size_t cLen = _tcslen(pValue);
                if (cLen == 16 + c)
                {
                    if (QpuInst)
                    {
                        _ftprintf_s(stderr, TEXT("Invalid immediate format %s, line %d\n"), pValue, Line);
                        hr = E_INVALIDARG;
                        break;
                    }
                    QpuInst = (VC4_QPU_INSTRUCTION) _tcstoull(pValue, NULL, 16);
                    break;
                }
                else if (cLen == 8 + c)
                {
                    VC4_QPU_INSTRUCTION value = (VC4_QPU_INSTRUCTION) _tcstoul(pValue, NULL, 16);
                    if (QpuInst)
                    {
                        assert((0xffffffff00000000ULL & QpuInst) == 0);
                        QpuInst |= (value << 32);
                        break;
                    }
                    else
                    {
                        QpuInst = value;
                    }
                    pValue = _tcstok_s(NULL, szDelimiters, &pTokenNext);
                }
                else
                {
                    _ftprintf_s(stderr, TEXT("Invalid immediate format %s, line %d\n"), pValue, Line);
                    hr = E_INVALIDARG;
                    break;
                }
            }
            while (true);
        }
        else
        {
            // Copy original before token break it.
            _tcscpy_s(szOriginal, szBuff);

            // Parse Signature if provided.
            UINT Sig = VC4_QPU_SIG_NO_SIGNAL;
            TCHAR *pOpCodeNext = NULL;
            if (!IsOpEmpty(szBuff, &pOpCodeNext))
            {
                TCHAR *pTokenNext;
                TCHAR *pToken = _tcstok_s(szBuff, szDelimiters, &pTokenNext);

                // Parse Signature if provided.
                Sig = VC4_QPU_LOOKUP_VALUE(SIG, pToken);
                if (Sig == VC4_QPU_INVALID_VALUE)
                {
                    _ftprintf_s(stderr, TEXT("Invalid Signature %s, line %d\n"), pToken, Line);
                    break;
                }
            }
            VC4_QPU_SET_SIG(QpuInst, Sig);

            switch (Sig)
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
                hr = ParseALUInstruction(QpuInst, pOpCodeNext, Line);
                break;
            case VC4_QPU_SIG_BRANCH:
                _ftprintf_s(stderr, TEXT("Branch is not supported, line %d\n"), Line);
                hr = E_NOTIMPL;
            default:
                assert(false); // this should never happen.
                break;
            }

            if (hr == S_OK)
            {
                _ftprintf_s(stdout, TEXT("0x%016llx,\t"), QpuInst);
                if (ShowOriginal)
                {
                    TCHAR *p = _tcschr(szOriginal, TEXT('\n'));
                    if (p) *p = NULL;
                    _ftprintf_s(stdout, TEXT("// %s \t"), szOriginal);
                }
            }
            else
            {
                break;
            }
        }

        if ((hr == S_OK) && (ShowDisassemble || Disassemble))
        {
            Vc4Disassemble(&QpuInst, sizeof(QpuInst), Printer);
        }
        _ftprintf(stdout, TEXT("\n"));
    }

    if (fpASM)
    {
        fclose(fpASM);
    }

    return 0;
}

