#pragma once

#include "..\roscommon\Vc4Qpu.h"
#include "roscompilerdebug.h"

#if VC4

class Vc4Shader;
class Vc4ShaderStorage;
class Vc4Instruction;

enum Vc4InstructionType
{
    vc4_alu,
    vc4_alu_small_immediate,
    vc4_branch,
    vc4_load_immediate_32,
    vc4_load_immediate_per_element_signed,
    vc4_load_immediate_per_element_unsigned,
    vc4_semaphore,
};

struct Vc4RegisterFlags
{
    union
    {
        struct
        {
            uint32_t valid : 1;
            uint32_t require_linear_conversion : 1;
            uint32_t position : 1;
            uint32_t temp : 1;
            uint32_t color : 1;
            uint32_t swap_color_channel : 1;
            uint32_t packed : 1;
            uint32_t immediate : 1;
            uint32_t uniform : 1;
            uint32_t linkage : 1;
        };
        uint32_t value;
    };
};

struct Vc4Register
{
    friend class Vc4Shader;
    // friend void Vc4Shader::HLSL_ParseDecl();
    friend class Vc4Instruction;

    Vc4Register() :
        value(0)
    {
        this->flags.value = 0;
    }

    Vc4Register(uint8_t _mux, uint8_t _addr = VC4_QPU_WADDR_NOP) :
        addr(_addr),
        mux(_mux),
        swizzleMask(D3D10_SB_OPERAND_4_COMPONENT_MASK_X | D3D10_SB_OPERAND_4_COMPONENT_MASK_Y | D3D10_SB_OPERAND_4_COMPONENT_MASK_Z | D3D10_SB_OPERAND_4_COMPONENT_MASK_W)
    {
        this->flags.value = 0;
        this->flags.valid = true;
        if (_addr == VC4_QPU_RADDR_UNIFORM)
        {
            this->flags.uniform = true;
        }
    }

    void SetMux(uint8_t _mux)
    {
        assert(this->flags.immediate == false);
        this->mux = _mux;
    }

    void SetModifier(D3D10_SB_OPERAND_MODIFIER _modifier)
    {
        assert(this->flags.immediate == false);
        assert(_modifier == D3D10_SB_OPERAND_MODIFIER_NONE ||
            _modifier == D3D10_SB_OPERAND_MODIFIER_NEG ||
            _modifier == D3D10_SB_OPERAND_MODIFIER_ABS ||
            _modifier == D3D10_SB_OPERAND_MODIFIER_ABSNEG);
        this->modifier = (uint8_t)_modifier;
    }

    void SetImmediateI(uint32_t _i)
    {
        this->flags.valid = true;
        this->flags.immediate = true;
        this->i = _i;
    }

    void SetImmediateF(float _f)
    {
        this->flags.valid = true;
        this->flags.immediate = true;
        this->f = _f;
    }

    Vc4RegisterFlags GetFlags()
    {
        return this->flags;
    }

    uint8_t GetMux()
    {
        assert(this->flags.immediate == false);
        return this->mux;
    }

    uint8_t GetAddr()
    {
        assert(this->flags.immediate == false);
        return this->addr;
    }

    uint8_t GetSwizzleMask()
    {
        assert(this->flags.immediate == false);
        return this->swizzleMask;
    }

    uint8_t GetPack(uint8_t swizzleIndex)
    {
        if (this->flags.packed)
        {
            if (this->flags.swap_color_channel)
            {
                assert(this->flags.color);
                if (swizzleIndex == 3) // keep alpha at 8d.
                {
                    return VC4_QPU_PACK_MUL_8d;
                }
                else
                {
                    return VC4_QPU_PACK_MUL_8c - swizzleIndex;
                }
            }
            else
            {
                return VC4_QPU_PACK_MUL_8a + swizzleIndex;
            }
        }
        else
        {
            return VC4_QPU_PACK_A_32;
        }
    }

private:

    Vc4RegisterFlags flags;
    union
    {
        struct
        {
            uint8_t addr;
            uint8_t mux;
            uint8_t swizzleMask;
            uint8_t modifier;
        }; // register
        union
        {
            C_ASSERT(sizeof(float) == sizeof(uint32_t));
            float f;
            uint32_t i;
        }; // immediate
        uint32_t value;
    };
};

class Vc4Instruction
{
public:

    Vc4Instruction(Vc4InstructionType _Type = vc4_alu) :
        Type(_Type),
        Sig(VC4_QPU_SIG_NO_SIGNAL)
    {
        memset(&this->Instruction, 0, sizeof(this->Instruction));
        switch (_Type)
        {
        case vc4_alu:
        case vc4_alu_small_immediate:
            if (_Type == vc4_alu_small_immediate)
            {
                Vc4_Sig(VC4_QPU_SIG_ALU_WITH_RADDR_B);
            }
            memset(&this->ALU, 0, sizeof(this->ALU));
            this->ALU.op_add = VC4_QPU_OPCODE_ADD_NOP;
            this->ALU.op_mul = VC4_QPU_OPCODE_MUL_NOP;
            this->ALU.cond_add = VC4_QPU_COND_NEVER;
            this->ALU.cond_mul = VC4_QPU_COND_NEVER;
            this->ALU.waddr_add = VC4_QPU_WADDR_NOP;
            this->ALU.waddr_mul = VC4_QPU_WADDR_NOP;
            this->ALU.raddr_a = VC4_QPU_RADDR_NOP;
            this->ALU.raddr_b = VC4_QPU_RADDR_NOP;
            break;
        case vc4_load_immediate_32:
            memset(&this->LOAD32, 0, sizeof(this->ALU));
            this->LOAD32.cond_add = VC4_QPU_COND_NEVER;
            this->LOAD32.cond_mul = VC4_QPU_COND_NEVER;
            this->LOAD32.waddr_add = VC4_QPU_WADDR_NOP;
            this->LOAD32.waddr_mul = VC4_QPU_WADDR_NOP;
            break;
        default:
            assert(false);
        }
    }
    ~Vc4Instruction() { ; }

    void Vc4_Sig(uint8_t sig)
    {
        this->Sig = sig;
    }

    void Vc4_a_Inst(uint8_t opcode, Vc4Register dst, Vc4Register src, uint8_t cond)
    {
        assert(dst.flags.valid);
        assert(src.flags.valid && (src.flags.immediate == false));
        assert(this->Type == vc4_alu || this->Type == vc4_alu_small_immediate);
        this->ALU.op_add = opcode;
        this->ALU.cond_add = cond;
        this->ALU.waddr_add = dst.addr;
        if (dst.mux == VC4_QPU_ALU_REG_B)
        {
            this->ALU.ws = true;
        }
        if (src.mux == VC4_QPU_ALU_REG_A)
        {
            this->ALU.raddr_a = src.addr;
        }
        else if (src.mux == VC4_QPU_ALU_REG_B)
        {
            this->ALU.raddr_b = src.addr;
        }
        this->ALU.add_a = this->ALU.add_b = src.mux;
    }

    void Vc4_a_Inst(uint8_t opcode, Vc4Register dst, Vc4Register src1, Vc4Register src2, uint8_t cond)
    {
        assert(dst.flags.valid);
        assert(src1.flags.valid && (src1.flags.immediate == false));
        assert(src2.flags.valid && (src2.flags.immediate == false));
        assert(this->Type == vc4_alu || this->Type == vc4_alu_small_immediate);
        this->ALU.op_add = opcode;
        this->ALU.cond_add = cond;
        this->ALU.waddr_add = dst.addr;
        if (dst.mux == VC4_QPU_ALU_REG_B)
        {
            this->ALU.ws = true;
        }
        if (src1.mux == VC4_QPU_ALU_REG_A)
        {
            assert((src2.mux != VC4_QPU_ALU_REG_A) || (src2.addr == src1.addr));
            this->ALU.raddr_a = src1.addr;
        }
        else if (src1.mux == VC4_QPU_ALU_REG_B)
        {
            assert((src2.mux != VC4_QPU_ALU_REG_B) || (src2.addr == src1.addr));
            this->ALU.raddr_b = src1.addr;
        }
        if (src2.mux == VC4_QPU_ALU_REG_A)
        {
            assert((src1.mux != VC4_QPU_ALU_REG_A) || (src1.addr == src2.addr));
            this->ALU.raddr_a = src2.addr;
        }
        else if (src2.mux == VC4_QPU_ALU_REG_B)
        {
            assert((src1.mux != VC4_QPU_ALU_REG_B) || (src1.addr == src2.addr));
            this->ALU.raddr_b = src2.addr;
        }
        this->ALU.add_a = src1.mux;
        this->ALU.add_b = src2.mux;
    }

    void Vc4_a_FADD(Vc4Register dst, Vc4Register src1, Vc4Register src2, uint8_t cond = VC4_QPU_COND_ALWAYS)
    {
        Vc4_a_Inst(VC4_QPU_OPCODE_ADD_FADD, dst, src1, src2, cond);
    }

    void Vc4_a_FMAX(Vc4Register dst, Vc4Register src1, Vc4Register src2, uint8_t cond = VC4_QPU_COND_ALWAYS)
    {
        Vc4_a_Inst(VC4_QPU_OPCODE_ADD_FMAX, dst, src1, src2, cond);
    }

    void Vc4_a_FMAXABS(Vc4Register dst, Vc4Register src1, Vc4Register src2, uint8_t cond = VC4_QPU_COND_ALWAYS)
    {
        Vc4_a_Inst(VC4_QPU_OPCODE_ADD_FMAX_ABS, dst, src1, src2, cond);
    }

    void Vc4_a_FMIN(Vc4Register dst, Vc4Register src1, Vc4Register src2, uint8_t cond = VC4_QPU_COND_ALWAYS)
    {
        Vc4_a_Inst(VC4_QPU_OPCODE_ADD_FMIN, dst, src1, src2, cond);
    }
    
    void Vc4_a_FTOI(Vc4Register dst, Vc4Register src, uint8_t cond = VC4_QPU_COND_ALWAYS)
    {
        Vc4_a_Inst(VC4_QPU_OPCODE_ADD_FTOI, dst, src, cond);
    }

    void Vc4_a_IADD(Vc4Register dst, Vc4Register src1, Vc4Register src2, uint8_t cond = VC4_QPU_COND_ALWAYS)
    {
        Vc4_a_Inst(VC4_QPU_OPCODE_ADD_ADD, dst, src1, src2, cond);
    }

    void Vc4_a_MOV(Vc4Register dst, Vc4Register src, uint8_t cond = VC4_QPU_COND_ALWAYS)
    {
        Vc4_a_Inst(VC4_QPU_OPCODE_ADD_OR, dst, src, cond);
    }

    void Vc4_a_LOAD32(Vc4Register dst, Vc4Register src, uint8_t cond = VC4_QPU_COND_ALWAYS)
    {
        assert(dst.flags.valid);
        assert(src.flags.valid && src.flags.immediate);
        assert(this->Type == vc4_load_immediate_32);
        this->Sig = VC4_QPU_SIG_LOAD_IMMEDIATE;
        this->LOAD32.cond_add = cond;
        this->LOAD32.waddr_add = dst.addr;
        if (dst.mux == VC4_QPU_ALU_REG_B)
        {
            this->LOAD32.ws = true;
        }
        this->LOAD32.immediate = src.i;
    }

    void Vc4_a_Pack(uint8_t pack)
    {
        switch (this->Type)
        {
        case vc4_alu:
        case vc4_alu_small_immediate:
            assert(this->ALU.pack == 0);
            this->ALU.pack = pack;
            if (pack)
            {
                assert(this->ALU.pm == false);
            }
            break;
        case vc4_load_immediate_32:
            assert(this->LOAD32.pack == 0);
            this->LOAD32.pack = pack;
            if (pack)
            {
                assert(this->LOAD32.pm == false);
            }
            break;
        default:
            assert(false);
        }
    }

    void Vc4_a_Unpack(uint8_t unpack, boolean pm)
    {
        switch (this->Type)
        {
        case vc4_alu:
        case vc4_alu_small_immediate:
            assert(this->ALU.unpack == 0);
            this->ALU.unpack = unpack;
            this->ALU.pm = pm;
            break;
        case vc4_load_immediate_32:
        default:
            assert(false);
        }
    }

    void Vc4_m_Inst(uint8_t opcode, Vc4Register dst, Vc4Register src, uint8_t cond)
    {
        assert(dst.flags.valid);
        assert(src.flags.valid && (src.flags.immediate == false));
        assert(this->Type == vc4_alu || this->Type == vc4_alu_small_immediate);
        this->ALU.op_mul = opcode;
        this->ALU.cond_mul = cond;
        this->ALU.waddr_mul = dst.addr;
        if (dst.mux == VC4_QPU_ALU_REG_A)
        {
            this->ALU.ws = true;
        }
        if (src.mux == VC4_QPU_ALU_REG_A)
        {
            this->ALU.raddr_a = src.addr;
        }
        else if (src.mux == VC4_QPU_ALU_REG_B)
        {
            this->ALU.raddr_b = src.addr;
        }
        this->ALU.mul_a = this->ALU.mul_b = src.mux;
    }

    void Vc4_m_Inst(uint8_t opcode, Vc4Register dst, Vc4Register src1, Vc4Register src2, uint8_t cond)
    {
        assert(dst.flags.valid);
        assert(src1.flags.valid && (src1.flags.immediate == false));
        assert(src2.flags.valid && (src2.flags.immediate == false));
        assert(this->Type == vc4_alu || this->Type == vc4_alu_small_immediate);
        this->ALU.op_mul = opcode;
        this->ALU.cond_mul = cond;
        this->ALU.waddr_mul = dst.addr;
        if (dst.mux == VC4_QPU_ALU_REG_A)
        {
            this->ALU.ws = true;
        }
        if (src1.mux == VC4_QPU_ALU_REG_A)
        {
            assert((src2.mux != VC4_QPU_ALU_REG_A) || (src2.addr == src1.addr));
            this->ALU.raddr_a = src1.addr;
        }
        else if (src1.mux == VC4_QPU_ALU_REG_B)
        {
            assert((src2.mux != VC4_QPU_ALU_REG_B) || (src2.addr == src1.addr));
            this->ALU.raddr_b = src1.addr;
        }
        if (src2.mux == VC4_QPU_ALU_REG_A)
        {
            assert((src1.mux != VC4_QPU_ALU_REG_A) || (src1.addr == src2.addr));
            this->ALU.raddr_a = src2.addr;
        }
        else if (src2.mux == VC4_QPU_ALU_REG_B)
        {
            assert((src1.mux != VC4_QPU_ALU_REG_B) || (src1.addr == src2.addr));
            this->ALU.raddr_b = src2.addr;
        }
        this->ALU.mul_a = src1.mux;
        this->ALU.mul_b = src2.mux;
    }

    void Vc4_m_FMUL(Vc4Register dst, Vc4Register src1, Vc4Register src2, uint8_t cond = VC4_QPU_COND_ALWAYS)
    {
        Vc4_m_Inst(VC4_QPU_OPCODE_MUL_FMUL, dst, src1, src2, cond);
    }

    void Vc4_m_MOV(Vc4Register dst, Vc4Register src, uint8_t cond = VC4_QPU_COND_ALWAYS)
    {
        Vc4_m_Inst(VC4_QPU_OPCODE_MUL_V8MIN, dst, src, cond);
    }

    void Vc4_m_LOAD32(Vc4Register dst, Vc4Register src, uint8_t cond = VC4_QPU_COND_ALWAYS)
    {
        assert(dst.flags.valid);
        assert(src.flags.valid && src.flags.immediate);
        assert(this->Type == vc4_load_immediate_32);
        this->Sig = VC4_QPU_SIG_LOAD_IMMEDIATE;
        this->LOAD32.cond_mul = cond;
        this->LOAD32.waddr_mul = dst.addr;
        if (dst.mux == VC4_QPU_ALU_REG_A)
        {
            this->LOAD32.ws = true;
        }
        this->LOAD32.immediate = src.i;
    }
    
    void Vc4_m_Pack(uint8_t pack)
    {
        switch (this->Type)
        {
        case vc4_alu:
        case vc4_alu_small_immediate:
            assert(this->ALU.pack == 0);
            this->ALU.pack = pack;
            if (pack)
            {
                this->ALU.pm = true;
            }
            break;
        case vc4_load_immediate_32:
            assert(this->LOAD32.pack == 0);
            this->LOAD32.pack = pack;
            if (pack)
            {
                this->LOAD32.pm = true;
            }
            break;
        default:
            assert(false);
        }
    }
    
    void Vc4_m_Unpack(uint8_t unpack, boolean pm)
    {
        switch (this->Type)
        {
        case vc4_alu:
        case vc4_alu_small_immediate:
            assert(this->ALU.unpack == 0);
            this->ALU.unpack = unpack;
            this->ALU.pm = pm;
            break;
        case vc4_load_immediate_32:
        default:
            assert(false);
        }
    }

    VC4_QPU_INSTRUCTION Build();
    void Emit(Vc4ShaderStorage *Storage);
    
private:
    
    Vc4InstructionType Type;
    uint8_t Sig;
    union
    {
        struct
        {
            uint8_t unpack;
            uint8_t pack;
            uint8_t cond_add;
            uint8_t cond_mul;
            uint8_t waddr_add;
            uint8_t waddr_mul;
            uint8_t op_mul;
            uint8_t op_add;
            uint8_t raddr_a;
            uint8_t raddr_b;
            uint8_t add_a;
            uint8_t add_b;
            uint8_t mul_a;
            uint8_t mul_b;
            boolean pm;
            boolean sf;
            boolean ws;
        } ALU;

        struct
        {
            uint8_t pack;
            uint8_t cond_add;
            uint8_t cond_mul;
            uint8_t waddr_add;
            uint8_t waddr_mul;
            uint32_t immediate;
            boolean pm;
            boolean sf;
            boolean ws;
        } LOAD32;
    };

    VC4_QPU_INSTRUCTION Instruction;
};

#endif // VC4
