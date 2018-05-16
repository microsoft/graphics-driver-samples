#include "precomp.h"
#include "roscompiler.h"

#if VC4

VC4_QPU_INSTRUCTION Vc4Instruction::Build()
{
    switch (this->Type)
    {
    case vc4_alu:
    case vc4_alu_small_immediate:
        assert(
            ((this->Type == vc4_alu) &&
             (this->Sig == VC4_QPU_SIG_NO_SIGNAL ||
              this->Sig == VC4_QPU_SIG_PROGRAM_END ||
              this->Sig == VC4_QPU_SIG_WAIT_FOR_SCOREBOARD ||
              this->Sig == VC4_QPU_SIG_SCOREBOARD_UNBLOCK ||
              this->Sig == VC4_QPU_SIG_LOAD_TMU0 ||
              this->Sig == VC4_QPU_SIG_LOAD_TMU1)) 
            ||
            ((this->Type == vc4_alu_small_immediate) && 
             (this->Sig == VC4_QPU_SIG_ALU_WITH_RADDR_B))
            );

        VC4_QPU_SET_SIG(this->Instruction, this->Sig);
        VC4_QPU_SET_UNPACK(this->Instruction, this->ALU.unpack);
        VC4_QPU_SET_PM(this->Instruction, this->ALU.pm);
        VC4_QPU_SET_PACK(this->Instruction, this->ALU.pack);
        VC4_QPU_SET_COND_ADD(this->Instruction, this->ALU.cond_add);
        VC4_QPU_SET_COND_MUL(this->Instruction, this->ALU.cond_mul);
        VC4_QPU_SET_SETFLAGS(this->Instruction, this->ALU.sf);
        VC4_QPU_SET_WRITESWAP(this->Instruction, this->ALU.ws);
        VC4_QPU_SET_WADDR_ADD(this->Instruction, this->ALU.waddr_add);
        VC4_QPU_SET_WADDR_MUL(this->Instruction, this->ALU.waddr_mul);
        VC4_QPU_SET_OPCODE_ADD(this->Instruction, this->ALU.op_add);
        VC4_QPU_SET_OPCODE_MUL(this->Instruction, this->ALU.op_mul);
        VC4_QPU_SET_RADDR_A(this->Instruction, this->ALU.raddr_a);
        VC4_QPU_SET_RADDR_B(this->Instruction, this->ALU.raddr_b);
        VC4_QPU_SET_ADD_A(this->Instruction, this->ALU.add_a);
        VC4_QPU_SET_ADD_B(this->Instruction, this->ALU.add_b);
        VC4_QPU_SET_MUL_A(this->Instruction, this->ALU.mul_a);
        VC4_QPU_SET_MUL_B(this->Instruction, this->ALU.mul_b);
        break;

    case vc4_load_immediate_32:
        assert(this->Sig == VC4_QPU_SIG_LOAD_IMMEDIATE);

        VC4_QPU_SET_SIG(this->Instruction, this->Sig);
        VC4_QPU_SET_IMMEDIATE_TYPE(this->Instruction, VC4_QPU_IMMEDIATE_TYPE_32);
        VC4_QPU_SET_PACK(this->Instruction, this->LOAD32.pack);
        VC4_QPU_SET_PM(this->Instruction, this->LOAD32.pm);
        VC4_QPU_SET_COND_ADD(this->Instruction, this->LOAD32.cond_add);
        VC4_QPU_SET_COND_MUL(this->Instruction, this->LOAD32.cond_mul);
        VC4_QPU_SET_SETFLAGS(this->Instruction, this->LOAD32.sf);
        VC4_QPU_SET_WRITESWAP(this->Instruction, this->LOAD32.ws);
        VC4_QPU_SET_WADDR_ADD(this->Instruction, this->LOAD32.waddr_add);
        VC4_QPU_SET_WADDR_MUL(this->Instruction, this->LOAD32.waddr_mul);
        VC4_QPU_SET_IMMEDIATE_32(this->Instruction, this->LOAD32.immediate);
        break;

    default:
        assert(false);
    }

    return this->Instruction;
}

void Vc4Instruction::Emit(Vc4ShaderStorage *Storage)
{
    assert(Storage);
    Storage->Store<VC4_QPU_INSTRUCTION>(this->Build());
}

#endif // VC4
