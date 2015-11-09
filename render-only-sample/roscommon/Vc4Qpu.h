#pragma once

//
// Video Core IV - QPU instruction set define
//

typedef unsigned __int64 VC4_QPU_INSTRUCTION; // every QPU instruction is 64bits.

#define DEFINE_VC4_QPU_GET(Inst,tag)       (((Inst) & VC4_QPU_##tag##_MASK) >> VC4_QPU_##tag##_SHIFT)
#define DEFINE_VC4_QPU_SET(Inst,Value,tag) ((Inst) = ((Inst) & ~VC4_QPU_##tag##_MASK) | (((Value) << VC4_QPU_##tag##_SHIFT) & VC4_QPU_##tag##_MASK))

//
// Siginaling Bits - [63]-[60]
//
#define VC4_QPU_SIG_SHIFT 60
#define VC4_QPU_SIG_MASK (0xfULL << VC4_QPU_SIG_SHIFT)
#define VC4_QPU_GET_SIG(Inst) DEFINE_VC4_QPU_GET(Inst,SIG)
#define VC4_QPU_SET_SIG(Inst,Value) DEFINE_VC4_QPU_SET(Inst,Value,SIG)

#define VC4_QPU_SIG_BREAK 0
#define VC4_QPU_SIG_NO_SIGNAL 1
#define VC4_QPU_SIG_THREAD_SWITCH 2
#define VC4_QPU_SIG_PROGRAM_END 3
#define VC4_QPU_SIG_WAIT_FOR_SCOREBOARD 4
#define VC4_QPU_SIG_SCOREBOARD_UNBLOCK 5
#define VC4_QPU_SIG_LAST_THREAD_SWITCH 6
#define VC4_QPU_SIG_COVERAGE_LOAD 7
#define VC4_QPU_SIG_COLOR_LOAD 8
#define VC4_QPU_SIG_COLOR_LOAD_AND_PROGRAM_END 9
#define VC4_QPU_SIG_LOAD_TMU0 10
#define VC4_QPU_SIG_LOAD_TMU1 11
#define VC4_QPU_SIG_ALPAH_MASK_LOAD 12
#define VC4_QPU_SIG_ALU_WITH_RADDR_B 13
#define VC4_QPU_SIG_LOAD_IMMEDIATE 14
#define VC4_QPU_SIG_BRANCH 15
#define VC4_QPU_SIG_ARRAY_SIZE 16 // just for array

//
// ALU instructions 
//
// With Signaling Bit value between 0 and 12
//

//
// Unpack Bits - [59]-[57]
//
#define VC4_QPU_UNPACK_SHIFT 57
#define VC4_QPU_UNPACK_MASK (0x7ULL << VC4_QPU_UNPACK_SHIFT)
#define VC4_QPU_GET_UNPACK(Inst) DEFINE_VC4_QPU_GET(Inst,UNPACK)
#define VC4_QPU_SET_UNPACK(Inst,Value) DEFINE_VC4_QPU_SET(Inst,Value,UNPACK)

// Regfile-a unpack operations (pm bit = 0):
// R4 unpack operations (pm bit = 1):
#define VC4_QPU_UNPACK_32 0 // NOP
#define VC4_QPU_UNPACK_16a 1 // Float16 -> Float32 or Signed 16 -> Signed 32
#define VC4_QPU_UNPACK_16b 2 // Float16 -> Float32 or Signed 16 -> Signed 32
#define VC4_QPU_UNPACK_8d_REP 3 // replicate alpha {8d,8d,8d,8d}
#define VC4_QPU_UNPACK_8a 4 // float or unsigned depending on opcode.
#define VC4_QPU_UNPACK_8b 5 // float or unsigned depending on opcode.
#define VC4_QPU_UNPACK_8c 6 // float or unsigned depending on opcode.
#define VC4_QPU_UNPACK_8d 7 // float or unsigned depending on opcode.
#define VC4_QPU_UNPACK_ARRAY_SIZE 8 // just for array

//
// PM Bit for unpack/pack - [56]
//
#define VC4_QPU_PM_SHIFT 56
#define VC4_QPU_PM_MASK (0x1ULL << VC4_QPU_PM_SHIFT)
#define VC4_QPU_IS_PM_SET(Inst) (DEFINE_VC4_QPU_GET(Inst,PM) ? true : false)
#define VC4_QPU_SET_PM(Inst,Value) DEFINE_VC4_QPU_SET(Inst,(Value ? 1 : 0),PM)

//
// Pack Bits - [55]-[52]
// 
#define VC4_QPU_PACK_SHIFT 52
#define VC4_QPU_PACK_MASK (0xfULL << VC4_QPU_PACK_SHIFT)
#define VC4_QPU_GET_PACK(Inst) DEFINE_VC4_QPU_GET(Inst,PACK)
#define VC4_QPU_SET_PACK(Inst,Value) DEFINE_VC4_QPU_SET(Inst,Value,PACK)

// Regfile-a pack operations (pm bit = 0):
#define VC4_QPU_PACK_A_32 0 // NOP
#define VC4_QPU_PACK_A_16a 1
#define VC4_QPU_PACK_A_16b 2
#define VC4_QPU_PACK_A_8888 3
#define VC4_QPU_PACK_A_8a 4
#define VC4_QPU_PACK_A_8b 5
#define VC4_QPU_PACK_A_8c 6
#define VC4_QPU_PACK_A_8d 7
#define VC4_QPU_PACK_A_32_SAT 8
#define VC4_QPU_PACK_A_16a_SAT 9
#define VC4_QPU_PACK_A_16b_SAT 10
#define VC4_QPU_PACK_A_8888_SAT 11
#define VC4_QPU_PACK_A_8a_SAT 12
#define VC4_QPU_PACK_A_8b_SAT 13
#define VC4_QPU_PACK_A_8c_SAT 14
#define VC4_QPU_PACK_A_8d_SAT 15
#define VC4_QPU_PACK_A_ARRAY_SIZE 16 // just for array

// MUL ALU pack operations (pm bit = 1):
#define VC4_QPU_PACK_MUL_32 0
// reserved 1-2
#define VC4_QPU_PACK_MUL_8888 3
#define VC4_QPU_PACK_MUL_8a 4
#define VC4_QPU_PACK_MUL_8b 5
#define VC4_QPU_PACK_MUL_8c 6
#define VC4_QPU_PACK_MUL_8d 7
#define VC4_QPU_PACK_MUL_ARRAY_SIZE 8 // just for array

//
// Condition Bits - [51]-[49] for add, [48]-[46] for mul.
//
#define VC4_QPU_COND_ADD_SHIFT 49
#define VC4_QPU_COND_ADD_MASK (0x7ULL << VC4_QPU_COND_ADD_SHIFT)
#define VC4_QPU_GET_COND_ADD(Inst) DEFINE_VC4_QPU_GET(Inst,COND_ADD)
#define VC4_QPU_SET_COND_ADD(Inst,Value) DEFINE_VC4_QPU_SET(Inst,Value,COND_ADD)

#define VC4_QPU_COND_MUL_SHIFT 46
#define VC4_QPU_COND_MUL_MASK (0x7ULL << VC4_QPU_COND_MUL_SHIFT)
#define VC4_QPU_GET_COND_MUL(Inst) DEFINE_VC4_QPU_GET(Inst,COND_MUL)
#define VC4_QPU_SET_COND_MUL(Inst,Value) DEFINE_VC4_QPU_SET(Inst,Value,COND_MUL)

#define VC4_QPU_COND_NEVER 0
#define VC4_QPU_COND_ALWAYS 1
#define VC4_QPU_COND_ZS 2 // Z set
#define VC4_QPU_COND_ZC 3 // Z clear
#define VC4_QPU_COND_NS 4 // N set
#define VC4_QPU_COND_NC 5 // N clear
#define VC4_QPU_COND_CS 6 // C set
#define VC4_QPU_COND_CC 7 // C clear
#define VC4_QPU_COND_ARRAY_SIZE 8 // just for array

//
// Setflags Bit - [45]
//
#define VC4_QPU_SETFLAGS_SHIFT 45
#define VC4_QPU_SETFLAGS_MASK (0x1ULL << VC4_QPU_SETFLAGS_SHIFT)
#define VC4_QPU_IS_SETFLAGS_SET(Inst) (DEFINE_VC4_QPU_GET(Inst,SETFLAGS) ? true : false)
#define VC4_QPU_SET_SETFLAGS(Inst,Value) DEFINE_VC4_QPU_SET(Inst,(Value ? 1 : 0),SETFLAGS)

//
// Write Swap Bit - [44]
//
#define VC4_QPU_WRITESWAP_SHIFT 44
#define VC4_QPU_WRITESWAP_MASK (0x1ULL << VC4_QPU_WRITESWAP_SHIFT)
#define VC4_QPU_IS_WRITESWAP_SET(Inst) (DEFINE_VC4_QPU_GET(Inst,WRITESWAP) ? true : false)
#define VC4_QPU_SET_WRITESWAP(Inst,Value) DEFINE_VC4_QPU_SET(Inst,(Value ? 1 : 0),WRITESWAP)

//
// Write Address for output - [43]-[38] for add
//                            [37]-[32] for mul
//
#define VC4_QPU_WADDR_ADD_SHIFT 38
#define VC4_QPU_WADDR_ADD_MASK (0x3fULL << VC4_QPU_WADDR_ADD_SHIFT)
#define VC4_QPU_GET_WADDR_ADD(Inst) DEFINE_VC4_QPU_GET(Inst,WADDR_ADD)
#define VC4_QPU_SET_WADDR_ADD(Inst,Value) DEFINE_VC4_QPU_SET(Inst,Value,WADDR_ADD)

#define VC4_QPU_WADDR_MUL_SHIFT 32
#define VC4_QPU_WADDR_MUL_MASK (0x3fULL << VC4_QPU_WADDR_MUL_SHIFT)
#define VC4_QPU_GET_WADDR_MUL(Inst) DEFINE_VC4_QPU_GET(Inst,WADDR_MUL)
#define VC4_QPU_SET_WADDR_MUL(Inst,Value) DEFINE_VC4_QPU_SET(Inst,Value,WADDR_MUL)

//
// QPU register address map for write
//
// Address 0 to 31 is for regfile A/B.
#define VC4_QPU_WADDR_ACC0 32
#define VC4_QPU_WADDR_ACC1 33
#define VC4_QPU_WADDR_ACC2 34
#define VC4_QPU_WADDR_ACC3 35
#define VC4_QPU_WADDR_TMU_NOSWAP 36
#define VC4_QPU_WADDR_ACC5 37 // A: replicate pixel 0 per quad, B: replicate SIMD element 0
#define VC4_QPU_WADDR_HOSTINT 38
#define VC4_QPU_WADDR_NOP 39
#define VC4_QPU_WADDR_UNIFORM 40
#define VC4_QPU_WADDR_QUAD_X 41 // regfile A
#define VC4_QPU_WADDR_QUAD_Y 41 // regfile B
#define VC4_QPU_WADDR_MS_FLAGS 42 // regfile A
#define VC4_QPU_WADDR_REV_FLAG 42 // regfile B
#define VC4_QPU_WADDR_TLB_STENCIL_SETUP 43
#define VC4_QPU_WADDR_TLB_Z 44
#define VC4_QPU_WADDR_TLB_COLOUR_MS 45
#define VC4_QPU_WADDR_TLB_COLOUR_ALL 46
#define VC4_QPU_WADDR_TLB_ALPHA_MASK 47
#define VC4_QPU_WADDR_VPM 48
#define VC4_QPU_WADDR_VPMVCD_RD_SETUP 49 // regfile A
#define VC4_QPU_WADDR_VPMVCD_WR_SETUP 49 // regfile B
#define VC4_QPU_WADDR_VPM_LD_ADDR 50 // regfile A
#define VC4_QPU_WADDR_VPM_ST_ADDR 50 // regfile B
#define VC4_QPU_WADDR_MUTEX_RELEASE 51
#define VC4_QPU_WADDR_SFU_RECIP 52
#define VC4_QPU_WADDR_SFU_RECIPSQRT 53
#define VC4_QPU_WADDR_SFU_EXP 54
#define VC4_QPU_WADDR_SFU_LOG 55
#define VC4_QPU_WADDR_TMU0_S 56 // X - retiring
#define VC4_QPU_WADDR_TMU0_T 57 // Y
#define VC4_QPU_WADDR_TMU0_R 58 // Z
#define VC4_QPU_WADDR_TMU0_B 59 // LOD Bias
#define VC4_QPU_WADDR_TMU1_S 60 // X - retiring
#define VC4_QPU_WADDR_TMU1_T 61 // Y
#define VC4_QPU_WADDR_TMU1_R 62 // Z
#define VC4_QPU_WADDR_TMU1_B 63 // LOD Bias
#define VC4_QPU_WADDR_ARRAY_SIZE 64 // just for array

//
// Op code for mul - [31]-[29]
//
#define VC4_QPU_OPCODE_MUL_SHIFT 29
#define VC4_QPU_OPCODE_MUL_MASK (0x7ULL << VC4_QPU_OPCODE_MUL_SHIFT)
#define VC4_QPU_GET_OPCODE_MUL(Inst) DEFINE_VC4_QPU_GET(Inst,OPCODE_MUL)
#define VC4_QPU_SET_OPCODE_MUL(Inst,Value) DEFINE_VC4_QPU_SET(Inst,Value,OPCODE_MUL)

#define VC4_QPU_OPCODE_MUL_NOP 0
#define VC4_QPU_OPCODE_MUL_FMUL 1
#define VC4_QPU_OPCODE_MUL_MUL24 2
#define VC4_QPU_OPCODE_MUL_V8MULD 3
#define VC4_QPU_OPCODE_MUL_V8MIN 4
#define VC4_QPU_OPCODE_MUL_V8MAX 5
#define VC4_QPU_OPCODE_MUL_V8ADDS 6
#define VC4_QPU_OPCODE_MUL_V8SUBS 7
#define VC4_QPU_OPCODE_MUL_ARRAY_SIZE 8 // just for array

//
// Op code for add - [28]-[24]
//
#define VC4_QPU_OPCODE_ADD_SHIFT 24
#define VC4_QPU_OPCODE_ADD_MASK (0x1fULL << VC4_QPU_OPCODE_ADD_SHIFT)
#define VC4_QPU_GET_OPCODE_ADD(Inst) DEFINE_VC4_QPU_GET(Inst,OPCODE_ADD)
#define VC4_QPU_SET_OPCODE_ADD(Inst,Value) DEFINE_VC4_QPU_SET(Inst,Value,OPCODE_ADD)

#define VC4_QPU_OPCODE_ADD_NOP 0
#define VC4_QPU_OPCODE_ADD_FADD 1
#define VC4_QPU_OPCODE_ADD_FSUB 2
#define VC4_QPU_OPCODE_ADD_FMIN 3
#define VC4_QPU_OPCODE_ADD_FMAX 4
#define VC4_QPU_OPCODE_ADD_FMIN_ABS 5
#define VC4_QPU_OPCODE_ADD_FMAX_ABS 6
#define VC4_QPU_OPCODE_ADD_FTOI 7
#define VC4_QPU_OPCODE_ADD_ITOF 8
// 9-11 reserved
#define VC4_QPU_OPCODE_ADD_ADD 12
#define VC4_QPU_OPCODE_ADD_SUB 13
#define VC4_QPU_OPCODE_ADD_SHR 14
#define VC4_QPU_OPCODE_ADD_ASR 15
#define VC4_QPU_OPCODE_ADD_ROR 16
#define VC4_QPU_OPCODE_ADD_SHL 17
#define VC4_QPU_OPCODE_ADD_MIN 18
#define VC4_QPU_OPCODE_ADD_MAX 19
#define VC4_QPU_OPCODE_ADD_AND 20
#define VC4_QPU_OPCODE_ADD_OR 21
#define VC4_QPU_OPCODE_ADD_XOR 22
#define VC4_QPU_OPCODE_ADD_NOT 23
#define VC4_QPU_OPCODE_ADD_CLZ 24 // count leading zero
// 25-29 reserved
#define VC4_QPU_OPCODE_ADD_V8ADDS 30
#define VC4_QPU_OPCODE_ADD_V8SUBS 31
#define VC4_QPU_OPCODE_ADD_ARRAY_SIZE 32

//
// Read address for register file A [23]-[18]
//                  register file B [17]-[12]
//
#define VC4_QPU_RADDR_A_SHIFT 18
#define VC4_QPU_RADDR_A_MASK (0x3fULL << VC4_QPU_RADDR_A_SHIFT)
#define VC4_QPU_GET_RADDR_A(Inst) DEFINE_VC4_QPU_GET(Inst,RADDR_A)
#define VC4_QPU_SET_RADDR_A(Inst,Value) DEFINE_VC4_QPU_SET(Inst,Value,RADDR_A)

#define VC4_QPU_RADDR_B_SHIFT 12
#define VC4_QPU_RADDR_B_MASK (0x3fULL << VC4_QPU_RADDR_B_SHIFT)
#define VC4_QPU_GET_RADDR_B(Inst) DEFINE_VC4_QPU_GET(Inst,RADDR_B)
#define VC4_QPU_SET_RADDR_B(Inst,Value) DEFINE_VC4_QPU_SET(Inst,Value,RADDR_B)

//
// QPU register address map for read.
//
// Address 0 to 31 is for regfile A/B.
#define VC4_QPU_RADDR_UNIFORM 32
#define VC4_QPU_RADDR_VERYING 35
#define VC4_QPU_RADDR_ELEMENT_NUMBER 38 // regfile A
#define VC4_QPU_RADDR_QPU_NUMBER     38 // regfile B
#define VC4_QPU_RADDR_NOP 39
#define VC4_QPU_RADDR_PIXEL_COORD_X 41 // regfile A
#define VC4_QPU_RADDR_PIXEL_COORD_Y 41 // regfile B
#define VC4_QPU_RADDR_MS_FLAGS 42 // regfile A
#define VC4_QPU_RADDR_REV_FLAG 42 // regfile B
#define VC4_QPU_RADDR_VPM 48
#define VC4_QPU_RADDR_VPM_LD_BUSY 49 // regfile A
#define VC4_QPU_RADDR_VPM_ST_BUSY 49 // regfile B
#define VC4_QPU_RADDR_VPM_LD_WAIT 50 // regfile A
#define VC4_QPU_RADDR_VPM_ST_WAIT 50 // regfile B
#define VC4_QPU_RADDR_MUTEX_ACQUIRE 51
#define VC4_QPU_RADDR_ARRAY_SIZE 52 // just for array

//
// add_a [11]-[9]
//
#define VC4_QPU_ADD_A_SHIFT 9
#define VC4_QPU_ADD_A_MASK (0x7ULL << VC4_QPU_ADD_A_SHIFT)
#define VC4_QPU_GET_ADD_A(Inst) DEFINE_VC4_QPU_GET(Inst,ADD_A)
#define VC4_QPU_SET_ADD_A(Inst,Value) DEFINE_VC4_QPU_SET(Inst,Value,ADD_A)

//
// add_b [8]-[6]
//
#define VC4_QPU_ADD_B_SHIFT 6
#define VC4_QPU_ADD_B_MASK (0x7ULL << VC4_QPU_ADD_B_SHIFT)
#define VC4_QPU_GET_ADD_B(Inst) DEFINE_VC4_QPU_GET(Inst,ADD_B)
#define VC4_QPU_SET_ADD_B(Inst,Value) DEFINE_VC4_QPU_SET(Inst,Value,ADD_B)

//
// mul_a [5]-[3]
//
#define VC4_QPU_MUL_A_SHIFT 3
#define VC4_QPU_MUL_A_MASK (0x7ULL << VC4_QPU_MUL_A_SHIFT)
#define VC4_QPU_GET_MUL_A(Inst) DEFINE_VC4_QPU_GET(Inst,MUL_A)
#define VC4_QPU_SET_MUL_A(Inst,Value) DEFINE_VC4_QPU_SET(Inst,Value,MUL_A)

//
// mul_b [2]-[0]
//
#define VC4_QPU_MUL_B_SHIFT 0
#define VC4_QPU_MUL_B_MASK (0x7ULL << VC4_QPU_MUL_B_SHIFT)
#define VC4_QPU_GET_MUL_B(Inst) DEFINE_VC4_QPU_GET(Inst,MUL_B)
#define VC4_QPU_SET_MUL_B(Inst,Value) DEFINE_VC4_QPU_SET(Inst,Value,MUL_B)

//
// ALU input muxes for add_a/b, mul_a/b.
//
#define VC4_QPU_ALU_R0 0
#define VC4_QPU_ALU_R1 1
#define VC4_QPU_ALU_R2 2
#define VC4_QPU_ALU_R3 3
#define VC4_QPU_ALU_R4 4
#define VC4_QPU_ALU_R5 5
#define VC4_QPU_ALU_REG_A 6
#define VC4_QPU_ALU_REG_B 7
#define VC4_QPU_ALU_ARRAY_SIZE 8 // just for array

//
// Load Small immediate instruction
//
// Only with VC4_QPU_SIG_ALU_WITH_RADDR_B
//

//
// Small immediate [17]-[12] : overlapping with register file B.
//
#define VC4_QPU_SMALL_IMMEDIATE_SHIFT 12
#define VC4_QPU_SMALL_IMMEDIATE_MASK (0x3fULL << VC4_QPU_SMALL_IMMEDIATE_SHIFT)
#define VC4_QPU_GET_SMALL_IMMEDIATE(Inst) DEFINE_VC4_QPU_GET(Inst,SMALL_IMMEDIATE)
#define VC4_QPU_SET_SMALL_IMMEDIATE(Inst,Value) DEFINE_VC4_QPU_SET(Inst,Value,SMALL_IMMEDIATE)

//
// Load immediate insturction
//
// Only with VC4_QPU_SIG_LOAD_IMMEDIATE
//

//
// Immediate type [59]-[57]
//
#define VC4_QPU_IMMEDIATE_TYPE_SHIFT 57
#define VC4_QPU_IMMEDIATE_TYPE_MASK (0x3ULL << VC4_QPU_IMMEDIATE_TYPE_SHIFT)
#define VC4_QPU_GET_IMMEDIATE_TYPE(Inst) DEFINE_VC4_QPU_GET(Inst,IMMEDIATE_TYPE)
#define VC4_QPU_SET_IMMEDIATE_TYPE(Inst,Value) DEFINE_VC4_QPU_SET(Inst,Value,IMMEDIATE_TYPE)

#define VC4_QPU_IMMEDIATE_TYPE_32 0
#define VC4_QPU_IMMEDIATE_TYPE_PER_ELEMENT_SIGNED 1
#define VC4_QPU_IMMEDIATE_TYPE_PER_ELEMENT_UNSIGNED 3
#define VC4_QPU_IMMEDIATE_TYPE_SEMAPHORE 4

//
// Immediate 32bit [31]-[0]
//
#define VC4_QPU_IMMEDIATE_32_SHIFT 0
#define VC4_QPU_IMMEDIATE_32_MASK (0xffffffffULL << VC4_QPU_IMMEDIATE_32_SHIFT)
#define VC4_QPU_GET_IMMEDIATE_32(Inst) DEFINE_VC4_QPU_GET(Inst,IMMEDIATE_32)
#define VC4_QPU_SET_IMMEDIATE_32(Inst,Value) DEFINE_VC4_QPU_SET(Inst,Value,IMMEDIATE_32)

//
// Immediate per element MS bit [31]-[16]
//
#define VC4_QPU_IMMEDIATE_MS_SHIFT 16
#define VC4_QPU_IMMEDIATE_MS_MASK (0xffffULL << VC4_QPU_IMMEDIATE_MS_SHIFT)

//
// Immediate per element LS bit [15]-[0]
//
#define VC4_QPU_IMMEDIATE_LS_SHIFT 0
#define VC4_QPU_IMMEDIATE_LS_MASK (0xffffULL << VC4_QPU_IMMEDIATE_LS_SHIFT)

//
// Only when VC4_QPU_IMMEDIATE_TYPE_SEMAPHORE
//
#define VC4_QPU_SEMAPHORE_SA_SHIFT 4
#define VC4_QPU_SEMAPHORE_SA_MASK (0x1ULL << VC4_QPU_SEMAPHORE_SA_SHIFT)

#define VC4_QPU_SEMAPHORE_SA_INC 0
#define VC4_QPU_SEMAPHORE_SA_DEC 1

#define VC4_QPU_SEMAPHORE_SHIFT 0
#define VC4_QPU_SEMAPHORE_MASK (0xfULL << VC4_QPU_SEMAPHORE_SHIFT)

//
// Branch instruction
//
// Only with VC4_QPU_SIG_BRANCH
//

//
// Branch condition [55]-[52]
//
#define VC4_QPU_BRANCH_COND_SHIFT 52
#define VC4_QPU_BRANCH_COND_MASK (0xfULL << VC4_QPU_BRANCH_COND_SHIFT)
#define VC4_QPU_GET_BRANCH_COND(Inst) DEFINE_VC4_QPU_GET(Inst,BRANCH_COND)
#define VC4_QPU_SET_BRANCH_COND(Inst,Value) DEFINE_VC4_QPU_SET(Inst,Value,BRANCH_COND)

#define VC4_QPU_BRANCH_COND_ALL_ZS 0 // All Z flags set
#define VC4_QPU_BRANCH_COND_ALL_ZC 1 // All Z flags clear
#define VC4_QPU_BRANCH_COND_ANY_ZS 2 // Any Z flags set
#define VC4_QPU_BRANCH_COND_ANY_ZC 3 // Any Z flags clear
#define VC4_QPU_BRANCH_COND_ALL_NS 4 // All N flags set
#define VC4_QPU_BRANCH_COND_ALL_NC 5 // All N flags clear
#define VC4_QPU_BRANCH_COND_ANY_NS 6 // Any N flags set
#define VC4_QPU_BRANCH_COND_ANY_NC 7 // Any N flags clear
#define VC4_QPU_BRANCH_COND_ALL_CS 8 // All C flags set
#define VC4_QPU_BRANCH_COND_ALL_CC 9 // All C flags clear
#define VC4_QPU_BRANCH_COND_ANY_CS 10 // Any C flags set
#define VC4_QPU_BRANCH_COND_ANY_CC 11 // Any C flags clear
// Reserved 12-14
#define VC4_QPU_BRANCH_COND_ALWAYS 15 // Always execute
#define VC4_QPU_BRANCH_COND_ARRAY_SIZE 16 // just for array

//
// Branch relative bit [51] : PC = PC + 4.
//
#define VC4_QPU_BRANCH_RELATIVE_SHIFT 51
#define VC4_QPU_BRANCH_RELATIVE_MASK (0x1ULL << VC4_QPU_BRANCH_RELATIVE_SHIFT)
#define VC4_QPU_IS_BRANCH_RELATIVE(Inst) (DEFINE_VC4_QPU_GET(Inst,BRANCH_RELATIVE) ? true : false)
#define VC4_QPU_SET_BRANCH_RELATIVE(Inst,Value) DEFINE_VC4_QPU_SET(Inst,(Value ? 1 : 0),BRANCH_RELATIVE)

//
// Branch register file A [50] : PC = PC + raddr_a[0].
//
#define VC4_QPU_BRANCH_USE_RADDR_A_SHIFT 50
#define VC4_QPU_BRANCH_USE_RADDR_A_MASK (0x1ULL << VC4_QPU_BRANCH_USE_RADDR_A_SHIFT)
#define VC4_QPU_IS_BRANCH_USE_RADDR_A(Inst) (DEFINE_VC4_QPU_GET(Inst,BRANCH_USE_RADDR_A) ? true : false)
#define VC4_QPU_SET_BRANCH_USE_RADDR_A(Inst,Value) DEFINE_VC4_QPU_SET(Inst,(Value ? 1 : 0),BRANCH_USE_RADDR_A)

//
// Brach raddr_a [49] - [45]
//
#define VC4_QPU_BRANCH_RADDR_A_SHIFT 45
#define VC4_QPU_BRANCH_RADDR_A_MASK (0x1fULL << VC4_QPU_BRANCH_RADDR_A_SHIFT)
#define VC4_QPU_GET_BRANCH_RADDR_A(Inst) DEFINE_VC4_QPU_GET(Inst,BRANCH_RADDR_A)
#define VC4_QPU_SET_BRANCH_RADDR_A(Inst,Value) DEFINE_VC4_QPU_SET(Inst,Value,BRANCH_RADDR_A)

//
// Others
//
#define VC4_QPU_IS_OPCODE_ADD_MOV(Inst) ((VC4_QPU_GET_OPCODE_ADD(Inst) == VC4_QPU_OPCODE_ADD_OR) && (VC4_QPU_GET_ADD_A(Inst) == VC4_QPU_GET_ADD_B(Inst)))
#define VC4_QPU_IS_OPCODE_MUL_MOV(Inst) ((VC4_QPU_GET_OPCODE_MUL(Inst) == VC4_QPU_OPCODE_MUL_V8MIN) && (VC4_QPU_GET_MUL_A(Inst) == VC4_QPU_GET_MUL_B(Inst)))

#define VC4_QPU_IS_OPCODE_ADD_NOP(Inst) (VC4_QPU_GET_OPCODE_ADD(Inst) == VC4_QPU_OPCODE_ADD_NOP)
#define VC4_QPU_IS_OPCODE_MUL_NOP(Inst) (VC4_QPU_GET_OPCODE_MUL(Inst) == VC4_QPU_OPCODE_MUL_NOP)

#define VC4_QPU_IS_OPCODE_LOAD_SM(Inst) (VC4_QPU_GET_SIG(Inst) == VC4_QPU_SIG_ALU_WITH_RADDR_B)
#define VC4_QPU_IS_OPCODE_LOAD_IM(Inst) (VC4_QPU_GET_SIG(Inst) == VC4_QPU_SIG_LOAD_IMMEDIATE)

#define VC4_QPU_IS_OPCODE_BRANCH(Inst)  (VC4_QPU_GET_SIG(Inst) == VC4_QPU_SIG_BRANCH)
#define VC4_QPU_IS_OPCODE_SEMAPHORE(Inst) ((VC4_QPU_IS_OPCODE_LOAD_IM(Inst) && VC4_QPU_GET_IMMEDIATE_TYPE(Inst) == VC4_QPU_IMMEDIATE_TYPE_SEMAPHORE))


