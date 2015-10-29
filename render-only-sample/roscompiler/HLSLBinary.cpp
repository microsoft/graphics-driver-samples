#include "roscompiler.h"

BOOL IsOpCodeValid(D3D10_SB_OPCODE_TYPE OpCode)
{
    return OpCode < D3D10_SB_NUM_OPCODES;
}

UINT GetNumInstructionOperands(D3D10_SB_OPCODE_TYPE OpCode)
{
    return GetNumInstructionSrcOperands(OpCode) + GetNumInstructionDstOperands(OpCode);
}

UINT GetNumInstructionSrcOperands(D3D10_SB_OPCODE_TYPE OpCode)
{
    AssertAndAssume(IsOpCodeValid(OpCode));
    return g_InstructionInfo[OpCode].m_NumSrcOperands;
}

UINT GetNumInstructionDstOperands(D3D10_SB_OPCODE_TYPE OpCode)
{
    AssertAndAssume(IsOpCodeValid(OpCode));
    return g_InstructionInfo[OpCode].m_NumDstOperands;
}

D3D11_SB_OPCODE_CLASS GetOpcodeClass(D3D10_SB_OPCODE_TYPE OpCode)
{
    AssertAndAssume(IsOpCodeValid(OpCode));
    return g_InstructionInfo[OpCode].m_OpClass;
}

LPCSTR GetOpcodeString(D3D10_SB_OPCODE_TYPE OpCode)
{
    AssertAndAssume(IsOpCodeValid(OpCode));
    return g_InstructionInfo[OpCode].m_Name;
}

CInstructionInfo g_InstructionInfo[D3D10_SB_NUM_OPCODES];

void InitInstructionInfo()
{
#define SET(OpCode, NumDstOperands, NumSrcOperands, Name, OpClass) \
    (g_InstructionInfo[OpCode].Set(NumDstOperands, NumSrcOperands, Name, OpClass))

    SET (D3D10_SB_OPCODE_ADD,        1, 2,  "add", D3D11_SB_FLOAT_OP);
    SET (D3D10_SB_OPCODE_AND,        1, 2,  "and", D3D11_SB_BIT_OP);
    SET (D3D10_SB_OPCODE_BREAK,      0, 0,  "break", D3D11_SB_FLOW_OP);
    SET (D3D10_SB_OPCODE_BREAKC,     0, 1,  "breakc", D3D11_SB_FLOW_OP);
    SET (D3D10_SB_OPCODE_CALL,       0, 1,  "call", D3D11_SB_FLOW_OP);
    SET (D3D10_SB_OPCODE_CALLC,      0, 2,  "callc", D3D11_SB_FLOW_OP);
    SET (D3D10_SB_OPCODE_CONTINUE,   0, 0,  "continue", D3D11_SB_FLOW_OP);
    SET (D3D10_SB_OPCODE_CONTINUEC,  0, 1,  "continuec", D3D11_SB_FLOW_OP);
    SET (D3D10_SB_OPCODE_CASE,       0, 1,  "case", D3D11_SB_FLOW_OP);
    SET (D3D10_SB_OPCODE_CUT,        0, 0,  "cut", D3D11_SB_FLOW_OP);
    SET (D3D10_SB_OPCODE_CUSTOMDATA, 0, 0,  "customdata", D3D11_SB_FLOW_OP);
    SET (D3D10_SB_OPCODE_DEFAULT,    0, 0,  "default", D3D11_SB_FLOW_OP);
    SET (D3D10_SB_OPCODE_DISCARD,    0, 1,  "discard", D3D11_SB_FLOW_OP);
    SET (D3D10_SB_OPCODE_DIV,        1, 2,  "div", D3D11_SB_FLOAT_OP);
    SET (D3D10_SB_OPCODE_DP2,        1, 2,  "dp2", D3D11_SB_FLOAT_OP);
    SET (D3D10_SB_OPCODE_DP3,        1, 2,  "dp3", D3D11_SB_FLOAT_OP);
    SET (D3D10_SB_OPCODE_DP4,        1, 2,  "dp4", D3D11_SB_FLOAT_OP);
    SET (D3D10_SB_OPCODE_ELSE,       0, 0,  "else", D3D11_SB_FLOW_OP);
    SET (D3D10_SB_OPCODE_EMIT,       0, 0,  "emit", D3D11_SB_FLOW_OP);
    SET (D3D10_SB_OPCODE_EMITTHENCUT,0, 0,  "emit_then_cut", D3D11_SB_FLOW_OP);
    SET (D3D10_SB_OPCODE_ENDIF,      0, 0,  "endif", D3D11_SB_FLOW_OP);
    SET (D3D10_SB_OPCODE_ENDLOOP,    0, 0,  "endloop", D3D11_SB_FLOW_OP);
    SET (D3D10_SB_OPCODE_ENDSWITCH,  0, 0,  "endswitch", D3D11_SB_FLOW_OP);
    SET (D3D10_SB_OPCODE_EQ,         1, 2,  "eq", D3D11_SB_FLOAT_OP);
    SET (D3D10_SB_OPCODE_EXP,        1, 1,  "exp", D3D11_SB_FLOAT_OP);
    SET (D3D10_SB_OPCODE_FRC,        1, 1,  "frc", D3D11_SB_FLOAT_OP);
    SET (D3D10_SB_OPCODE_FTOI,       1, 1,  "ftoi", D3D11_SB_FLOAT_OP);
    SET (D3D10_SB_OPCODE_FTOU,       1, 1,  "ftou", D3D11_SB_FLOAT_OP);
    SET (D3D10_SB_OPCODE_GE,         1, 2,  "ge", D3D11_SB_FLOAT_OP);
    SET (D3D10_SB_OPCODE_DERIV_RTX,  1, 1,  "rtx", D3D11_SB_FLOAT_OP);
    SET (D3D10_SB_OPCODE_DERIV_RTY,  1, 1,  "rty", D3D11_SB_FLOAT_OP);
    SET (D3D10_SB_OPCODE_IADD,       1, 2,  "iadd", D3D11_SB_INT_OP);
    SET (D3D10_SB_OPCODE_IF,         0, 1,  "if", D3D11_SB_FLOW_OP);
    SET (D3D10_SB_OPCODE_IEQ,        1, 2,  "ieq", D3D11_SB_INT_OP);
    SET (D3D10_SB_OPCODE_IGE,        1, 2,  "ige", D3D11_SB_INT_OP);
    SET (D3D10_SB_OPCODE_ILT,        1, 2,  "ilt", D3D11_SB_INT_OP);
    SET (D3D10_SB_OPCODE_IMAD,       1, 3,  "imad", D3D11_SB_INT_OP);
    SET (D3D10_SB_OPCODE_IMAX,       1, 2,  "imax", D3D11_SB_INT_OP);
    SET (D3D10_SB_OPCODE_IMIN,       1, 2,  "imin", D3D11_SB_INT_OP);
    SET (D3D10_SB_OPCODE_IMUL,       2, 2,  "imul", D3D11_SB_INT_OP);
    SET (D3D10_SB_OPCODE_INE,        1, 2,  "ine", D3D11_SB_INT_OP);
    SET (D3D10_SB_OPCODE_INEG,       1, 1,  "ineg", D3D11_SB_INT_OP);
    SET (D3D10_SB_OPCODE_ISHL,       1, 2,  "ishl", D3D11_SB_INT_OP);
    SET (D3D10_SB_OPCODE_ISHR,       1, 2,  "ishr", D3D11_SB_INT_OP);
    SET (D3D10_SB_OPCODE_ITOF,       1, 1,  "itof", D3D11_SB_INT_OP);
    SET (D3D10_SB_OPCODE_LABEL,      0, 1,  "label", D3D11_SB_FLOW_OP);
    SET (D3D10_SB_OPCODE_LD,         1, 2,  "ld", D3D11_SB_TEX_OP);
    SET (D3D10_SB_OPCODE_LD_MS,      1, 3,  "ldms", D3D11_SB_TEX_OP);
    SET (D3D10_SB_OPCODE_LOG,        1, 1,  "log", D3D11_SB_FLOAT_OP);
    SET (D3D10_SB_OPCODE_LOOP,       0, 0,  "loop", D3D11_SB_FLOW_OP);
    SET (D3D10_SB_OPCODE_LT,         1, 2,  "lt", D3D11_SB_FLOAT_OP);
    SET (D3D10_SB_OPCODE_MAD,        1, 3,  "mad", D3D11_SB_FLOAT_OP);
    SET (D3D10_SB_OPCODE_MAX,        1, 2,  "max", D3D11_SB_FLOAT_OP);
    SET (D3D10_SB_OPCODE_MIN,        1, 2,  "min", D3D11_SB_FLOAT_OP);
    SET (D3D10_SB_OPCODE_MOV,        1, 1,  "mov", D3D11_SB_FLOAT_OP);
    SET (D3D10_SB_OPCODE_MOVC,       1, 3,  "movc", D3D11_SB_FLOAT_OP);
    SET (D3D10_SB_OPCODE_MUL,        1, 2,  "mul", D3D11_SB_FLOAT_OP);
    SET (D3D10_SB_OPCODE_NE,         1, 2,  "ne", D3D11_SB_FLOAT_OP);
    SET (D3D10_SB_OPCODE_NOP,        0, 0,  "nop", D3D11_SB_FLOW_OP);
    SET (D3D10_SB_OPCODE_NOT,        1, 1,  "not", D3D11_SB_BIT_OP);
    SET (D3D10_SB_OPCODE_OR,         1, 2,  "or", D3D11_SB_BIT_OP);
    SET (D3D10_SB_OPCODE_RESINFO,    1, 2,  "resinfo", D3D11_SB_FLOW_OP);
    SET (D3D10_SB_OPCODE_RET,        0, 0,  "ret", D3D11_SB_FLOW_OP);
    SET (D3D10_SB_OPCODE_RETC,       0, 1,  "retc", D3D11_SB_FLOW_OP);
    SET (D3D10_SB_OPCODE_ROUND_NE,   1, 1,  "round_ne", D3D11_SB_FLOAT_OP);
    SET (D3D10_SB_OPCODE_ROUND_NI,   1, 1,  "round_ni", D3D11_SB_FLOAT_OP);
    SET (D3D10_SB_OPCODE_ROUND_PI,   1, 1,  "round_pi", D3D11_SB_FLOAT_OP);
    SET (D3D10_SB_OPCODE_ROUND_Z,    1, 1,  "round_z", D3D11_SB_FLOAT_OP);
    SET (D3D10_SB_OPCODE_RSQ,        1, 1,  "rsq", D3D11_SB_FLOAT_OP);
    SET (D3D10_SB_OPCODE_SAMPLE,     1, 3,  "sample", D3D11_SB_TEX_OP);
    SET (D3D10_SB_OPCODE_SAMPLE_B,   1, 4,  "sample_b", D3D11_SB_TEX_OP);
    SET (D3D10_SB_OPCODE_SAMPLE_L,   1, 4,  "sample_l", D3D11_SB_TEX_OP);
    SET (D3D10_SB_OPCODE_SAMPLE_D,   1, 5,  "sample_d", D3D11_SB_TEX_OP);
    SET (D3D10_SB_OPCODE_SAMPLE_C,   1, 4,  "sample_c", D3D11_SB_TEX_OP);
    SET (D3D10_SB_OPCODE_SAMPLE_C_LZ,1, 4,  "sample_c_lz", D3D11_SB_TEX_OP);
    SET (D3D10_SB_OPCODE_SQRT,       1, 1,  "sqrt", D3D11_SB_FLOAT_OP);
    SET (D3D10_SB_OPCODE_SWITCH,     0, 1,  "switch", D3D11_SB_FLOW_OP);
    SET (D3D10_SB_OPCODE_SINCOS,     1, 2,  "sincos", D3D11_SB_FLOAT_OP);
    SET (D3D10_SB_OPCODE_UDIV,       2, 2,  "udiv", D3D11_SB_UINT_OP);
    SET (D3D10_SB_OPCODE_ULT,        1, 2,  "ult", D3D11_SB_UINT_OP);
    SET (D3D10_SB_OPCODE_UGE,        1, 2,  "uge", D3D11_SB_UINT_OP);
    SET (D3D10_SB_OPCODE_UMAX,       1, 2,  "umax", D3D11_SB_UINT_OP);
    SET (D3D10_SB_OPCODE_UMIN,       1, 2,  "umin", D3D11_SB_UINT_OP);
    SET (D3D10_SB_OPCODE_UMUL,       2, 2,  "umul", D3D11_SB_UINT_OP);
    SET (D3D10_SB_OPCODE_UMAD,       1, 3,  "umad", D3D11_SB_UINT_OP);
    SET (D3D10_SB_OPCODE_USHR,       1, 2,  "ushr", D3D11_SB_UINT_OP);
    SET (D3D10_SB_OPCODE_UTOF,       1, 1,  "utof", D3D11_SB_UINT_OP);
    SET (D3D10_SB_OPCODE_XOR,        1, 2,  "xor", D3D11_SB_BIT_OP);
    SET (D3D10_SB_OPCODE_RESERVED0,  0, 0,  "jmp", D3D11_SB_FLOW_OP);
    SET (D3D10_SB_OPCODE_DCL_INPUT,                          0, 1, "dcl_input", D3D11_SB_DCL_OP);
    SET (D3D10_SB_OPCODE_DCL_OUTPUT,                         0, 1, "dcl_output", D3D11_SB_DCL_OP);
    SET (D3D10_SB_OPCODE_DCL_INPUT_SGV,                      0, 1, "dcl_input_sgv", D3D11_SB_DCL_OP);
    SET (D3D10_SB_OPCODE_DCL_INPUT_PS_SGV,                   0, 1, "dcl_input_sgv", D3D11_SB_DCL_OP);
    SET (D3D10_SB_OPCODE_DCL_GS_INPUT_PRIMITIVE,             0, 0, "dcl_inputprimitive", D3D11_SB_DCL_OP);
    SET (D3D10_SB_OPCODE_DCL_GS_OUTPUT_PRIMITIVE_TOPOLOGY,   0, 0, "dcl_outputtopology", D3D11_SB_DCL_OP);
    SET (D3D10_SB_OPCODE_DCL_MAX_OUTPUT_VERTEX_COUNT,        0, 0, "dcl_maxout", D3D11_SB_DCL_OP);
    SET (D3D10_SB_OPCODE_DCL_INPUT_PS,                       0, 1, "dcl_input", D3D11_SB_DCL_OP);
    SET (D3D10_SB_OPCODE_DCL_CONSTANT_BUFFER,                0, 1, "dcl_constantbuffer", D3D11_SB_DCL_OP);
    SET (D3D10_SB_OPCODE_DCL_SAMPLER,                        0, 1, "dcl_sampler", D3D11_SB_DCL_OP);
    SET (D3D10_SB_OPCODE_DCL_RESOURCE,                       0, 1, "dcl_resource", D3D11_SB_DCL_OP);
    SET (D3D10_SB_OPCODE_DCL_INPUT_SIV,                      0, 1, "dcl_input_siv", D3D11_SB_DCL_OP);
    SET (D3D10_SB_OPCODE_DCL_INPUT_PS_SIV,                   0, 1, "dcl_input_siv", D3D11_SB_DCL_OP);
    SET (D3D10_SB_OPCODE_DCL_OUTPUT_SIV,                     0, 1, "dcl_output_siv", D3D11_SB_DCL_OP);
    SET (D3D10_SB_OPCODE_DCL_OUTPUT_SGV,                     0, 1, "dcl_output_sgv", D3D11_SB_DCL_OP);
    SET (D3D10_SB_OPCODE_DCL_TEMPS,                          0, 0, "dcl_temps", D3D11_SB_DCL_OP);
    SET (D3D10_SB_OPCODE_DCL_INDEXABLE_TEMP,                 0, 0, "dcl_indexableTemp", D3D11_SB_DCL_OP);
    SET (D3D10_SB_OPCODE_DCL_INDEX_RANGE,                    0, 1, "dcl_indexrange", D3D11_SB_DCL_OP);
    SET (D3D10_SB_OPCODE_DCL_GLOBAL_FLAGS,                   0, 0, "dcl_globalFlags", D3D11_SB_DCL_OP);

    SET (D3D10_1_SB_OPCODE_SAMPLE_INFO,                      1, 1, "sampleinfo", D3D11_SB_TEX_OP);
    SET (D3D10_1_SB_OPCODE_SAMPLE_POS,                       1, 2, "samplepos", D3D11_SB_TEX_OP);
    SET (D3D10_1_SB_OPCODE_GATHER4,                          1, 3, "gather4", D3D11_SB_TEX_OP);
    SET (D3D10_1_SB_OPCODE_LOD,                              1, 3, "lod", D3D11_SB_TEX_OP);

    //
    // DX11 opcodes
    //
    SET (D3D11_SB_OPCODE_HS_DECLS,                          0, 0, "hs_decls", D3D11_SB_DCL_OP);
    SET (D3D11_SB_OPCODE_HS_CONTROL_POINT_PHASE,            0, 0, "hs_control_point_phase", D3D11_SB_DCL_OP);
    SET (D3D11_SB_OPCODE_HS_FORK_PHASE,                     0, 0, "hs_fork_phase", D3D11_SB_DCL_OP);
    SET (D3D11_SB_OPCODE_HS_JOIN_PHASE,                     0, 0, "hs_join_phase", D3D11_SB_DCL_OP);

    SET (D3D11_SB_OPCODE_INTERFACE_CALL,                    0, 1, "fcall", D3D11_SB_FLOW_OP);

    SET (D3D11_SB_OPCODE_BUFINFO,                           1, 1, "bufinfo", D3D11_SB_TEX_OP);
    SET (D3D11_SB_OPCODE_DERIV_RTX_COARSE,                  1, 1, "deriv_rtx_coarse", D3D11_SB_FLOAT_OP);
    SET (D3D11_SB_OPCODE_DERIV_RTX_FINE,                    1, 1, "deriv_rtx_fine", D3D11_SB_FLOAT_OP);
    SET (D3D11_SB_OPCODE_DERIV_RTY_COARSE,                  1, 1, "deriv_rty_coarse", D3D11_SB_FLOAT_OP);
    SET (D3D11_SB_OPCODE_DERIV_RTY_FINE,                    1, 1, "deriv_rty_fine", D3D11_SB_FLOAT_OP);
    SET (D3D11_SB_OPCODE_GATHER4_C,                         1, 4, "gather4_c", D3D11_SB_TEX_OP);
    SET (D3D11_SB_OPCODE_GATHER4_PO,                        1, 4, "gather4_po", D3D11_SB_TEX_OP);
    SET (D3D11_SB_OPCODE_GATHER4_PO_C,                      1, 5, "gather4_po_c", D3D11_SB_TEX_OP);
    SET (D3D11_SB_OPCODE_RCP,                               1, 1, "rcp", D3D11_SB_FLOAT_OP);
    SET (D3D11_SB_OPCODE_F32TOF16,                          1, 1, "f32tof16", D3D11_SB_FLOAT_OP);
    SET (D3D11_SB_OPCODE_F16TOF32,                          1, 1, "f16tof32", D3D11_SB_FLOAT_OP);
    SET (D3D11_SB_OPCODE_UADDC,                             2, 2, "uaddc", D3D11_SB_UINT_OP);
    SET (D3D11_SB_OPCODE_USUBB,                             2, 2, "usubb", D3D11_SB_UINT_OP);
    SET (D3D11_SB_OPCODE_COUNTBITS,                         1, 1, "countbits", D3D11_SB_UINT_OP);
    SET (D3D11_SB_OPCODE_FIRSTBIT_HI,                       1, 1, "firstbit_hi", D3D11_SB_UINT_OP);
    SET (D3D11_SB_OPCODE_FIRSTBIT_LO,                       1, 1, "firstbit_lo", D3D11_SB_UINT_OP);
    SET (D3D11_SB_OPCODE_FIRSTBIT_SHI,                      1, 1, "firstbit_shi", D3D11_SB_INT_OP);
    SET (D3D11_SB_OPCODE_UBFE,                              1, 3, "ubfe", D3D11_SB_UINT_OP);
    SET (D3D11_SB_OPCODE_IBFE,                              1, 3, "ibfe", D3D11_SB_INT_OP);
    SET (D3D11_SB_OPCODE_BFI,                               1, 4, "bfi", D3D11_SB_UINT_OP);
    SET (D3D11_SB_OPCODE_BFREV,                             1, 1, "bfrev", D3D11_SB_UINT_OP);
    SET (D3D11_SB_OPCODE_SWAPC,                             2, 3, "swapc", D3D11_SB_UINT_OP);

    SET (D3D11_SB_OPCODE_DCL_STREAM,                        0, 1, "dcl_stream", D3D11_SB_DCL_OP);
    SET (D3D11_SB_OPCODE_DCL_FUNCTION_BODY,                 0, 0, "dcl_function_body", D3D11_SB_DCL_OP);
    SET (D3D11_SB_OPCODE_DCL_FUNCTION_TABLE,                0, 0, "dcl_function_table ", D3D11_SB_DCL_OP);
    SET (D3D11_SB_OPCODE_DCL_INTERFACE,                     0, 0, "dcl_interface", D3D11_SB_DCL_OP);

    SET (D3D11_SB_OPCODE_DCL_INPUT_CONTROL_POINT_COUNT,     0, 1, "dcl_input_control_point_count", D3D11_SB_DCL_OP);
    SET (D3D11_SB_OPCODE_DCL_OUTPUT_CONTROL_POINT_COUNT,    0, 1, "dcl_output_control_point_count", D3D11_SB_DCL_OP);
    SET (D3D11_SB_OPCODE_DCL_TESS_DOMAIN,                   0, 1, "dcl_tessellator_domain", D3D11_SB_DCL_OP);
    SET (D3D11_SB_OPCODE_DCL_TESS_PARTITIONING,             0, 1, "dcl_tessellator_partitioning", D3D11_SB_DCL_OP);
    SET (D3D11_SB_OPCODE_DCL_TESS_OUTPUT_PRIMITIVE,         0, 1, "dcl_tessellator_output_primitive", D3D11_SB_DCL_OP);
    SET (D3D11_SB_OPCODE_DCL_HS_MAX_TESSFACTOR,             0, 1, "dcl_hs_max_tessfactor", D3D11_SB_DCL_OP);
    SET (D3D11_SB_OPCODE_DCL_HS_FORK_PHASE_INSTANCE_COUNT,  0, 1, "dcl_hs_fork_phase_instance_count", D3D11_SB_DCL_OP);
    SET (D3D11_SB_OPCODE_DCL_HS_JOIN_PHASE_INSTANCE_COUNT,  0, 1, "dcl_hs_join_phase_instance_count", D3D11_SB_DCL_OP);

    SET (D3D11_SB_OPCODE_DCL_THREAD_GROUP,                          0, 3, "dcl_thread_group", D3D11_SB_DCL_OP);
    SET (D3D11_SB_OPCODE_DCL_UNORDERED_ACCESS_VIEW_TYPED,           0, 3, "dcl_uav_typed", D3D11_SB_DCL_OP);
    SET (D3D11_SB_OPCODE_DCL_UNORDERED_ACCESS_VIEW_RAW,             0, 1, "dcl_uav_raw", D3D11_SB_DCL_OP);
    SET (D3D11_SB_OPCODE_DCL_UNORDERED_ACCESS_VIEW_STRUCTURED,      0, 2, "dcl_uav_structured", D3D11_SB_DCL_OP);
    SET (D3D11_SB_OPCODE_DCL_THREAD_GROUP_SHARED_MEMORY_RAW,        0, 2, "dcl_tgsm_raw", D3D11_SB_DCL_OP);
    SET (D3D11_SB_OPCODE_DCL_THREAD_GROUP_SHARED_MEMORY_STRUCTURED, 0, 3, "dcl_tgsm_structured", D3D11_SB_DCL_OP);
    SET (D3D11_SB_OPCODE_DCL_RESOURCE_RAW,                          0, 1, "dcl_resource_raw", D3D11_SB_DCL_OP);
    SET (D3D11_SB_OPCODE_DCL_RESOURCE_STRUCTURED,                   0, 2, "dcl_resource_structured", D3D11_SB_DCL_OP);

    SET (D3D11_SB_OPCODE_LD_UAV_TYPED,                              1, 2, "ld_uav_typed", D3D11_SB_TEX_OP);
    SET (D3D11_SB_OPCODE_STORE_UAV_TYPED,                           1, 2, "store_uav_typed", D3D11_SB_TEX_OP);
    SET (D3D11_SB_OPCODE_LD_RAW,                                    1, 2, "ld_raw", D3D11_SB_TEX_OP);
    SET (D3D11_SB_OPCODE_STORE_RAW,                                 1, 2, "store_raw", D3D11_SB_TEX_OP);
    SET (D3D11_SB_OPCODE_LD_STRUCTURED,                             1, 3, "ld_structured", D3D11_SB_TEX_OP);
    SET (D3D11_SB_OPCODE_STORE_STRUCTURED,                          1, 3, "store_structured ", D3D11_SB_TEX_OP);

    SET (D3D11_SB_OPCODE_ATOMIC_AND,                                1, 2, "atomic_and", D3D11_SB_UINT_OP);
    SET (D3D11_SB_OPCODE_ATOMIC_OR,                                 1, 2, "atomic_or", D3D11_SB_UINT_OP);
    SET (D3D11_SB_OPCODE_ATOMIC_XOR,                                1, 2, "atomic_xor", D3D11_SB_UINT_OP);
    SET (D3D11_SB_OPCODE_ATOMIC_CMP_STORE,                          1, 3, "atomic_cmp_store", D3D11_SB_UINT_OP);
    SET (D3D11_SB_OPCODE_ATOMIC_IADD,                               1, 2, "atomic_iadd", D3D11_SB_INT_OP);
    SET (D3D11_SB_OPCODE_ATOMIC_IMAX,                               1, 2, "atomic_imax", D3D11_SB_INT_OP);
    SET (D3D11_SB_OPCODE_ATOMIC_IMIN,                               1, 2, "atomic_imin", D3D11_SB_INT_OP);
    SET (D3D11_SB_OPCODE_ATOMIC_UMAX,                               1, 2, "atomic_umax", D3D11_SB_UINT_OP);
    SET (D3D11_SB_OPCODE_ATOMIC_UMIN,                               1, 2, "atomic_umin", D3D11_SB_UINT_OP);
    SET (D3D11_SB_OPCODE_IMM_ATOMIC_ALLOC,                          1, 1, "imm_atomic_alloc", D3D11_SB_UINT_OP);
    SET (D3D11_SB_OPCODE_IMM_ATOMIC_CONSUME,                        1, 1, "imm_atomic_consume", D3D11_SB_UINT_OP);
    SET (D3D11_SB_OPCODE_IMM_ATOMIC_IADD,                           2, 2, "imm_atomic_iadd", D3D11_SB_INT_OP);
    SET (D3D11_SB_OPCODE_IMM_ATOMIC_AND,                            2, 2, "imm_atomic_and", D3D11_SB_UINT_OP);
    SET (D3D11_SB_OPCODE_IMM_ATOMIC_OR,                             2, 2, "imm_atomic_or", D3D11_SB_UINT_OP);
    SET (D3D11_SB_OPCODE_IMM_ATOMIC_XOR,                            2, 2, "imm_atomic_xor", D3D11_SB_UINT_OP);
    SET (D3D11_SB_OPCODE_IMM_ATOMIC_EXCH,                           2, 2, "imm_atomic_exch", D3D11_SB_UINT_OP);
    SET (D3D11_SB_OPCODE_IMM_ATOMIC_CMP_EXCH,                       2, 3, "imm_atomic_cmp_exch", D3D11_SB_UINT_OP);
    SET (D3D11_SB_OPCODE_IMM_ATOMIC_IMAX,                           2, 2, "imm_atomic_imax", D3D11_SB_INT_OP);
    SET (D3D11_SB_OPCODE_IMM_ATOMIC_IMIN,                           2, 2, "imm_atomic_imin", D3D11_SB_INT_OP);
    SET (D3D11_SB_OPCODE_IMM_ATOMIC_UMAX,                           2, 2, "imm_atomic_umax", D3D11_SB_UINT_OP);
    SET (D3D11_SB_OPCODE_IMM_ATOMIC_UMIN,                           2, 2, "imm_atomic_umin", D3D11_SB_UINT_OP);
    SET (D3D11_SB_OPCODE_SYNC,                                      0, 0, "sync", D3D11_SB_FLOW_OP);

    SET (D3D11_SB_OPCODE_DADD,                                      1, 2, "dadd", D3D11_SB_FLOAT_OP);
    SET (D3D11_SB_OPCODE_DMAX,                                      1, 2, "dmax", D3D11_SB_FLOAT_OP);
    SET (D3D11_SB_OPCODE_DMIN,                                      1, 2, "dmin", D3D11_SB_FLOAT_OP);
    SET (D3D11_SB_OPCODE_DMUL,                                      1, 2, "dmul", D3D11_SB_FLOAT_OP);
    SET (D3D11_SB_OPCODE_DEQ,                                       1, 2, "deq", D3D11_SB_FLOAT_OP);
    SET (D3D11_SB_OPCODE_DGE,                                       1, 2, "dge", D3D11_SB_FLOAT_OP);
    SET (D3D11_SB_OPCODE_DLT,                                       1, 2, "dlt", D3D11_SB_FLOAT_OP);
    SET (D3D11_SB_OPCODE_DNE,                                       1, 2, "dne", D3D11_SB_FLOAT_OP);
    SET (D3D11_SB_OPCODE_DMOV,                                      1, 1, "dmov", D3D11_SB_FLOAT_OP);
    SET (D3D11_SB_OPCODE_DMOVC,                                     1, 3, "dmovc", D3D11_SB_FLOAT_OP);
    SET (D3D11_SB_OPCODE_DTOF,                                      1, 1, "dtof", D3D11_SB_FLOAT_OP);
    SET (D3D11_SB_OPCODE_FTOD,                                      1, 1, "ftod", D3D11_SB_FLOAT_OP);

    SET (D3D11_1_SB_OPCODE_DDIV,                                    1, 2, "ddiv", D3D11_SB_FLOAT_OP);
    SET (D3D11_1_SB_OPCODE_DFMA,                                    1, 3, "dfma", D3D11_SB_FLOAT_OP);
    SET (D3D11_1_SB_OPCODE_DRCP,                                    1, 1, "drcp", D3D11_SB_FLOAT_OP);

    SET (D3D11_1_SB_OPCODE_MSAD,                                    1, 3, "msad", D3D11_SB_UINT_OP);

    SET (D3D11_1_SB_OPCODE_DTOI,                                    1, 1, "dtoi", D3D11_SB_FLOAT_OP);
    SET (D3D11_1_SB_OPCODE_DTOU,                                    1, 1, "dtou", D3D11_SB_FLOAT_OP);
    SET (D3D11_1_SB_OPCODE_ITOD,                                    1, 1, "itod", D3D11_SB_FLOAT_OP);
    SET (D3D11_1_SB_OPCODE_UTOD,                                    1, 1, "utod", D3D11_SB_FLOAT_OP);

    SET (D3D11_SB_OPCODE_EVAL_SNAPPED,                              1, 2, "eval_snapped", D3D11_SB_TEX_OP);
    SET (D3D11_SB_OPCODE_EVAL_SAMPLE_INDEX,                         1, 2, "eval_sample_index", D3D11_SB_TEX_OP);
    SET (D3D11_SB_OPCODE_EVAL_CENTROID,                             1, 1, "eval_centroid", D3D11_SB_TEX_OP);

    SET (D3D11_SB_OPCODE_DCL_GS_INSTANCE_COUNT,                     0, 0, "dcl_input", D3D11_SB_DCL_OP);
    SET (D3D11_SB_OPCODE_EMIT_STREAM,                               0, 1, "emit_stream", D3D11_SB_FLOW_OP);
    SET (D3D11_SB_OPCODE_CUT_STREAM,                                0, 1, "cut_stream", D3D11_SB_FLOW_OP);
    SET (D3D11_SB_OPCODE_EMITTHENCUT_STREAM,                        0, 1, "emitThenCut_stream", D3D11_SB_FLOW_OP);

    SET (D3D11_SB_OPCODE_ABORT,                                     0, 0, "abort", D3D11_SB_FLOW_OP); //TODO: correct this
    SET (D3D11_SB_OPCODE_DEBUG_BREAK,                               0, 0, "debug_break", D3D11_SB_FLOW_OP); //TODO: correct this

    SET (D3DWDDM1_3_SB_OPCODE_GATHER4_FEEDBACK,                     2, 3, "gather4_fb", D3D11_SB_TEX_OP);
    SET (D3DWDDM1_3_SB_OPCODE_GATHER4_C_FEEDBACK,                   2, 4, "gather4_c_fb", D3D11_SB_TEX_OP);
    SET (D3DWDDM1_3_SB_OPCODE_GATHER4_PO_FEEDBACK,                  2, 4, "gather4_po_fb", D3D11_SB_TEX_OP);
    SET (D3DWDDM1_3_SB_OPCODE_GATHER4_PO_C_FEEDBACK,                2, 5, "gather4_po_c_fb", D3D11_SB_TEX_OP);
    SET (D3DWDDM1_3_SB_OPCODE_LD_FEEDBACK,                          2, 2, "ld_fb", D3D11_SB_TEX_OP);
    SET (D3DWDDM1_3_SB_OPCODE_LD_MS_FEEDBACK,                       2, 3, "ldms_fb", D3D11_SB_TEX_OP);
    SET (D3DWDDM1_3_SB_OPCODE_LD_UAV_TYPED_FEEDBACK,                2, 2, "ld_uav_typed_fb", D3D11_SB_TEX_OP);
    SET (D3DWDDM1_3_SB_OPCODE_LD_RAW_FEEDBACK,                      2, 2, "ld_raw_fb", D3D11_SB_TEX_OP);
    SET (D3DWDDM1_3_SB_OPCODE_LD_STRUCTURED_FEEDBACK,               2, 3, "ld_structured_fb", D3D11_SB_TEX_OP);
    SET (D3DWDDM1_3_SB_OPCODE_SAMPLE_L_FEEDBACK,                    2, 4, "sample_l_fb", D3D11_SB_TEX_OP);
    SET (D3DWDDM1_3_SB_OPCODE_SAMPLE_C_LZ_FEEDBACK,                 2, 4, "sample_c_lz_fb", D3D11_SB_TEX_OP);

    SET (D3DWDDM1_3_SB_OPCODE_SAMPLE_CLAMP_FEEDBACK,                2, 4, "sample_clamp_fb", D3D11_SB_TEX_OP);
    SET (D3DWDDM1_3_SB_OPCODE_SAMPLE_B_CLAMP_FEEDBACK,              2, 5, "sample_clamp_b_fb", D3D11_SB_TEX_OP);
    SET (D3DWDDM1_3_SB_OPCODE_SAMPLE_D_CLAMP_FEEDBACK,              2, 6, "sample_clamp_d_fb", D3D11_SB_TEX_OP);
    SET (D3DWDDM1_3_SB_OPCODE_SAMPLE_C_CLAMP_FEEDBACK,              2, 5, "sample_clamp_c_fb", D3D11_SB_TEX_OP);

    SET (D3DWDDM1_3_SB_OPCODE_CHECK_ACCESS_FULLY_MAPPED,            1, 1, "checkAccess", D3D11_SB_UINT_OP);
}

//*****************************************************************************
//
//  CShaderCodeParser
//
//*****************************************************************************

void CShaderCodeParser::SetShader(CONST CShaderToken* pBuffer)
{
    m_NumParsedInstructions = 0;

    m_pShaderCode = (CShaderToken*)pBuffer;
    m_pShaderEndToken = (CShaderToken*)pBuffer + pBuffer[1];
    // First OpCode token
    m_pCurrentToken = (CShaderToken*)&pBuffer[2];
}

ParserPositionToken CShaderCodeParser::GetCurrentToken()
{
    ParserPositionToken Ret;
    Ret.Inst      = m_pCurrentToken;
    Ret.InstCount = m_NumParsedInstructions;
    return Ret;
}

UINT CShaderCodeParser::CurrentTokenOffsetInBytes()
{
    return UINT(m_pCurrentToken - m_pShaderCode) * sizeof(CShaderToken);
}

void CShaderCodeParser::SetCurrentToken(ParserPositionToken In)
{
    m_pCurrentToken         = In.Inst;
    m_NumParsedInstructions = In.InstCount;
}

D3D10_SB_TOKENIZED_PROGRAM_TYPE CShaderCodeParser::ShaderType()
{
    return (D3D10_SB_TOKENIZED_PROGRAM_TYPE)DECODE_D3D10_SB_TOKENIZED_PROGRAM_TYPE(*m_pShaderCode);
}

UINT CShaderCodeParser::ShaderLengthInTokens()
{
    return m_pShaderCode[1];
}

UINT CShaderCodeParser::ShaderMinorVersion()
{
    return DECODE_D3D10_SB_TOKENIZED_PROGRAM_MINOR_VERSION(m_pShaderCode[0]);
}

UINT CShaderCodeParser::ShaderMajorVersion()
{
    return DECODE_D3D10_SB_TOKENIZED_PROGRAM_MAJOR_VERSION(m_pShaderCode[0]);
}

void CShaderCodeParser::ParseResourceDcl(CInstruction* pInstruction, CResourceSpaceDecl* pSpaceDcl)
{
    // if this is a 3D dimension then it defines a 5.1 register space, otherwise it's a standard 5.0 resource dcl
    if (pInstruction->m_Operands[0].m_IndexDimension == D3D10_SB_OPERAND_INDEX_3D)
    {
        assert(ShaderMajorVersion() > 5 || (ShaderMajorVersion() == 5 && ShaderMinorVersion() >= 1));
        pSpaceDcl->SetIdx = pInstruction->Operand(0).RegIndex(0);
        pSpaceDcl->MinShaderRegister = pInstruction->Operand(0).RegIndex(1);
        pSpaceDcl->MaxShaderRegister = pInstruction->Operand(0).RegIndex(2);
        pSpaceDcl->Space = (UINT) (*m_pCurrentToken++);
    }
    else
    {
        assert(ShaderMajorVersion() < 5 || (ShaderMajorVersion() == 5 && ShaderMinorVersion() < 1));
        pSpaceDcl->SetIdx = pInstruction->Operand(0).RegIndex(0);
        pSpaceDcl->MinShaderRegister = pInstruction->Operand(0).RegIndex(0);
        pSpaceDcl->MaxShaderRegister = pInstruction->Operand(0).RegIndex(0);
        pSpaceDcl->Space = 0;
    }
}

__checkReturn HRESULT CShaderCodeParser::ParseOperand(COperandBase* pOperand)
{
    HRESULT hr = S_OK;

    CShaderToken Token = *m_pCurrentToken++;

    pOperand->m_Type = DECODE_D3D10_SB_OPERAND_TYPE(Token);
    pOperand->m_NumComponents = DECODE_D3D10_SB_OPERAND_NUM_COMPONENTS(Token);
    pOperand->m_bExtendedOperand = DECODE_IS_D3D10_SB_OPERAND_EXTENDED(Token);

    UINT NumComponents = 0;
    switch (pOperand->m_NumComponents)
    {
    case D3D10_SB_OPERAND_1_COMPONENT:   NumComponents = 1; break;
    case D3D10_SB_OPERAND_4_COMPONENT:   NumComponents = 4; break;
    }

    switch (pOperand->m_Type)
    {
    case D3D10_SB_OPERAND_TYPE_IMMEDIATE32:
    case D3D10_SB_OPERAND_TYPE_IMMEDIATE64:
        pOperand->m_IndexDimension = DECODE_D3D10_SB_OPERAND_INDEX_DIMENSION(Token);
        break;
    default:
        {
            if (pOperand->m_NumComponents == D3D10_SB_OPERAND_4_COMPONENT)
            {
                // Component selection mode
                pOperand->m_ComponentSelection = DECODE_D3D10_SB_OPERAND_4_COMPONENT_SELECTION_MODE(Token);
                switch(pOperand->m_ComponentSelection)
                {
                case D3D10_SB_OPERAND_4_COMPONENT_MASK_MODE:
                    pOperand->m_WriteMask = DECODE_D3D10_SB_OPERAND_4_COMPONENT_MASK(Token);
                    break;
                case D3D10_SB_OPERAND_4_COMPONENT_SWIZZLE_MODE:
                    pOperand->m_Swizzle[0] = (BYTE)DECODE_D3D10_SB_OPERAND_4_COMPONENT_SWIZZLE_SOURCE(Token, 0);
                    pOperand->m_Swizzle[1] = (BYTE)DECODE_D3D10_SB_OPERAND_4_COMPONENT_SWIZZLE_SOURCE(Token, 1);
                    pOperand->m_Swizzle[2] = (BYTE)DECODE_D3D10_SB_OPERAND_4_COMPONENT_SWIZZLE_SOURCE(Token, 2);
                    pOperand->m_Swizzle[3] = (BYTE)DECODE_D3D10_SB_OPERAND_4_COMPONENT_SWIZZLE_SOURCE(Token, 3);
                    break;
                case D3D10_SB_OPERAND_4_COMPONENT_SELECT_1_MODE:
                {
                    D3D10_SB_4_COMPONENT_NAME Component = DECODE_D3D10_SB_OPERAND_4_COMPONENT_SELECT_1(Token);
                    pOperand->m_Swizzle[0] = (BYTE)Component;
                    pOperand->m_Swizzle[1] = (BYTE)Component;
                    pOperand->m_Swizzle[2] = (BYTE)Component;
                    pOperand->m_Swizzle[3] = (BYTE)Component;
                    pOperand->m_ComponentName = Component;
                    break;
                }
                default:
                    hr = E_OUTOFMEMORY; // Can't return E_INVALIDARG because that is not allowed by the runtime (causes device removed);
                    goto Cleanup;
                }
            }
            pOperand->m_IndexDimension = DECODE_D3D10_SB_OPERAND_INDEX_DIMENSION(Token);
            if (pOperand->m_IndexDimension != D3D10_SB_OPERAND_INDEX_0D)
            {
                UINT NumDimensions = pOperand->m_IndexDimension;
                // Index representation
                for (UINT i=0; i < NumDimensions; i++)
                {
                    pOperand->m_IndexType[i] = DECODE_D3D10_SB_OPERAND_INDEX_REPRESENTATION(i, Token);
                }
            }
            break;
        }
    }

    // Extended operand
    if (pOperand->m_bExtendedOperand)
    {
        Token = *m_pCurrentToken++;
        pOperand->m_ExtendedOperandType = DECODE_D3D10_SB_EXTENDED_OPERAND_TYPE(Token);
        if (pOperand->m_ExtendedOperandType == D3D10_SB_EXTENDED_OPERAND_MODIFIER)
        {
            pOperand->m_Modifier = DECODE_D3D10_SB_OPERAND_MODIFIER(Token);
        }
        pOperand->m_MinPrecision = DECODE_D3D11_SB_OPERAND_MIN_PRECISION(Token);
    }

    if (pOperand->m_Type == D3D10_SB_OPERAND_TYPE_IMMEDIATE32 ||
            pOperand->m_Type == D3D10_SB_OPERAND_TYPE_IMMEDIATE64)
    {
        for (UINT i=0 ; i < NumComponents; i++)
        {
            pOperand->m_Value[i] = *m_pCurrentToken++;
        }
    }

    // Operand indices
    if (pOperand->m_IndexDimension != D3D10_SB_OPERAND_INDEX_0D)
    {
        const UINT NumDimensions = pOperand->m_IndexDimension;
        // Index representation
        for (UINT i=0; i < NumDimensions; i++)
        {
            switch (pOperand->m_IndexType[i])
            {
            case D3D10_SB_OPERAND_INDEX_IMMEDIATE32:
                pOperand->m_Index[i].m_RegIndex = *m_pCurrentToken++;
                pOperand->m_Index[i].m_ComponentName = pOperand->m_ComponentName;
                pOperand->m_Index[i].m_RelRegType = pOperand->m_Type;
                break;
            case D3D10_SB_OPERAND_INDEX_IMMEDIATE64:
                pOperand->m_Index[i].m_RegIndexA[0] = *m_pCurrentToken++;
                pOperand->m_Index[i].m_RegIndexA[1] = *m_pCurrentToken++;
                pOperand->m_Index[i].m_ComponentName = pOperand->m_ComponentName;
                pOperand->m_Index[i].m_RelRegType = pOperand->m_Type;
                break;
            case D3D10_SB_OPERAND_INDEX_RELATIVE:
                {
                    COperand operand;

                    IFC (ParseOperand(&operand));

                    pOperand->m_Index[i].m_RelIndex = operand.m_Index[0].m_RegIndex;
                    pOperand->m_Index[i].m_RelIndex1 = operand.m_Index[1].m_RegIndex;
                    pOperand->m_Index[i].m_RelRegType = operand.m_Type;
                    pOperand->m_Index[i].m_IndexDimension = operand.m_IndexDimension;
                    pOperand->m_Index[i].m_ComponentName = operand.m_Index[0].m_ComponentName;
                    break;
                }
            case D3D10_SB_OPERAND_INDEX_IMMEDIATE32_PLUS_RELATIVE:
                {
                    pOperand->m_Index[i].m_RegIndex = *m_pCurrentToken++;
                    COperand operand;

                    IFC (ParseOperand(&operand));

                    pOperand->m_Index[i].m_RelIndex = operand.m_Index[0].m_RegIndex;
                    pOperand->m_Index[i].m_RelIndex1 = operand.m_Index[1].m_RegIndex;
                    pOperand->m_Index[i].m_RelRegType = operand.m_Index[0].m_RelRegType;
                    pOperand->m_Index[i].m_IndexDimension = operand.m_IndexDimension;
                    pOperand->m_Index[i].m_ComponentName = operand.m_Index[0].m_ComponentName;
                }
                break;
            default:
                hr = E_INVALIDARG;
                goto Cleanup;
            }
        }
    }

Cleanup:
    RRETURN (hr);
}

//
// Returns the opcode of the next instruction to be read. If the end of the shader has been reached, D3D10_SB_OPCODE_MAX is returned
//
D3D10_SB_OPCODE_TYPE CShaderCodeParser::PeekNextInstructionOpCode()
{
    if (EndOfShader())
    {
        return D3D10_SB_OPCODE_MAX;
    }
    else
    {
        return DECODE_D3D10_SB_OPCODE_TYPE(*m_pCurrentToken);
    }
}

__checkReturn HRESULT CShaderCodeParser::ParseInstruction(CInstruction* pInstruction)
{
    HRESULT hr = S_OK;

    pInstruction->Clear(true);
    CShaderToken* pStart = m_pCurrentToken;
    CShaderToken Token = *m_pCurrentToken++;
    pInstruction->m_OpCode = DECODE_D3D10_SB_OPCODE_TYPE(Token);
    pInstruction->m_bSaturate = DECODE_IS_D3D10_SB_INSTRUCTION_SATURATE_ENABLED(Token);
    pInstruction->m_bExtended = DECODE_IS_D3D10_SB_OPCODE_EXTENDED(Token);
    UINT InstructionLength = DECODE_D3D10_SB_TOKENIZED_INSTRUCTION_LENGTH(Token);
    pInstruction->m_NumOperands = GetNumInstructionOperands(pInstruction->m_OpCode);
    pInstruction->m_ExtendedOpCodeCount = 0;

    if(pInstruction->m_bExtended)
    {
        if(pInstruction->m_OpCode == D3D11_SB_OPCODE_DCL_INTERFACE ||
           pInstruction->m_OpCode == D3D11_SB_OPCODE_DCL_FUNCTION_TABLE)
        {
            pInstruction->m_ExtendedOpCodeCount = 1;
            CShaderToken _Token = *m_pCurrentToken++;
            // these instructions may be longer than can fit in the normal instructionlength field
            InstructionLength = (UINT)(_Token);
        }
        else
        {
            BOOL bExtended = TRUE;
            for(int i = 0; i < D3D11_SB_MAX_SIMULTANEOUS_EXTENDED_OPCODES; i++)
            {
                pInstruction->m_ExtendedOpCodeCount++;
                CShaderToken _Token = *m_pCurrentToken++;
                bExtended = DECODE_IS_D3D10_SB_OPCODE_EXTENDED(_Token);
                pInstruction->m_OpCodeEx[i] = DECODE_D3D10_SB_EXTENDED_OPCODE_TYPE(_Token);
                switch(pInstruction->m_OpCodeEx[i])
                {
                case D3D10_SB_EXTENDED_OPCODE_SAMPLE_CONTROLS:
                    {
                        pInstruction->m_TexelOffset[0] = (INT8)DECODE_IMMEDIATE_D3D10_SB_ADDRESS_OFFSET(D3D10_SB_IMMEDIATE_ADDRESS_OFFSET_U, _Token);
                        pInstruction->m_TexelOffset[1] = (INT8)DECODE_IMMEDIATE_D3D10_SB_ADDRESS_OFFSET(D3D10_SB_IMMEDIATE_ADDRESS_OFFSET_V, _Token);
                        pInstruction->m_TexelOffset[2] = (INT8)DECODE_IMMEDIATE_D3D10_SB_ADDRESS_OFFSET(D3D10_SB_IMMEDIATE_ADDRESS_OFFSET_W, _Token);
                        for(UINT j = 0; j < 3; j++)
                        {
                            if(pInstruction->m_TexelOffset[j] & 0x8)
                                pInstruction->m_TexelOffset[j] |= 0xfffffff0;
                        }
                        break;
                    }
                    break;
                case D3D11_SB_EXTENDED_OPCODE_RESOURCE_DIM:
                    {
                        pInstruction->m_ExtendedResourceDecl.Dimension = DECODE_D3D11_SB_EXTENDED_RESOURCE_DIMENSION(_Token);
                        pInstruction->m_bExtendedResourceDim = true;
                    }
                    break;
                }
                if( !bExtended )
                {
                    break;
                }
            }
        }
    }

    switch (pInstruction->m_OpCode)
    {
    case D3D10_SB_OPCODE_CUSTOMDATA:
        // not bothering to keep custom-data for now. TODO: store
        pInstruction->m_CustomData.Type = DECODE_D3D10_SB_CUSTOMDATA_CLASS(Token);
        InstructionLength = *m_pCurrentToken;
        if (*m_pCurrentToken <2)
        {
            InstructionLength = 2;
            pInstruction->m_CustomData.pData = 0;
            pInstruction->m_CustomData.DataSizeInBytes = 0;
        }
        else
        {
            pInstruction->m_CustomData.DataSizeInBytes = (*m_pCurrentToken-2)*4;
			IFCOOM((pInstruction->m_CustomData.pData = new UINT[*m_pCurrentToken - 2]));

            memcpy(pInstruction->m_CustomData.pData, m_pCurrentToken+1, (*m_pCurrentToken - 2)*4);
        }
        break;

    case D3D10_SB_OPCODE_DCL_RESOURCE:
        pInstruction->m_ResourceDecl.SRVInfo.Dimension = DECODE_D3D10_SB_RESOURCE_DIMENSION(Token);
        IFC (ParseOperand(&pInstruction->m_Operands[0]));
        pInstruction->m_ResourceDecl.SRVInfo.ReturnType[0] = DECODE_D3D10_SB_RESOURCE_RETURN_TYPE(*m_pCurrentToken, 0);
        pInstruction->m_ResourceDecl.SRVInfo.ReturnType[1] = DECODE_D3D10_SB_RESOURCE_RETURN_TYPE(*m_pCurrentToken, 1);
        pInstruction->m_ResourceDecl.SRVInfo.ReturnType[2] = DECODE_D3D10_SB_RESOURCE_RETURN_TYPE(*m_pCurrentToken, 2);
        pInstruction->m_ResourceDecl.SRVInfo.ReturnType[3] = DECODE_D3D10_SB_RESOURCE_RETURN_TYPE(*m_pCurrentToken, 3);
        pInstruction->m_ResourceDecl.SRVInfo.SampleCount = DECODE_D3D10_SB_RESOURCE_SAMPLE_COUNT(Token);
        m_pCurrentToken++;
        ParseResourceDcl(pInstruction, &pInstruction->m_ResourceDecl.Space);
        break;

    case D3D10_SB_OPCODE_DCL_SAMPLER:
        pInstruction->m_ResourceDecl.SamplerInfo.SamplerMode = DECODE_D3D10_SB_SAMPLER_MODE(Token);
        IFC (ParseOperand(&pInstruction->m_Operands[0]));
        ParseResourceDcl(pInstruction, &pInstruction->m_ResourceDecl.Space);
        break;

    case D3D10_SB_OPCODE_DCL_TEMPS:
        pInstruction->m_TempsDecl.NumTemps = (UINT)(*m_pCurrentToken);
        m_pCurrentToken++;
        break;

    case D3D10_SB_OPCODE_DCL_INDEXABLE_TEMP:
        pInstruction->m_IndexableTempDecl.IndexableTempNumber = (UINT)(*m_pCurrentToken);
        m_pCurrentToken++;
        pInstruction->m_IndexableTempDecl.NumRegisters  = (UINT)(*m_pCurrentToken);
        m_pCurrentToken++;
        switch( min( 4, max( 1, (UINT)(*m_pCurrentToken) ) ) )
        {
        case 1:
            pInstruction->m_IndexableTempDecl.Mask = D3D10_SB_OPERAND_4_COMPONENT_MASK_X;
            break;
        case 2:
            pInstruction->m_IndexableTempDecl.Mask = D3D10_SB_OPERAND_4_COMPONENT_MASK_X |
                                                     D3D10_SB_OPERAND_4_COMPONENT_MASK_Y;
            break;
        case 3:
            pInstruction->m_IndexableTempDecl.Mask = D3D10_SB_OPERAND_4_COMPONENT_MASK_X |
                                                     D3D10_SB_OPERAND_4_COMPONENT_MASK_Y |
                                                     D3D10_SB_OPERAND_4_COMPONENT_MASK_Z;
            break;
        case 4:
            pInstruction->m_IndexableTempDecl.Mask = D3D10_SB_OPERAND_4_COMPONENT_MASK_ALL;
            break;
        }
        m_pCurrentToken++;
        break;

    case D3D10_SB_OPCODE_DCL_INPUT:
    case D3D10_SB_OPCODE_DCL_OUTPUT:
        IFC (ParseOperand(&pInstruction->m_Operands[0]));
        break;

    case D3D10_SB_OPCODE_DCL_INPUT_SIV:
        IFC (ParseOperand(&pInstruction->m_Operands[0]));
        pInstruction->m_InputDeclSIV.Name = DECODE_D3D10_SB_NAME(*m_pCurrentToken);
        m_pCurrentToken++;
        break;

    case D3D10_SB_OPCODE_DCL_INPUT_SGV:
        IFC (ParseOperand(&pInstruction->m_Operands[0]));
        pInstruction->m_InputDeclSIV.Name = DECODE_D3D10_SB_NAME(*m_pCurrentToken);
        m_pCurrentToken++;
        break;

    case D3D10_SB_OPCODE_DCL_INPUT_PS:
        pInstruction->m_InputPSDecl.InterpolationMode = DECODE_D3D10_SB_INPUT_INTERPOLATION_MODE(Token);
        IFC (ParseOperand(&pInstruction->m_Operands[0]));
        break;

    case D3D10_SB_OPCODE_DCL_INPUT_PS_SIV:
        pInstruction->m_InputPSDeclSIV.InterpolationMode = DECODE_D3D10_SB_INPUT_INTERPOLATION_MODE(Token);
        IFC (ParseOperand(&pInstruction->m_Operands[0]));
        pInstruction->m_InputPSDeclSIV.Name = DECODE_D3D10_SB_NAME(*m_pCurrentToken);
        m_pCurrentToken++;
        break;

    case D3D10_SB_OPCODE_DCL_INPUT_PS_SGV:
        pInstruction->m_InputPSDeclSGV.InterpolationMode = DECODE_D3D10_SB_INPUT_INTERPOLATION_MODE(Token);
        IFC (ParseOperand(&pInstruction->m_Operands[0]));
        pInstruction->m_InputPSDeclSGV.Name = DECODE_D3D10_SB_NAME(*m_pCurrentToken);
        m_pCurrentToken++;
        break;

    case D3D10_SB_OPCODE_DCL_OUTPUT_SIV:
        IFC (ParseOperand(&pInstruction->m_Operands[0]));
        pInstruction->m_OutputDeclSIV.Name = DECODE_D3D10_SB_NAME(*m_pCurrentToken);
        m_pCurrentToken++;
        break;

    case D3D10_SB_OPCODE_DCL_OUTPUT_SGV:
        IFC (ParseOperand(&pInstruction->m_Operands[0]));
        pInstruction->m_OutputDeclSGV.Name = DECODE_D3D10_SB_NAME(*m_pCurrentToken);
        m_pCurrentToken++;
        break;

    case D3D10_SB_OPCODE_DCL_INDEX_RANGE:
        IFC (ParseOperand(&pInstruction->m_Operands[0]));
        pInstruction->m_IndexRangeDecl.RegCount = (UINT)(*m_pCurrentToken);
        m_pCurrentToken++;
        break;

    case D3D10_SB_OPCODE_DCL_CONSTANT_BUFFER:
        pInstruction->m_ResourceDecl.CBInfo.AccessPattern = DECODE_D3D10_SB_CONSTANT_BUFFER_ACCESS_PATTERN(Token);
        IFC (ParseOperand(&pInstruction->m_Operands[0]));
        if (pInstruction->m_Operands[0].m_IndexDimension == D3D10_SB_OPERAND_INDEX_3D) // SM 5.1+
            pInstruction->m_ResourceDecl.CBInfo.Size = *m_pCurrentToken++;
        ParseResourceDcl(pInstruction, &pInstruction->m_ResourceDecl.Space);
        break;

    case D3D10_SB_OPCODE_DCL_GS_OUTPUT_PRIMITIVE_TOPOLOGY:
        pInstruction->m_OutputTopologyDecl.Topology = DECODE_D3D10_SB_GS_OUTPUT_PRIMITIVE_TOPOLOGY(Token);
        break;

    case D3D10_SB_OPCODE_DCL_GS_INPUT_PRIMITIVE:
        pInstruction->m_InputPrimitiveDecl.Primitive = DECODE_D3D10_SB_GS_INPUT_PRIMITIVE(Token);
        break;

    case D3D10_SB_OPCODE_DCL_MAX_OUTPUT_VERTEX_COUNT:
        pInstruction->m_GSMaxOutputVertexCountDecl.MaxOutputVertexCount = (UINT)(*m_pCurrentToken);
        m_pCurrentToken++;
        break;

    case D3D11_SB_OPCODE_DCL_GS_INSTANCE_COUNT:
        pInstruction->m_GSInstanceCount.InstanceCount = (UINT)(*m_pCurrentToken);
        m_pCurrentToken++;
        break;

    case D3D11_SB_OPCODE_DCL_UNORDERED_ACCESS_VIEW_RAW:
        pInstruction->m_ResourceDecl.UAVInfo.UAVType = pInstruction->m_OpCode;
        pInstruction->m_ResourceDecl.UAVInfo.Coherency = DECODE_D3D11_SB_ACCESS_COHERENCY_FLAGS(Token);
        pInstruction->m_ResourceDecl.UAVInfo.Dimension = D3D11_SB_RESOURCE_DIMENSION_RAW_BUFFER;
        IFC (ParseOperand(&pInstruction->m_Operands[0]));
        pInstruction->m_ResourceDecl.UAVInfo.Type = (D3D10_SB_RESOURCE_RETURN_TYPE) D3D11_RETURN_TYPE_UINT;
        ParseResourceDcl(pInstruction, &pInstruction->m_ResourceDecl.Space);
        break;

    case D3D11_SB_OPCODE_DCL_UNORDERED_ACCESS_VIEW_STRUCTURED:
        pInstruction->m_ResourceDecl.UAVInfo.UAVType = pInstruction->m_OpCode;
        pInstruction->m_ResourceDecl.UAVInfo.Coherency = DECODE_D3D11_SB_ACCESS_COHERENCY_FLAGS(Token);
        pInstruction->m_ResourceDecl.UAVInfo.Counter = DECODE_D3D11_SB_UAV_FLAGS(Token);
        pInstruction->m_ResourceDecl.UAVInfo.Dimension = D3D11_SB_RESOURCE_DIMENSION_STRUCTURED_BUFFER;
        IFC (ParseOperand(&pInstruction->m_Operands[0]));
        pInstruction->m_ResourceDecl.UAVInfo.Stride = (DWORD) (*m_pCurrentToken++);
        pInstruction->m_ResourceDecl.UAVInfo.Type = (D3D10_SB_RESOURCE_RETURN_TYPE) D3D11_RETURN_TYPE_UINT;
        ParseResourceDcl(pInstruction, &pInstruction->m_ResourceDecl.Space);
        break;

    case D3D11_SB_OPCODE_DCL_UNORDERED_ACCESS_VIEW_TYPED:
        pInstruction->m_ResourceDecl.UAVInfo.UAVType = pInstruction->m_OpCode;
        pInstruction->m_ResourceDecl.UAVInfo.Coherency = DECODE_D3D11_SB_ACCESS_COHERENCY_FLAGS(Token);
        pInstruction->m_ResourceDecl.UAVInfo.Dimension = DECODE_D3D10_SB_RESOURCE_DIMENSION(Token);
        IFC (ParseOperand(&pInstruction->m_Operands[0]));
        pInstruction->m_ResourceDecl.UAVInfo.Type = DECODE_D3D10_SB_RESOURCE_RETURN_TYPE(*m_pCurrentToken++, 0);
        ParseResourceDcl(pInstruction, &pInstruction->m_ResourceDecl.Space);
        break;

    case D3D11_SB_OPCODE_DCL_RESOURCE_RAW:
        pInstruction->m_ResourceDecl.SRVInfo.UAVType = D3D11_SB_OPCODE_DCL_RESOURCE_RAW;
        IFC (ParseOperand(&pInstruction->m_Operands[0]));
        ParseResourceDcl(pInstruction, &pInstruction->m_ResourceDecl.Space);
        break;

    case D3D11_SB_OPCODE_DCL_RESOURCE_STRUCTURED:
        pInstruction->m_ResourceDecl.SRVInfo.UAVType = D3D11_SB_OPCODE_DCL_RESOURCE_STRUCTURED;
        IFC (ParseOperand(&pInstruction->m_Operands[0]));
        pInstruction->m_ResourceDecl.SRVInfo.Stride = (DWORD) (*m_pCurrentToken++);
        ParseResourceDcl(pInstruction, &pInstruction->m_ResourceDecl.Space);
        break;

    case D3D11_SB_OPCODE_DCL_THREAD_GROUP_SHARED_MEMORY_RAW:
        pInstruction->m_TGSMInfo.Type = D3D11_SB_OPCODE_DCL_THREAD_GROUP_SHARED_MEMORY_RAW;
        m_pCurrentToken++;
        pInstruction->m_TGSMInfo.Index = (DWORD)(*m_pCurrentToken);
        m_pCurrentToken++;
        pInstruction->m_TGSMInfo.StructCount = (DWORD)(*m_pCurrentToken);
        pInstruction->m_TGSMInfo.StructStride = 4;
        break;

    case D3D11_SB_OPCODE_DCL_THREAD_GROUP_SHARED_MEMORY_STRUCTURED:
        pInstruction->m_TGSMInfo.Type = D3D11_SB_OPCODE_DCL_THREAD_GROUP_SHARED_MEMORY_STRUCTURED;
        m_pCurrentToken++;
        pInstruction->m_TGSMInfo.Index = (DWORD)(*m_pCurrentToken);
        m_pCurrentToken++;
        pInstruction->m_TGSMInfo.StructStride = (DWORD)(*m_pCurrentToken);
        m_pCurrentToken++;
        pInstruction->m_TGSMInfo.StructCount = (DWORD)(*m_pCurrentToken);
        break;

    case D3D10_SB_OPCODE_DCL_GLOBAL_FLAGS:
        pInstruction->m_GlobalFlagsDecl.Flags = DECODE_D3D10_SB_GLOBAL_FLAGS(Token);
        break;

    case D3D10_SB_OPCODE_RESINFO:
        pInstruction->m_ResInfoReturnType = DECODE_D3D10_SB_RESINFO_INSTRUCTION_RETURN_TYPE(Token);
        IFC (ParseOperand(&pInstruction->m_Operands[0]));
        IFC (ParseOperand(&pInstruction->m_Operands[1]));
        IFC (ParseOperand(&pInstruction->m_Operands[2]));
        break;

    case D3D10_1_SB_OPCODE_SAMPLE_INFO:
        pInstruction->m_InstructionReturnType = DECODE_D3D10_SB_INSTRUCTION_RETURN_TYPE(Token);
        IFC (ParseOperand(&pInstruction->m_Operands[0]));
        IFC (ParseOperand(&pInstruction->m_Operands[1]));
        break;

    case D3D10_SB_OPCODE_IF:
    case D3D10_SB_OPCODE_BREAKC:
    case D3D10_SB_OPCODE_CONTINUEC:
    case D3D10_SB_OPCODE_RETC:
    case D3D10_SB_OPCODE_DISCARD:
        pInstruction->SetTest(DECODE_D3D10_SB_INSTRUCTION_TEST_BOOLEAN(Token));
        IFC (ParseOperand(&pInstruction->m_Operands[0]));
        break;

    case D3D10_SB_OPCODE_CALLC:
        pInstruction->SetTest(DECODE_D3D10_SB_INSTRUCTION_TEST_BOOLEAN(Token));
        IFC (ParseOperand(&pInstruction->m_Operands[0]));
        IFC (ParseOperand(&pInstruction->m_Operands[1]));
        break;

    case D3D11_SB_OPCODE_DCL_THREAD_GROUP:
        for(int i = 0 ; i < 3; i++)
        {
            COperandBase* pOperand = &pInstruction->m_Operands[i];
            pOperand->m_NumComponents = D3D10_SB_OPERAND_1_COMPONENT;
            pOperand->m_Type = D3D10_SB_OPERAND_TYPE_IMMEDIATE32;
            pOperand->m_ComponentName = D3D10_SB_4_COMPONENT_X;
            pOperand->m_ComponentSelection = D3D10_SB_OPERAND_4_COMPONENT_SELECT_1_MODE;
            pOperand->m_Value[0] = *m_pCurrentToken++;
        }
        break;

    case D3D11_SB_OPCODE_DCL_FUNCTION_BODY:
        pInstruction->m_FunctionBodyIdx = *m_pCurrentToken++;
        if(pInstruction->m_FunctionBodyIdx < 0) IFC(E_OUTOFMEMORY);
        break;

    case D3D11_SB_OPCODE_DCL_FUNCTION_TABLE:
        pInstruction->m_FunctionTable.FtIdx = *m_pCurrentToken++;
        if(pInstruction->m_FunctionTable.FtIdx < 0) IFC(E_OUTOFMEMORY);
        pInstruction->m_FunctionTable.FbCount = *m_pCurrentToken++;
        pInstruction->m_FunctionTable.pFbStartToken = m_pCurrentToken;
        m_pCurrentToken += pInstruction->m_FunctionTable.FbCount;
        break;

    case D3D11_SB_OPCODE_DCL_INTERFACE:
        pInstruction->m_InterfaceTable.bDynamic = DECODE_D3D11_SB_INTERFACE_INDEXED_BIT(Token);
        pInstruction->m_InterfaceTable.FpIdx = *m_pCurrentToken++;
        if(pInstruction->m_InterfaceTable.FpIdx < 0) IFC(E_OUTOFMEMORY);
        pInstruction->m_InterfaceTable.NumCallSites = *m_pCurrentToken++;
        pInstruction->m_InterfaceTable.FtCount = DECODE_D3D11_SB_INTERFACE_TABLE_LENGTH(*m_pCurrentToken);
        pInstruction->m_InterfaceTable.FpArraySize = DECODE_D3D11_SB_INTERFACE_ARRAY_LENGTH(*m_pCurrentToken++);
        pInstruction->m_InterfaceTable.pFtStartToken = m_pCurrentToken;
        m_pCurrentToken += pInstruction->m_InterfaceTable.FtCount;
        break;

    case D3D11_SB_OPCODE_INTERFACE_CALL:
        pInstruction->m_InterfaceCallSiteIdx = *m_pCurrentToken++;
        if(pInstruction->m_InterfaceCallSiteIdx < 0) IFC(E_OUTOFMEMORY);
        IFC (ParseOperand(&pInstruction->m_Operands[0]));
        break;

    case D3D11_SB_OPCODE_DCL_INPUT_CONTROL_POINT_COUNT:
        pInstruction->m_InputControlPointCountDecl.InputControlPointCount = DECODE_D3D11_SB_INPUT_CONTROL_POINT_COUNT(Token);
        break;
    case D3D11_SB_OPCODE_DCL_OUTPUT_CONTROL_POINT_COUNT:
        pInstruction->m_OutputControlPointCountDecl.OutputControlPointCount = DECODE_D3D11_SB_OUTPUT_CONTROL_POINT_COUNT(Token);
        break;
    case D3D11_SB_OPCODE_DCL_TESS_DOMAIN:
        pInstruction->m_TessellatorDomainDecl.TessellatorDomain = DECODE_D3D11_SB_TESS_DOMAIN(Token);
        break;
    case D3D11_SB_OPCODE_DCL_TESS_PARTITIONING:
        pInstruction->m_TessellatorPartitioningDecl.TessellatorPartitioning = DECODE_D3D11_SB_TESS_PARTITIONING(Token);
        break;
    case D3D11_SB_OPCODE_DCL_TESS_OUTPUT_PRIMITIVE:
        pInstruction->m_TessellatorOutputPrimitiveDecl.TessellatorOutputPrimitive = DECODE_D3D11_SB_TESS_OUTPUT_PRIMITIVE(Token);
        break;
    case D3D11_SB_OPCODE_DCL_HS_MAX_TESSFACTOR:
        pInstruction->m_HSMaxTessFactorDecl.MaxTessFactor = (float)*m_pCurrentToken++;
        break;
    case D3D11_SB_OPCODE_DCL_HS_FORK_PHASE_INSTANCE_COUNT:
        pInstruction->m_HSForkPhaseDecl.InstanceCount = *m_pCurrentToken++;
        break;
    case D3D11_SB_OPCODE_DCL_HS_JOIN_PHASE_INSTANCE_COUNT:
        pInstruction->m_HSJoinPhaseDecl.InstanceCount = *m_pCurrentToken++;
        break;
    case D3D11_SB_OPCODE_HS_CONTROL_POINT_PHASE:
    case D3D11_SB_OPCODE_HS_FORK_PHASE:
    case D3D11_SB_OPCODE_HS_JOIN_PHASE:
           //m_pCurrentToken++;
        break;
    case D3D11_SB_OPCODE_SYNC:
        {
            UINT SyncFlags = DECODE_D3D11_SB_SYNC_FLAGS(Token);
            pInstruction->m_SyncFlags.bThreadsInGroup = (SyncFlags & D3D11_SB_SYNC_THREADS_IN_GROUP) != 0;
            pInstruction->m_SyncFlags.bThreadGroupSharedMemory = (SyncFlags & D3D11_SB_SYNC_THREAD_GROUP_SHARED_MEMORY) != 0;
            pInstruction->m_SyncFlags.bUnorderedAccessViewMemoryGlobal = (SyncFlags & D3D11_SB_SYNC_UNORDERED_ACCESS_VIEW_MEMORY_GLOBAL) != 0;
            pInstruction->m_SyncFlags.bUnorderedAccessViewMemoryGroup = (SyncFlags & D3D11_SB_SYNC_UNORDERED_ACCESS_VIEW_MEMORY_GROUP) != 0;
        }
        break;

    default:
        //
        // It is assumed that operands follow the (extended) instruction token immediatedly.
        //
        for (UINT i = 0; i < pInstruction->m_NumOperands; i++)
        {
            IFC (ParseOperand(&pInstruction->m_Operands[i]));
        }
        break;
    }

    //
    // If ParseOperand() pushed m_pCurrentToken beyond the length of the instruction
    // encoded in the instruction, then something went very wrong.
    //
    // Suggestion: replace <= with a stricter == check to make sure that we do parse instructions correctly.
    //
    assert(m_pCurrentToken <= (pStart + InstructionLength));

    m_pCurrentToken = pStart + InstructionLength;

    m_NumParsedInstructions++;

Cleanup:
    RRETURN (hr);
}

void CShaderCodeParser::Advance(UINT InstructionSize)
{
    m_pCurrentToken += InstructionSize;
    m_NumParsedInstructions++;
}

D3D10_SB_OPCODE_TYPE CShaderCodeParser::CurrentOpcode()
{
    assert(!EndOfShader());

    CONST DWORD CurrentToken = *m_pCurrentToken;

    return DECODE_D3D10_SB_OPCODE_TYPE(CurrentToken);
}

DWORD CShaderCodeParser::CurrentInstructionLength()
{
    assert(!EndOfShader());

    CONST D3D10_SB_OPCODE_TYPE OpCode = DECODE_D3D10_SB_OPCODE_TYPE(m_pCurrentToken[0]);

    if(D3D10_SB_OPCODE_CUSTOMDATA == OpCode)
    {
        return m_pCurrentToken[1];
    }
    else if (DECODE_IS_D3D10_SB_OPCODE_EXTENDED(m_pCurrentToken[0]))
    {
        if (OpCode == D3D11_SB_OPCODE_DCL_INTERFACE ||
            OpCode == D3D11_SB_OPCODE_DCL_FUNCTION_TABLE)
        {
            return m_pCurrentToken[1];
        }
    }

    return DECODE_D3D10_SB_TOKENIZED_INSTRUCTION_LENGTH(m_pCurrentToken[0]);
}

// ****************************************************************************
//
// class CShaderAsm
//
// ****************************************************************************

void CShaderAsm::EmitOperand(const COperandBase& operand)
{
    if (FAILED(m_Status))
        return;

    HRESULT hr = S_OK;

    CShaderToken Token = ENCODE_D3D10_SB_OPERAND_TYPE(operand.m_Type) |
                            ENCODE_D3D10_SB_OPERAND_NUM_COMPONENTS(operand.m_NumComponents) |
                            ENCODE_D3D10_SB_OPERAND_EXTENDED(operand.m_bExtendedOperand);

    BOOL bProcessOperandIndices = FALSE;
    if (!(operand.m_Type == D3D10_SB_OPERAND_TYPE_IMMEDIATE32 ||
          operand.m_Type == D3D10_SB_OPERAND_TYPE_IMMEDIATE64))
    {
        Token |= ENCODE_D3D10_SB_OPERAND_INDEX_DIMENSION(operand.m_IndexDimension);
        if (operand.m_NumComponents == D3D10_SB_OPERAND_4_COMPONENT)
        {
            // Component selection mode
            Token |= ENCODE_D3D10_SB_OPERAND_4_COMPONENT_SELECTION_MODE(operand.m_ComponentSelection);
            switch(operand.m_ComponentSelection)
            {
            case D3D10_SB_OPERAND_4_COMPONENT_MASK_MODE:
                Token |= ENCODE_D3D10_SB_OPERAND_4_COMPONENT_MASK(operand.m_WriteMask );
                break;
            case D3D10_SB_OPERAND_4_COMPONENT_SWIZZLE_MODE:
                Token |= ENCODE_D3D10_SB_OPERAND_4_COMPONENT_SWIZZLE(operand.m_Swizzle[0],
                                                                operand.m_Swizzle[1],
                                                                operand.m_Swizzle[2],
                                                                operand.m_Swizzle[3]);
                break;
            case D3D10_SB_OPERAND_4_COMPONENT_SELECT_1_MODE:
                Token |= ENCODE_D3D10_SB_OPERAND_4_COMPONENT_SELECT_1(operand.m_ComponentName);
                break;
            default:
                hr = E_INVALIDARG;
                goto Cleanup;
            }
        }

        UINT NumDimensions = operand.m_IndexDimension;
        if (NumDimensions > 0)
        {
            bProcessOperandIndices = TRUE;
            // Encode index representation
            for (UINT i=0; i < NumDimensions; i++)
            {
                Token |= ENCODE_D3D10_SB_OPERAND_INDEX_REPRESENTATION(i, operand.m_IndexType[i]);
            }
        }
        FUNC(Token);
    }

    // Extended operand
    if (operand.m_bExtendedOperand)
    {
        Token = ENCODE_D3D10_SB_EXTENDED_OPERAND_TYPE(operand.m_ExtendedOperandType);
        if (operand.m_ExtendedOperandType == D3D10_SB_EXTENDED_OPERAND_MODIFIER)
        {
            Token |= ENCODE_D3D10_SB_EXTENDED_OPERAND_MODIFIER(operand.m_Modifier);
        }
        FUNC(Token);
    }

    if (operand.m_Type == D3D10_SB_OPERAND_TYPE_IMMEDIATE32 ||
        operand.m_Type == D3D10_SB_OPERAND_TYPE_IMMEDIATE64)
    {
        FUNC(Token);
        UINT n = 0;
        if (operand.m_NumComponents == D3D10_SB_OPERAND_4_COMPONENT)
            n = 4;
        else
        if (operand.m_NumComponents == D3D10_SB_OPERAND_1_COMPONENT)
            n = 1;
        else
        {
            hr = E_INVALIDARG;
            goto Cleanup;
        }
        for (UINT i=0 ; i < n; i++)
        {
            FUNC(operand.m_Value[i]);
        }
    }

    // Operand indices
    if (bProcessOperandIndices)
    {
        const UINT NumDimensions = operand.m_IndexDimension;
        // Encode index representation
        for (UINT i=0; i < NumDimensions; i++)
        {
            switch (operand.m_IndexType[i])
            {
            case D3D10_SB_OPERAND_INDEX_IMMEDIATE32:
                FUNC(operand.m_Index[i].m_RegIndex);
                break;
            case D3D10_SB_OPERAND_INDEX_IMMEDIATE64:
                FUNC(operand.m_Index[i].m_RegIndexA[0]);
                FUNC(operand.m_Index[i].m_RegIndexA[1]);
                break;
            case D3D10_SB_OPERAND_INDEX_IMMEDIATE32_PLUS_RELATIVE:
                FUNC(operand.m_Index[i].m_RegIndex);
                // Fall through
            case D3D10_SB_OPERAND_INDEX_RELATIVE:
                {
                    D3D10_SB_OPERAND_TYPE RelRegType = operand.m_Index[i].m_RelRegType;
                    if( operand.m_Index[i].m_IndexDimension == D3D10_SB_OPERAND_INDEX_2D )
                    {
                        EmitOperand(COperand2D(RelRegType,
                                                    operand.m_Index[i].m_RelIndex,
                                                    operand.m_Index[i].m_RelIndex1,
                                               operand.m_Index[i].m_ComponentName));
                    }
                    else
                    {
                        EmitOperand(COperand4(RelRegType,
                                                   operand.m_Index[i].m_RelIndex,
                                              operand.m_Index[i].m_ComponentName));
                    }
                }
                break;
            default:
                hr = E_INVALIDARG;
                goto Cleanup;
            }
        }
    }

Cleanup:
    if (FAILED(hr))
        m_Status = hr;
}

//-----------------------------------------------------------------------------
void CShaderAsm::EmitInstruction(const CInstruction& instruction)
{
    UINT  OpCode;

    if(instruction.m_OpCode == D3D10_SB_OPCODE_CUSTOMDATA)
    {
        OPCODE(D3D10_SB_OPCODE_CUSTOMDATA);
        FUNC(instruction.m_CustomData.DataSizeInBytes/4 + 2);
        for(UINT i = 0;i < instruction.m_CustomData.DataSizeInBytes/4; i++)
            FUNC(((UINT*)instruction.m_CustomData.pData)[i]);

        ENDINSTRUCTION();
        return;
    }

    OpCode = ENCODE_D3D10_SB_OPCODE_TYPE(instruction.m_OpCode) | ENCODE_D3D10_SB_OPCODE_EXTENDED(instruction.m_bExtended);
    switch (instruction.m_OpCode)
    {
    case D3D10_SB_OPCODE_IF:
    case D3D10_SB_OPCODE_BREAKC:
    case D3D10_SB_OPCODE_CALLC:
    case D3D10_SB_OPCODE_CONTINUEC:
    case D3D10_SB_OPCODE_RETC:
    case D3D10_SB_OPCODE_DISCARD:
        OpCode |= ENCODE_D3D10_SB_INSTRUCTION_TEST_BOOLEAN(instruction.Test());
        break;
    case D3D10_SB_OPCODE_RESINFO:
        OpCode |= ENCODE_D3D10_SB_RESINFO_INSTRUCTION_RETURN_TYPE(instruction.m_ResInfoReturnType);
        break;
    case D3D10_1_SB_OPCODE_SAMPLE_INFO:
        OpCode |= ENCODE_D3D10_SB_INSTRUCTION_RETURN_TYPE(instruction.m_InstructionReturnType);
        break;
    case D3D11_SB_OPCODE_SYNC:
        OpCode |= ENCODE_D3D11_SB_SYNC_FLAGS(
                  ( instruction.m_SyncFlags.bThreadsInGroup ? D3D11_SB_SYNC_THREADS_IN_GROUP : 0 ) |
                  ( instruction.m_SyncFlags.bThreadGroupSharedMemory ? D3D11_SB_SYNC_THREAD_GROUP_SHARED_MEMORY : 0 ) |
                  ( instruction.m_SyncFlags.bUnorderedAccessViewMemoryGlobal ? D3D11_SB_SYNC_UNORDERED_ACCESS_VIEW_MEMORY_GLOBAL : 0 ) |
                  ( instruction.m_SyncFlags.bUnorderedAccessViewMemoryGroup ? D3D11_SB_SYNC_UNORDERED_ACCESS_VIEW_MEMORY_GROUP : 0 ) );
        break;
    };
    OpCode |= ENCODE_D3D10_SB_INSTRUCTION_SATURATE(instruction.m_bSaturate);
    OPCODE(OpCode);

    for(UINT i = 0; i < min(instruction.m_ExtendedOpCodeCount,D3D11_SB_MAX_SIMULTANEOUS_EXTENDED_OPCODES); i++)
    {
        UINT  Extended = ENCODE_D3D10_SB_EXTENDED_OPCODE_TYPE(instruction.m_OpCodeEx[i]);
        switch( instruction.m_OpCodeEx[i] )
        {
        case D3D10_SB_EXTENDED_OPCODE_SAMPLE_CONTROLS:
            {
                Extended |= ENCODE_IMMEDIATE_D3D10_SB_ADDRESS_OFFSET(D3D10_SB_IMMEDIATE_ADDRESS_OFFSET_U, instruction.m_TexelOffset[0]);
                Extended |= ENCODE_IMMEDIATE_D3D10_SB_ADDRESS_OFFSET(D3D10_SB_IMMEDIATE_ADDRESS_OFFSET_V, instruction.m_TexelOffset[1]);
                Extended |= ENCODE_IMMEDIATE_D3D10_SB_ADDRESS_OFFSET(D3D10_SB_IMMEDIATE_ADDRESS_OFFSET_W, instruction.m_TexelOffset[2]);
            }
            break;
        }
        Extended |= ENCODE_D3D10_SB_OPCODE_EXTENDED((i + 1 < instruction.m_ExtendedOpCodeCount) ? true : false);
        FUNC(Extended);
    }

    for (UINT i=0; i < instruction.m_NumOperands; i++)
    {
        EmitOperand(instruction.m_Operands[i]);
    }

    ENDINSTRUCTION();
}

//-----------------------------------------------------------------------------
void CShaderAsm::EmitBinary(CONST DWORD* Ptr, DWORD Count)
{
    Reserve(Count);

    if (SUCCEEDED(m_Status))
    {
        assert((m_Index + Count) <= m_BufferSize);

        CONST D3D10_SB_OPCODE_TYPE OpCode = DECODE_D3D10_SB_OPCODE_TYPE(Ptr[0]);

        if(D3D10_SB_OPCODE_CUSTOMDATA == OpCode)
        {
            CONST D3D10_SB_CUSTOMDATA_CLASS Class = DECODE_D3D10_SB_CUSTOMDATA_CLASS(Ptr[0]);
            CONST DWORD DataSize = Ptr[1];
            assert(DataSize == Count);
            assert(DataSize >= 2);

            EmitCustomData(Class, DataSize - 2, (CONST UINT*)(Ptr + 2));
        }
        else
        {
            assert((m_Index + Count) <= m_BufferSize);

            memcpy(m_dwFunc + m_Index, Ptr, sizeof(DWORD) * Count);

            m_StartOpIndex = m_Index; // so that ENDINSTRUCTION writes the instruction length to the correct place

            m_Index += Count;

            ENDINSTRUCTION();
        }
    }
}

