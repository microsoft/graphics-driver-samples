#pragma once
#include "..\roscommon\Vc4Qpu.h"

#if VC4

class Vc4Disasm : public BaseDisasm
{
public:
    Vc4Disasm() { }
    ~Vc4Disasm() { }
    HRESULT Run(const VC4_QPU_INSTRUCTION* pShader, ULONG Size);

private:
    HRESULT ParseSignature(VC4_QPU_INSTRUCTION Instruction);

    HRESULT ParseALUInstruction(VC4_QPU_INSTRUCTION Instruction);
    HRESULT ParseAddOp(VC4_QPU_INSTRUCTION Instruction);
    HRESULT ParseMulOp(VC4_QPU_INSTRUCTION Instruction);
    
    HRESULT ParseLoadImmInstruction(VC4_QPU_INSTRUCTION Instruction);
    HRESULT ParseSemaphoreInstruction(VC4_QPU_INSTRUCTION Instruction);
    HRESULT ParseBranchInstruction(VC4_QPU_INSTRUCTION Instruction);
};

#endif // VC4
