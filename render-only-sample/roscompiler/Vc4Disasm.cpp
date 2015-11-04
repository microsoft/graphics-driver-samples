#include "roscompiler.h"
#include "Vc4Disasm.hpp"

#if VC4

static char* VC4_QPU_Name_Signaling_Bits[VC4_QPU_SIG_ARRAY_SIZE] = { NULL };
static char* VC4_QPU_Name_Unpack[VC4_QPU_UNPACK_ARRAY_SIZE] = { NULL };
static char* VC4_QPU_Name_Pack_A[VC4_QPU_PACK_A_ARRAY_SIZE] = { NULL };
static char* VC4_QPU_Name_Pack_Mul[VC4_QPU_PACK_MUL_ARRAY_SIZE] = { NULL };
static char* VC4_QPU_Name_Cond[VC4_QPU_COND_ARRAY_SIZE] = { NULL };
static char* VC4_QPU_Name_Waddr[VC4_QPU_WADDR_ARRAY_SIZE][2] = { NULL }; // [0] for regfile A, [1] for regfile B
static char* VC4_QPU_Name_Op_Mul[VC4_QPU_OPCODE_MUL_ARRAY_SIZE] = { NULL };
static char* VC4_QPU_Name_Op_Add[VC4_QPU_OPCODE_ADD_ARRAY_SIZE] = { NULL };
static char* VC4_QPU_Name_Op_Move = NULL;
static char* VC4_QPU_Name_Raddr[VC4_QPU_RADDR_ARRAY_SIZE][2] = { NULL }; // [0] for regfile A, [1] for regfile B
static char* VC4_QPU_Name_Alu[VC4_QPU_ALU_ARRAY_SIZE] = { NULL };
static char* VC4_QPU_Name_Branch_Cond[VC4_QPU_BRANCH_COND_ARRAY_SIZE] = { NULL };
static char* VC4_QPU_Name_SetFlag = NULL;
static char* VC_QPU_Name_Empty = NULL;

void __stdcall VC4_InitializeName()
{
	VC4_QPU_Name_Signaling_Bits[VC4_QPU_SIG_BREAK] = "bkpt";
	VC4_QPU_Name_Signaling_Bits[VC4_QPU_SIG_NO_SIGNAL] = "";
	VC4_QPU_Name_Signaling_Bits[VC4_QPU_SIG_THREAD_SWITCH] = "thrsw";
	VC4_QPU_Name_Signaling_Bits[VC4_QPU_SIG_PROGRAM_END] = "thrend";
	VC4_QPU_Name_Signaling_Bits[VC4_QPU_SIG_WAIT_FOR_SCOREBOARD] = "sbwait";
	VC4_QPU_Name_Signaling_Bits[VC4_QPU_SIG_SCOREBOARD_UNBLOCK] = "sbdone";
	VC4_QPU_Name_Signaling_Bits[VC4_QPU_SIG_LAST_THREAD_SWITCH] = "lthrsw";
	VC4_QPU_Name_Signaling_Bits[VC4_QPU_SIG_COVERAGE_LOAD] = "loadcv";
	VC4_QPU_Name_Signaling_Bits[VC4_QPU_SIG_COLOR_LOAD] = "loadc";
	VC4_QPU_Name_Signaling_Bits[VC4_QPU_SIG_COLOR_LOAD_AND_PROGRAM_END] = "ldcend";
	VC4_QPU_Name_Signaling_Bits[VC4_QPU_SIG_LOAD_TMU0] = "ldtmu0";
	VC4_QPU_Name_Signaling_Bits[VC4_QPU_SIG_LOAD_TMU1] = "ldtmu1";
	VC4_QPU_Name_Signaling_Bits[VC4_QPU_SIG_ALPAH_MASK_LOAD] = "loadam";
	VC4_QPU_Name_Signaling_Bits[VC4_QPU_SIG_ALU_WITH_RADDR_B] = "loadsm";
	VC4_QPU_Name_Signaling_Bits[VC4_QPU_SIG_LOAD_IMMEDIATE] = "loadim";
	VC4_QPU_Name_Signaling_Bits[VC4_QPU_SIG_BRANCH] = "branch";

	VC4_QPU_Name_Unpack[VC4_QPU_UNPACK_32] = "";
	VC4_QPU_Name_Unpack[VC4_QPU_UNPACK_16a] = ".16a";
	VC4_QPU_Name_Unpack[VC4_QPU_UNPACK_16b] = ".16b";
	VC4_QPU_Name_Unpack[VC4_QPU_UNPACK_8d_REP] = ".8d_replicate";
	VC4_QPU_Name_Unpack[VC4_QPU_UNPACK_8a] = ".8a";
	VC4_QPU_Name_Unpack[VC4_QPU_UNPACK_8b] = ".8b";
	VC4_QPU_Name_Unpack[VC4_QPU_UNPACK_8c] = ".8c";
	VC4_QPU_Name_Unpack[VC4_QPU_UNPACK_8d] = ".8d";

	VC4_QPU_Name_Pack_A[VC4_QPU_PACK_A_32] = "";
	VC4_QPU_Name_Pack_A[VC4_QPU_PACK_A_16a] = ".16a";
	VC4_QPU_Name_Pack_A[VC4_QPU_PACK_A_16b] = ".16b";
	VC4_QPU_Name_Pack_A[VC4_QPU_PACK_A_8888] = ".8888";
	VC4_QPU_Name_Pack_A[VC4_QPU_PACK_A_8a] = ".8a";
	VC4_QPU_Name_Pack_A[VC4_QPU_PACK_A_8b] = ".8b";
	VC4_QPU_Name_Pack_A[VC4_QPU_PACK_A_8c] = ".8c";
	VC4_QPU_Name_Pack_A[VC4_QPU_PACK_A_8d] = ".8d";
	VC4_QPU_Name_Pack_A[VC4_QPU_PACK_A_32_SAT] = ".32_saturate";
	VC4_QPU_Name_Pack_A[VC4_QPU_PACK_A_16a_SAT] = ".16a_saturate";
	VC4_QPU_Name_Pack_A[VC4_QPU_PACK_A_16b_SAT] = ".16b_saturate";
	VC4_QPU_Name_Pack_A[VC4_QPU_PACK_A_8888_SAT] = ".8888_saturate";
	VC4_QPU_Name_Pack_A[VC4_QPU_PACK_A_8a_SAT] = ".8a_saturate";
	VC4_QPU_Name_Pack_A[VC4_QPU_PACK_A_8b_SAT] = ".8b_saturate";
	VC4_QPU_Name_Pack_A[VC4_QPU_PACK_A_8c_SAT] = ".8c_saturate";
	VC4_QPU_Name_Pack_A[VC4_QPU_PACK_A_8d_SAT] = ".8d_saturate";

	VC4_QPU_Name_Pack_Mul[VC4_QPU_PACK_MUL_32] = "";
	VC4_QPU_Name_Pack_Mul[VC4_QPU_PACK_MUL_8888] = ".8888";
	VC4_QPU_Name_Pack_Mul[VC4_QPU_PACK_MUL_8a] = ".8a";
	VC4_QPU_Name_Pack_Mul[VC4_QPU_PACK_MUL_8b] = ".8b";
	VC4_QPU_Name_Pack_Mul[VC4_QPU_PACK_MUL_8c] = ".8c";
	VC4_QPU_Name_Pack_Mul[VC4_QPU_PACK_MUL_8d] = ".8d";

	VC4_QPU_Name_Cond[VC4_QPU_COND_NEVER] = ".never";
	VC4_QPU_Name_Cond[VC4_QPU_COND_ALWAYS] = "";
	VC4_QPU_Name_Cond[VC4_QPU_COND_ZS] = ".if_zs";
	VC4_QPU_Name_Cond[VC4_QPU_COND_ZC] = ".if_zc";
	VC4_QPU_Name_Cond[VC4_QPU_COND_NS] = ".if_ns";
	VC4_QPU_Name_Cond[VC4_QPU_COND_NC] = ".if_nc";
	VC4_QPU_Name_Cond[VC4_QPU_COND_CS] = ".if_cs";
	VC4_QPU_Name_Cond[VC4_QPU_COND_CC] = ".if_cc";

	VC4_QPU_Name_Waddr[VC4_QPU_WADDR_ACC0][0] =
	VC4_QPU_Name_Waddr[VC4_QPU_WADDR_ACC0][1] = "r0";
	VC4_QPU_Name_Waddr[VC4_QPU_WADDR_ACC1][0] =
	VC4_QPU_Name_Waddr[VC4_QPU_WADDR_ACC1][1] = "r1";
	VC4_QPU_Name_Waddr[VC4_QPU_WADDR_ACC2][0] =
	VC4_QPU_Name_Waddr[VC4_QPU_WADDR_ACC2][1] = "r2";
	VC4_QPU_Name_Waddr[VC4_QPU_WADDR_ACC3][0] =
	VC4_QPU_Name_Waddr[VC4_QPU_WADDR_ACC3][1] = "r3";
	VC4_QPU_Name_Waddr[VC4_QPU_WADDR_TMU_NOSWAP][0] =
	VC4_QPU_Name_Waddr[VC4_QPU_WADDR_TMU_NOSWAP][1] = "tmu_noswap";
    VC4_QPU_Name_Waddr[VC4_QPU_WADDR_ACC5][0] = "r5_replicate_pixel_0"; // A: replicate pixel 0 per quad.
	VC4_QPU_Name_Waddr[VC4_QPU_WADDR_ACC5][1] = "r5_replicate_SIMD_0"; // B: replicate SIMD element 0.
	VC4_QPU_Name_Waddr[VC4_QPU_WADDR_HOSTINT][0] =
	VC4_QPU_Name_Waddr[VC4_QPU_WADDR_HOSTINT][1] = "hostint";
	VC4_QPU_Name_Waddr[VC4_QPU_WADDR_NOP][0] =
	VC4_QPU_Name_Waddr[VC4_QPU_WADDR_NOP][1] = "";
	VC4_QPU_Name_Waddr[VC4_QPU_WADDR_UNIFORM][0] =
	VC4_QPU_Name_Waddr[VC4_QPU_WADDR_UNIFORM][1] = "uniform";
	VC4_QPU_Name_Waddr[VC4_QPU_WADDR_QUAD_X][0] = "quad_X"; // X for regfile A.
	VC4_QPU_Name_Waddr[VC4_QPU_WADDR_QUAD_Y][1] = "quad_Y"; // Y for regfile B.
	VC4_QPU_Name_Waddr[VC4_QPU_WADDR_MS_FLAGS][0] = "ms_flags"; // regfile A.
	VC4_QPU_Name_Waddr[VC4_QPU_WADDR_REV_FLAG][1] = "rev_flag"; // regfile B.
	VC4_QPU_Name_Waddr[VC4_QPU_WADDR_TLB_STENCIL_SETUP][0] =
	VC4_QPU_Name_Waddr[VC4_QPU_WADDR_TLB_STENCIL_SETUP][1] = "tlb_stencil_setup";
	VC4_QPU_Name_Waddr[VC4_QPU_WADDR_TLB_Z][0] =
	VC4_QPU_Name_Waddr[VC4_QPU_WADDR_TLB_Z][1] = "tlb_z";
	VC4_QPU_Name_Waddr[VC4_QPU_WADDR_TLB_COLOUR_MS][0] =
	VC4_QPU_Name_Waddr[VC4_QPU_WADDR_TLB_COLOUR_MS][1] = "tlb_colour_ms";
	VC4_QPU_Name_Waddr[VC4_QPU_WADDR_TLB_COLOUR_ALL][0] =
	VC4_QPU_Name_Waddr[VC4_QPU_WADDR_TLB_COLOUR_ALL][1] = "tbl_colour";
	VC4_QPU_Name_Waddr[VC4_QPU_WADDR_TLB_ALPHA_MASK][0] =
	VC4_QPU_Name_Waddr[VC4_QPU_WADDR_TLB_ALPHA_MASK][1] = "tbl_alpha_mask";
	VC4_QPU_Name_Waddr[VC4_QPU_WADDR_VPM][0] =
	VC4_QPU_Name_Waddr[VC4_QPU_WADDR_VPM][1] = "vpm";
	VC4_QPU_Name_Waddr[VC4_QPU_WADDR_VPMVCD_RD_SETUP][0] = "vpm_rd_setup"; // regfile A
	VC4_QPU_Name_Waddr[VC4_QPU_WADDR_VPMVCD_WR_SETUP][1] = "vpm_wr_setup"; // regfile B
	VC4_QPU_Name_Waddr[VC4_QPU_WADDR_VPM_LD_ADDR][0] = "vpm_ld_addr"; // regfile A
	VC4_QPU_Name_Waddr[VC4_QPU_WADDR_VPM_ST_ADDR][1] = "vpm_st_addr"; // regfile B
	VC4_QPU_Name_Waddr[VC4_QPU_WADDR_MUTEX_RELEASE][0] =
	VC4_QPU_Name_Waddr[VC4_QPU_WADDR_MUTEX_RELEASE][1] = "mutex_release";
	VC4_QPU_Name_Waddr[VC4_QPU_WADDR_SFU_RECIP][0] =
	VC4_QPU_Name_Waddr[VC4_QPU_WADDR_SFU_RECIP][1] = "sfu_recip";
	VC4_QPU_Name_Waddr[VC4_QPU_WADDR_SFU_RECIPSQRT][0] =
	VC4_QPU_Name_Waddr[VC4_QPU_WADDR_SFU_RECIPSQRT][1] = "sfu_recipsqrt";
	VC4_QPU_Name_Waddr[VC4_QPU_WADDR_SFU_EXP][0] =
	VC4_QPU_Name_Waddr[VC4_QPU_WADDR_SFU_EXP][1] = "sfu_exp";
	VC4_QPU_Name_Waddr[VC4_QPU_WADDR_SFU_LOG][0] =
	VC4_QPU_Name_Waddr[VC4_QPU_WADDR_SFU_LOG][1] = "sfu_log";
	VC4_QPU_Name_Waddr[VC4_QPU_WADDR_TMU0_S][0] =
	VC4_QPU_Name_Waddr[VC4_QPU_WADDR_TMU0_S][1] = "tmu0_s";  // X - retiring
	VC4_QPU_Name_Waddr[VC4_QPU_WADDR_TMU0_T][0] =
	VC4_QPU_Name_Waddr[VC4_QPU_WADDR_TMU0_T][1] = "tmu0_t";  // Y
	VC4_QPU_Name_Waddr[VC4_QPU_WADDR_TMU0_R][0] =
	VC4_QPU_Name_Waddr[VC4_QPU_WADDR_TMU0_R][1] = "tmu0_r";  // Z
	VC4_QPU_Name_Waddr[VC4_QPU_WADDR_TMU0_B][0] =
	VC4_QPU_Name_Waddr[VC4_QPU_WADDR_TMU0_B][1] = "tmu0_b";  // LOD Bias
	VC4_QPU_Name_Waddr[VC4_QPU_WADDR_TMU1_S][0] =
	VC4_QPU_Name_Waddr[VC4_QPU_WADDR_TMU1_S][1] = "tmu1_s";  // X - retiring
	VC4_QPU_Name_Waddr[VC4_QPU_WADDR_TMU1_T][0] =
	VC4_QPU_Name_Waddr[VC4_QPU_WADDR_TMU1_T][1] = "tmu1_t";  // Y
	VC4_QPU_Name_Waddr[VC4_QPU_WADDR_TMU1_R][0] =
	VC4_QPU_Name_Waddr[VC4_QPU_WADDR_TMU1_R][1] = "tmu1_r";  // Z
	VC4_QPU_Name_Waddr[VC4_QPU_WADDR_TMU1_B][0] =
	VC4_QPU_Name_Waddr[VC4_QPU_WADDR_TMU1_B][1] = "tmu1_b";  // LOD Bias

	VC4_QPU_Name_Op_Mul[VC4_QPU_OPCODE_MUL_NOP] = "nop";
	VC4_QPU_Name_Op_Mul[VC4_QPU_OPCODE_MUL_FMUL] = "fmul";
	VC4_QPU_Name_Op_Mul[VC4_QPU_OPCODE_MUL_MUL24] = "mul24";
	VC4_QPU_Name_Op_Mul[VC4_QPU_OPCODE_MUL_V8MULD] = "v8muld";
	VC4_QPU_Name_Op_Mul[VC4_QPU_OPCODE_MUL_V8MIN] = "v8min";
	VC4_QPU_Name_Op_Mul[VC4_QPU_OPCODE_MUL_V8MAX] = "v8max";
	VC4_QPU_Name_Op_Mul[VC4_QPU_OPCODE_MUL_V8ADDS] = "v8add_saturate";
	VC4_QPU_Name_Op_Mul[VC4_QPU_OPCODE_MUL_V8SUBS] = "v8sub_saturate";

	VC4_QPU_Name_Op_Add[VC4_QPU_OPCODE_ADD_NOP] = "nop";
	VC4_QPU_Name_Op_Add[VC4_QPU_OPCODE_ADD_FADD] = "fadd";
	VC4_QPU_Name_Op_Add[VC4_QPU_OPCODE_ADD_FSUB] = "fsub";
	VC4_QPU_Name_Op_Add[VC4_QPU_OPCODE_ADD_FMIN] = "fmin";
	VC4_QPU_Name_Op_Add[VC4_QPU_OPCODE_ADD_FMAX] = "fmax";
	VC4_QPU_Name_Op_Add[VC4_QPU_OPCODE_ADD_FMIN_ABS] = "fmin_abs";
	VC4_QPU_Name_Op_Add[VC4_QPU_OPCODE_ADD_FMAX_ABS] = "fmax_abs";
	VC4_QPU_Name_Op_Add[VC4_QPU_OPCODE_ADD_FTOI] = "ftoi";
	VC4_QPU_Name_Op_Add[VC4_QPU_OPCODE_ADD_ITOF] = "itof";
	VC4_QPU_Name_Op_Add[VC4_QPU_OPCODE_ADD_ADD] = "add";
	VC4_QPU_Name_Op_Add[VC4_QPU_OPCODE_ADD_SUB] = "sub";
	VC4_QPU_Name_Op_Add[VC4_QPU_OPCODE_ADD_SHR] = "shr";
	VC4_QPU_Name_Op_Add[VC4_QPU_OPCODE_ADD_ASR] = "asr";
	VC4_QPU_Name_Op_Add[VC4_QPU_OPCODE_ADD_ROR] = "ror";
	VC4_QPU_Name_Op_Add[VC4_QPU_OPCODE_ADD_SHL] = "shl";
	VC4_QPU_Name_Op_Add[VC4_QPU_OPCODE_ADD_MIN] = "min";
	VC4_QPU_Name_Op_Add[VC4_QPU_OPCODE_ADD_MAX] = "max";
	VC4_QPU_Name_Op_Add[VC4_QPU_OPCODE_ADD_AND] = "and";
	VC4_QPU_Name_Op_Add[VC4_QPU_OPCODE_ADD_OR] = "or";
	VC4_QPU_Name_Op_Add[VC4_QPU_OPCODE_ADD_XOR] = "xor";
	VC4_QPU_Name_Op_Add[VC4_QPU_OPCODE_ADD_NOT] = "not";
	VC4_QPU_Name_Op_Add[VC4_QPU_OPCODE_ADD_CLZ] = "clz";
	VC4_QPU_Name_Op_Add[VC4_QPU_OPCODE_ADD_V8ADDS] = "v8add_saturate";
	VC4_QPU_Name_Op_Add[VC4_QPU_OPCODE_ADD_V8SUBS] = "v8sub_saturate";

    VC4_QPU_Name_Op_Move = "mov";

	VC4_QPU_Name_Raddr[VC4_QPU_RADDR_UNIFORM][0] =
	VC4_QPU_Name_Raddr[VC4_QPU_RADDR_UNIFORM][1] = "uniform";
	VC4_QPU_Name_Raddr[VC4_QPU_RADDR_VERYING][0] =
	VC4_QPU_Name_Raddr[VC4_QPU_RADDR_VERYING][1] = "varying";
	VC4_QPU_Name_Raddr[VC4_QPU_RADDR_ELEMENT_NUMBER][0] = "element_number"; // regfile A
	VC4_QPU_Name_Raddr[VC4_QPU_RADDR_QPU_NUMBER][1] = "qpu_number"; // regfile B
	VC4_QPU_Name_Raddr[VC4_QPU_RADDR_NOP][0] =
	VC4_QPU_Name_Raddr[VC4_QPU_RADDR_NOP][1] = "";
	VC4_QPU_Name_Raddr[VC4_QPU_RADDR_PIXEL_COORD_X][0] = "pixel_coord_x"; // regfile A
	VC4_QPU_Name_Raddr[VC4_QPU_RADDR_PIXEL_COORD_Y][1] = "pixel_coord_y"; // regfile B
	VC4_QPU_Name_Raddr[VC4_QPU_RADDR_MS_FLAGS][0] = "ms_flags"; // regfile A
	VC4_QPU_Name_Raddr[VC4_QPU_RADDR_REV_FLAG][1] = "rev_flag"; // regfile B
	VC4_QPU_Name_Raddr[VC4_QPU_RADDR_VPM][0] =
	VC4_QPU_Name_Raddr[VC4_QPU_RADDR_VPM][1] = "vpm";
	VC4_QPU_Name_Raddr[VC4_QPU_RADDR_VPM_LD_BUSY][0] = "vpm_ld_busy"; // regfile A
	VC4_QPU_Name_Raddr[VC4_QPU_RADDR_VPM_ST_BUSY][1] = "vpm_st_busy"; // regfile B
	VC4_QPU_Name_Raddr[VC4_QPU_RADDR_VPM_LD_WAIT][0] = "vpm_ld_wait"; // regfile A
	VC4_QPU_Name_Raddr[VC4_QPU_RADDR_VPM_ST_WAIT][1] = "vpm_st_wait"; // regfile B
	VC4_QPU_Name_Raddr[VC4_QPU_RADDR_MUTEX_ACQUIRE][0] =
	VC4_QPU_Name_Raddr[VC4_QPU_RADDR_MUTEX_ACQUIRE][1] = "mutex_acquire";

	VC4_QPU_Name_Alu[VC4_QPU_ALU_R0] = "r0";
	VC4_QPU_Name_Alu[VC4_QPU_ALU_R1] = "r1";
	VC4_QPU_Name_Alu[VC4_QPU_ALU_R2] = "r2";
	VC4_QPU_Name_Alu[VC4_QPU_ALU_R3] = "r3";
	VC4_QPU_Name_Alu[VC4_QPU_ALU_R4] = "r4";
	VC4_QPU_Name_Alu[VC4_QPU_ALU_R5] = "r5";
	VC4_QPU_Name_Alu[VC4_QPU_ALU_REG_A] = "ra";
	VC4_QPU_Name_Alu[VC4_QPU_ALU_REG_B] = "rb";

	VC4_QPU_Name_Branch_Cond[VC4_QPU_BRANCH_COND_ALL_ZS] = ".if_all_zs"; // All Z flags set
	VC4_QPU_Name_Branch_Cond[VC4_QPU_BRANCH_COND_ALL_ZC] = ".if_all_zc"; // All Z flags clear
	VC4_QPU_Name_Branch_Cond[VC4_QPU_BRANCH_COND_ANY_ZS] = ".if_any_zs"; // Any Z flags set
	VC4_QPU_Name_Branch_Cond[VC4_QPU_BRANCH_COND_ANY_ZC] = ".if_any_zc"; // Any Z flags clear
	VC4_QPU_Name_Branch_Cond[VC4_QPU_BRANCH_COND_ALL_NS] = ".if_all_ns"; // All N flags set
	VC4_QPU_Name_Branch_Cond[VC4_QPU_BRANCH_COND_ALL_NC] = ".if_all_nc"; // All N flags clear
	VC4_QPU_Name_Branch_Cond[VC4_QPU_BRANCH_COND_ANY_NS] = ".if_any_ns"; // Any N flags set
	VC4_QPU_Name_Branch_Cond[VC4_QPU_BRANCH_COND_ANY_NC] = ".if_any_nc"; // Any N flags clear
	VC4_QPU_Name_Branch_Cond[VC4_QPU_BRANCH_COND_ALL_CS] = ".if_all_cs"; // All C flags set
	VC4_QPU_Name_Branch_Cond[VC4_QPU_BRANCH_COND_ALL_CC] = ".if_all_cc"; // All C flags clear
	VC4_QPU_Name_Branch_Cond[VC4_QPU_BRANCH_COND_ANY_CS] = ".if_any_cs"; // Any C flags set
	VC4_QPU_Name_Branch_Cond[VC4_QPU_BRANCH_COND_ANY_CC] = ".if_any_cc"; // Any C flags clear
	VC4_QPU_Name_Branch_Cond[VC4_QPU_BRANCH_COND_ALWAYS] = ""; // Always execute

    VC4_QPU_Name_SetFlag = ".setFlags";

    VC_QPU_Name_Empty = "";
}

HRESULT Vc4Disasm::ParseSignature(VC4_QPU_INSTRUCTION Instruction)
{
    this->xprintf("%s\t", VC4_QPU_Name_Signaling_Bits[VC4_QPU_GET_SIG(Instruction)]);
    return S_OK;
}

HRESULT Vc4Disasm::ParseSmallImmediate(DWORD dwSmallImmediate)
{
    if (dwSmallImmediate < 16)
    {
        this->xprintf("%d", dwSmallImmediate);
    }
    else if (dwSmallImmediate < 32)
    {
        this->xprintf("%d", -16 + (dwSmallImmediate - 16));
    }
    else if (dwSmallImmediate < 40)
    {
        this->xprintf("%.1f", (float)(1 << (dwSmallImmediate - 32)));
    }
    else if (dwSmallImmediate < 48)
    {
        this->xprintf("%f", 1.0f / (1 << (48 - dwSmallImmediate)));
    }
    else if (dwSmallImmediate == 48)
    {
        this->xprintf("Mul output vector rotation is taken from accumulator r5, element 0, bits [3:0]");
    }
    else if (dwSmallImmediate < 64)
    {
        this->xprintf("Mul output vector rotated by %d upwards (so element 0 moves to element %d)", dwSmallImmediate - 48, dwSmallImmediate - 48);
    }
    else
    {
        this->xprintf("Invalid");
    }
    return S_OK;
 }

HRESULT Vc4Disasm::ParseWriteAddr(DWORD waddr, bool bRegfile_A)
{
    if (waddr < 32)
    {
        this->xprintf("%s%d", bRegfile_A ? VC4_QPU_Name_Alu[VC4_QPU_ALU_REG_A] : VC4_QPU_Name_Alu[VC4_QPU_ALU_REG_B], waddr);
    }
    else if (waddr < VC4_QPU_RADDR_ARRAY_SIZE)
    {
        this->xprintf("%s", VC4_QPU_Name_Waddr[waddr][bRegfile_A ? 0 : 1]);
    }
    else
    {
        this->xprintf("Invalid");
    }
    return S_OK;
}

HRESULT Vc4Disasm::ParseWrite(VC4_QPU_INSTRUCTION Instruction, bool bAddOp, bool bShowPack)
{
    bool bRegfile_A = ((bAddOp == true) == (VC4_QPU_IS_WRITESWAP_SET(Instruction) == 0)) ? true : false;
    ParseWriteAddr(bAddOp ? VC4_QPU_GET_WADDR_ADD(Instruction) : VC4_QPU_GET_WADDR_MUL(Instruction), bRegfile_A);
    if (bShowPack)
    {
        if (bRegfile_A && !VC4_QPU_IS_PM_SET(Instruction))
        {
            this->xprintf("%s", VC4_QPU_Name_Pack_A[VC4_QPU_GET_PACK(Instruction)]);
        }
        else if (!bAddOp && VC4_QPU_IS_PM_SET(Instruction))
        {
            this->xprintf("%s", VC4_QPU_Name_Pack_Mul[VC4_QPU_GET_PACK(Instruction)]);
        }
    }
    return S_OK;
}

HRESULT Vc4Disasm::ParseReadAddr(DWORD raddr, bool bRegfile_A)
{
    if (raddr < 32)
    {
        this->xprintf("%s%d", bRegfile_A ? VC4_QPU_Name_Alu[VC4_QPU_ALU_REG_A] : VC4_QPU_Name_Alu[VC4_QPU_ALU_REG_B], raddr);
    }
    else if (raddr < VC4_QPU_RADDR_ARRAY_SIZE)
    {
        this->xprintf("%s", VC4_QPU_Name_Raddr[raddr][bRegfile_A ? 0 : 1]);
    }
    else
    {
        this->xprintf("Invalid");
    }
    return S_OK;
}

HRESULT Vc4Disasm::ParseRead(VC4_QPU_INSTRUCTION Instruction, DWORD mux)
{
    if (mux < VC4_QPU_ALU_REG_A)
    {
        this->xprintf("%s", VC4_QPU_Name_Alu[mux]);
        if (mux == VC4_QPU_ALU_R4 && VC4_QPU_IS_PM_SET(Instruction))
        {
            this->xprintf("%s", VC4_QPU_Name_Unpack[VC4_QPU_GET_UNPACK(Instruction)]);
        }
    }
    else if (mux == VC4_QPU_ALU_REG_A)
    {
        ParseReadAddr(VC4_QPU_GET_RADDR_A(Instruction), true);
        if (!VC4_QPU_IS_PM_SET(Instruction))
        {
            this->xprintf("%s", VC4_QPU_Name_Unpack[VC4_QPU_GET_UNPACK(Instruction)]);
        }
    } 
    else if (mux == VC4_QPU_ALU_REG_B)
    {
        if (VC4_QPU_IS_OPCODE_LOAD_SM(Instruction))
        {
            ParseSmallImmediate(VC4_QPU_GET_SMALL_IMMEDIATE(Instruction));
        }
        else
        {
            ParseReadAddr(VC4_QPU_GET_RADDR_B(Instruction), false);
        }
    }
    else
    {
        this->xprintf("Invalid");
    }
    return S_OK;
}

HRESULT Vc4Disasm::ParseAddOp(VC4_QPU_INSTRUCTION Instruction)
{
    this->xprintf("%s%s%s ",
        (VC4_QPU_IS_OPCODE_ADD_MOV(Instruction) ? VC4_QPU_Name_Op_Move : VC4_QPU_Name_Op_Add[VC4_QPU_GET_OPCODE_ADD(Instruction)]),
        (VC4_QPU_IS_SETFLAGS_SET(Instruction) ? VC4_QPU_Name_SetFlag : VC_QPU_Name_Empty),
        (VC4_QPU_IS_OPCODE_ADD_NOP(Instruction) ? VC_QPU_Name_Empty : VC4_QPU_Name_Cond[VC4_QPU_GET_COND_ADD(Instruction)]));
    if (!VC4_QPU_IS_OPCODE_ADD_NOP(Instruction))
    {
        ParseWrite(Instruction, true);
        this->xprintf(", ");
        ParseRead(Instruction, VC4_QPU_GET_ADD_A(Instruction));
        if (!VC4_QPU_IS_OPCODE_ADD_MOV(Instruction))
        {
            this->xprintf(", ");
            ParseRead(Instruction, VC4_QPU_GET_ADD_B(Instruction));
        }
    }
    return S_OK;
}

HRESULT Vc4Disasm::ParseMulOp(VC4_QPU_INSTRUCTION Instruction)
{
    this->xprintf("%s%s%s ",
        (VC4_QPU_IS_OPCODE_MUL_MOV(Instruction) ? VC4_QPU_Name_Op_Move : VC4_QPU_Name_Op_Mul[VC4_QPU_GET_OPCODE_MUL(Instruction)]),
        (VC4_QPU_IS_SETFLAGS_SET(Instruction) ? VC4_QPU_Name_SetFlag : VC_QPU_Name_Empty),
        (VC4_QPU_IS_OPCODE_MUL_NOP(Instruction) ? VC_QPU_Name_Empty : VC4_QPU_Name_Cond[VC4_QPU_GET_COND_MUL(Instruction)]));
    if (!VC4_QPU_IS_OPCODE_MUL_NOP(Instruction))
    {
        ParseWrite(Instruction, false);
        this->xprintf(", ");
        ParseRead(Instruction, VC4_QPU_GET_MUL_A(Instruction));
        if (!VC4_QPU_IS_OPCODE_MUL_MOV(Instruction))
        {
            this->xprintf(", ");
            ParseRead(Instruction, VC4_QPU_GET_MUL_B(Instruction));
        }
    }
    return S_OK;
}

HRESULT Vc4Disasm::ParseALUInstruction(VC4_QPU_INSTRUCTION Instruction)
{
    ParseAddOp(Instruction);
    this->xprintf(" ; ");
    ParseMulOp(Instruction);
    return S_OK;
}

HRESULT Vc4Disasm::ParseLoadImmInstruction(VC4_QPU_INSTRUCTION Instruction)
{
    this->xprintf("%s%s%s ",
        VC4_QPU_Name_Op_Move,
        ((VC4_QPU_GET_IMMEDIATE_TYPE(Instruction) == VC4_QPU_IMMEDIATE_TYPE_32) ? "" : (VC4_QPU_GET_IMMEDIATE_TYPE(Instruction) == VC4_QPU_IMMEDIATE_TYPE_PER_ELEMENT_SIGNED) ? "_per_element_signed" : (VC4_QPU_GET_IMMEDIATE_TYPE(Instruction) == VC4_QPU_IMMEDIATE_TYPE_PER_ELEMENT_UNSIGNED) ? "per_element_unsigned" : "Invalid"),
        (VC4_QPU_IS_SETFLAGS_SET(Instruction) ? VC4_QPU_Name_SetFlag : VC_QPU_Name_Empty));
    if (VC4_QPU_GET_WADDR_ADD(Instruction) != VC4_QPU_WADDR_NOP)
    {
        ParseWrite(Instruction, true);
        this->xprintf("%s", VC4_QPU_Name_Cond[VC4_QPU_GET_COND_ADD(Instruction)]);
        this->xprintf(", ");
    }
    if (VC4_QPU_GET_WADDR_MUL(Instruction) != VC4_QPU_WADDR_NOP)
    {
        ParseWrite(Instruction, false);
        this->xprintf("%s", VC4_QPU_Name_Cond[VC4_QPU_GET_COND_MUL(Instruction)]);
        this->xprintf(", ");
    }
    this->xprintf("%x", VC4_QPU_GET_IMMEDIATE_32(Instruction));
    return S_OK;
}

HRESULT Vc4Disasm::ParseSemaphoreInstruction(VC4_QPU_INSTRUCTION Instruction)
{
    this->xprintf("Semaphore instruction is not supported yet");
    return S_OK;
    Instruction;
}
        
HRESULT Vc4Disasm::ParseBranchInstruction(VC4_QPU_INSTRUCTION Instruction)
{
    this->xprintf("%s%s ",
        (VC4_QPU_IS_BRANCH_RELATIVE(Instruction) ? "brr" : "bra"),
        (VC4_QPU_Name_Branch_Cond[VC4_QPU_GET_BRANCH_COND(Instruction)]));
    if (VC4_QPU_IS_BRANCH_USE_RADDR_A(Instruction))
    {
        ParseReadAddr(VC4_QPU_GET_BRANCH_RADDR_A(Instruction), true);
    }
    if (VC4_QPU_GET_IMMEDIATE_32(Instruction) != 0)
    {
        this->xprintf("+0x%x", VC4_QPU_GET_IMMEDIATE_32(Instruction));
    }
    if ((VC4_QPU_GET_WADDR_ADD(Instruction) != VC4_QPU_WADDR_NOP) ||
        (VC4_QPU_GET_WADDR_MUL(Instruction) != VC4_QPU_WADDR_NOP))
    {
        this->xprintf("; ret_addr saved to ");
        ParseWrite(Instruction, true, false);
        this->xprintf(" ");
        ParseWrite(Instruction, false, false);
    }
    return S_OK;
}

HRESULT
Vc4Disasm::Run(const VC4_QPU_INSTRUCTION* pShader, ULONG ShaderSize)
{
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
            if (VC4_QPU_IS_SEMAPHORE(Instruction))
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
            this->xprintf("Invalid signature");
            break;
        }
        Flush(0);
    }
	return S_OK;
}

#endif