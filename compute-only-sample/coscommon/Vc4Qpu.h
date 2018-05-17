#pragma once
#include <stdio.h>
#include <stdint.h>
#include <tchar.h>
#include <windows.h>

//
// Video Core IV - QPU instruction set define
//

typedef unsigned __int64 VC4_QPU_INSTRUCTION; // every QPU instruction is 64bits.

#define DEFINE_VC4_QPU_GET(Inst,tag)       (((Inst) & VC4_QPU_##tag##_MASK) >> VC4_QPU_##tag##_SHIFT)
#define DEFINE_VC4_QPU_SET(Inst,Value,tag) ((Inst) = ((Inst) & ~VC4_QPU_##tag##_MASK) | ((((VC4_QPU_INSTRUCTION)Value) << VC4_QPU_##tag##_SHIFT) & VC4_QPU_##tag##_MASK))

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

// MUL ALU pack operations (pm bit = 1):
#define VC4_QPU_PACK_MUL_32 0
// reserved 1-2
#define VC4_QPU_PACK_MUL_8888 3
#define VC4_QPU_PACK_MUL_8a 4
#define VC4_QPU_PACK_MUL_8b 5
#define VC4_QPU_PACK_MUL_8c 6
#define VC4_QPU_PACK_MUL_8d 7

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
#define VC4_QPU_WADDR_UNIFORM_ADDRESS 40
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
#define VC4_QPU_OPCODE_MUL_MOV 0x100 // fake instruction

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
#define VC4_QPU_OPCODE_ADD_MOV 0x100 // fake instruction

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
// VR_SETUP/VR_SETUP
//
#define VC4_QPU_8BIT_VECTOR  0
#define VC4_QPU_16BIT_VECTOR 1
#define VC4_QPU_32BIT_VECTOR 2

#define MAKE_VR_SETUP(NUM, STRIDE, HORIZ, LANED, VSIZE, ADDR) ((((NUM) & 0xf) << 20) | ((STRIDE & 0x1f) << 12) | ((HORIZ ? 1 : 0) << 11) | ((LANED ? 1 : 0) << 10) | ((VSIZE & 0x3) << 8) | (ADDR & 0xff))
#define MAKE_VW_SETUP(STRIDE, HORIZ, LANED, VSIZE, ADDR)                              (((STRIDE & 0x1f) << 12) | ((HORIZ ? 1 : 0) << 11) | ((LANED ? 1 : 0) << 10) | ((VSIZE & 0x3) << 8) | (ADDR & 0xff))

//
// Others
//
#define VC4_QPU_IS_OPCODE_ADD_MOV(Inst) ((VC4_QPU_GET_OPCODE_ADD(Inst) == VC4_QPU_OPCODE_ADD_OR) && (VC4_QPU_GET_ADD_A(Inst) == VC4_QPU_GET_ADD_B(Inst)))
#define VC4_QPU_IS_OPCODE_MUL_MOV(Inst) ((VC4_QPU_GET_OPCODE_MUL(Inst) == VC4_QPU_OPCODE_MUL_V8MIN) && (VC4_QPU_GET_MUL_A(Inst) == VC4_QPU_GET_MUL_B(Inst)))

#define VC4_QPU_IS_OPCODE_ADD_NOP(Inst) (VC4_QPU_GET_OPCODE_ADD(Inst) == VC4_QPU_OPCODE_ADD_NOP)
#define VC4_QPU_IS_OPCODE_MUL_NOP(Inst) (VC4_QPU_GET_OPCODE_MUL(Inst) == VC4_QPU_OPCODE_MUL_NOP)
#define VC4_QPU_IS_OPCODE_NOP(Inst)     (VC4_QPU_IS_OPCODE_ADD_NOP(Inst) && VC4_QPU_IS_OPCODE_MUL_NOP(Inst))

#define VC4_QPU_IS_OPCODE_LOAD_SM(Inst) (VC4_QPU_GET_SIG(Inst) == VC4_QPU_SIG_ALU_WITH_RADDR_B)
#define VC4_QPU_IS_OPCODE_LOAD_IM(Inst) (VC4_QPU_GET_SIG(Inst) == VC4_QPU_SIG_LOAD_IMMEDIATE)

#define VC4_QPU_IS_OPCODE_BRANCH(Inst)  (VC4_QPU_GET_SIG(Inst) == VC4_QPU_SIG_BRANCH)
#define VC4_QPU_IS_OPCODE_SEMAPHORE(Inst) ((VC4_QPU_IS_OPCODE_LOAD_IM(Inst) && VC4_QPU_GET_IMMEDIATE_TYPE(Inst) == VC4_QPU_IMMEDIATE_TYPE_SEMAPHORE))

//
// Helper for Assembler/Disassembler
//
_declspec(selectany) TCHAR* VC4_QPU_Name_Op_Move = TEXT("mov");
_declspec(selectany) TCHAR* VC4_QPU_Name_SetFlag = TEXT(".setFlags");
_declspec(selectany) TCHAR* VC4_QPU_Name_Empty = TEXT("");

typedef struct _VC4QPU_TOKENLOOKUP_TABLE
{
    INT Value;
    TCHAR *Token;
} VC4QPU_TOKENLOOKUP_TABLE;

typedef struct _VC4QPU_TOKENLOOKUP_ADDR_TABLE
{
    boolean Exchangeable;
    struct
    {
        INT Value;
        TCHAR *Token;
    } LookUp[2];
} VC4QPU_TOKENLOOKUP_ADDR_TABLE;

#define VC4_QPU_END_OF_LOOKUPTABLE (-1)
#define VC4_QPU_INVALID_VALUE      (-1)

_declspec(selectany) VC4QPU_TOKENLOOKUP_TABLE VC4_QPU_SIG_LOOKUP[] =
{
    { VC4_QPU_SIG_BREAK, _TEXT("bkpt") },
    { VC4_QPU_SIG_NO_SIGNAL, _TEXT("") },
    { VC4_QPU_SIG_NO_SIGNAL, _TEXT("nop") },
    { VC4_QPU_SIG_THREAD_SWITCH, _TEXT("thrsw") },
    { VC4_QPU_SIG_PROGRAM_END, _TEXT("thrend") },
    { VC4_QPU_SIG_WAIT_FOR_SCOREBOARD, _TEXT("sbwait") },
    { VC4_QPU_SIG_SCOREBOARD_UNBLOCK, _TEXT("sbdone") },
    { VC4_QPU_SIG_LAST_THREAD_SWITCH, _TEXT("lthrsw") },
    { VC4_QPU_SIG_COVERAGE_LOAD, _TEXT("loadcv") },
    { VC4_QPU_SIG_COLOR_LOAD, _TEXT("loadc") },
    { VC4_QPU_SIG_COLOR_LOAD_AND_PROGRAM_END, _TEXT("ldcend") },
    { VC4_QPU_SIG_LOAD_TMU0, _TEXT("ldtmu0") },
    { VC4_QPU_SIG_LOAD_TMU1, _TEXT("ldtmu1") },
    { VC4_QPU_SIG_ALPAH_MASK_LOAD, _TEXT("loadam") },
    { VC4_QPU_SIG_ALPAH_MASK_LOAD, _TEXT("lda") },
    { VC4_QPU_SIG_ALU_WITH_RADDR_B, _TEXT("loadsm") },
    { VC4_QPU_SIG_ALU_WITH_RADDR_B, _TEXT("lds") },
    { VC4_QPU_SIG_LOAD_IMMEDIATE, _TEXT("loadim") },
    { VC4_QPU_SIG_LOAD_IMMEDIATE, _TEXT("ldi") },
    { VC4_QPU_SIG_BRANCH, _TEXT("branch") },
    { VC4_QPU_END_OF_LOOKUPTABLE, NULL }
};

_declspec(selectany) VC4QPU_TOKENLOOKUP_TABLE VC4_QPU_UNPACK_LOOKUP[] =
{
    { VC4_QPU_UNPACK_32, _TEXT("") },
    { VC4_QPU_UNPACK_16a, _TEXT(".16a") },
    { VC4_QPU_UNPACK_16b, _TEXT(".16b") },
    { VC4_QPU_UNPACK_8d_REP, _TEXT(".8d_replicate") },
    { VC4_QPU_UNPACK_8a, _TEXT(".8a") },
    { VC4_QPU_UNPACK_8b, _TEXT(".8b") },
    { VC4_QPU_UNPACK_8c, _TEXT(".8c") },
    { VC4_QPU_UNPACK_8d, _TEXT(".8d") },
    { VC4_QPU_END_OF_LOOKUPTABLE, NULL }
};

_declspec(selectany) VC4QPU_TOKENLOOKUP_TABLE VC4_QPU_PACK_A_LOOKUP[] =
{
    { VC4_QPU_PACK_A_32, _TEXT("") },
    { VC4_QPU_PACK_A_16a, _TEXT(".16a") },
    { VC4_QPU_PACK_A_16b, _TEXT(".16b") },
    { VC4_QPU_PACK_A_8888, _TEXT(".8888") },
    { VC4_QPU_PACK_A_8a, _TEXT(".8a") },
    { VC4_QPU_PACK_A_8b, _TEXT(".8b") },
    { VC4_QPU_PACK_A_8c, _TEXT(".8c") },
    { VC4_QPU_PACK_A_8d, _TEXT(".8d") },
    { VC4_QPU_PACK_A_32_SAT, _TEXT(".32_saturate") },
    { VC4_QPU_PACK_A_16a_SAT, _TEXT(".16a_saturate") },
    { VC4_QPU_PACK_A_16b_SAT, _TEXT(".16b_saturate") },
    { VC4_QPU_PACK_A_8888_SAT, _TEXT(".8888_saturate") },
    { VC4_QPU_PACK_A_8a_SAT, _TEXT(".8a_saturate") },
    { VC4_QPU_PACK_A_8b_SAT, _TEXT(".8b_saturate") },
    { VC4_QPU_PACK_A_8c_SAT, _TEXT(".8c_saturate") },
    { VC4_QPU_PACK_A_8d_SAT, _TEXT(".8d_saturate") },
    { VC4_QPU_END_OF_LOOKUPTABLE, NULL }
};

_declspec(selectany) VC4QPU_TOKENLOOKUP_TABLE VC4_QPU_PACK_MUL_LOOKUP[] =
{
    { VC4_QPU_PACK_MUL_32, _TEXT("") },
    { VC4_QPU_PACK_MUL_8888, _TEXT(".8888") },
    { VC4_QPU_PACK_MUL_8a, _TEXT(".8a") },
    { VC4_QPU_PACK_MUL_8b, _TEXT(".8b") },
    { VC4_QPU_PACK_MUL_8c, _TEXT(".8c") },
    { VC4_QPU_PACK_MUL_8d, _TEXT(".8d") },
    { VC4_QPU_END_OF_LOOKUPTABLE, NULL }
};

_declspec(selectany) VC4QPU_TOKENLOOKUP_TABLE VC4_QPU_COND_LOOKUP[] =
{
    { VC4_QPU_COND_NEVER, _TEXT(".never") },
    { VC4_QPU_COND_ALWAYS, _TEXT("") },
    { VC4_QPU_COND_ZS, _TEXT(".if_zs") },
    { VC4_QPU_COND_ZC, _TEXT(".if_zc") },
    { VC4_QPU_COND_NS, _TEXT(".if_ns") },
    { VC4_QPU_COND_NC, _TEXT(".if_nc") },
    { VC4_QPU_COND_CS, _TEXT(".if_cs") },
    { VC4_QPU_COND_CC, _TEXT(".if_cc") },
    { VC4_QPU_END_OF_LOOKUPTABLE, NULL }
};

_declspec(selectany) VC4QPU_TOKENLOOKUP_ADDR_TABLE VC4_QPU_WADDR_LOOKUP[] =
{
    { false, 0, _TEXT("ra0"), 
             0, _TEXT("rb0") },
    { false, 1, _TEXT("ra1"), 
             1, _TEXT("rb1") },
    { false, 2, _TEXT("ra2"), 
             2, _TEXT("rb2") },
    { false, 3, _TEXT("ra3"), 
             3, _TEXT("rb3") },
    { false, 4, _TEXT("ra4"), 
             4, _TEXT("rb4") },
    { false, 5, _TEXT("ra5"), 
             5, _TEXT("rb5") },
    { false, 6, _TEXT("ra6"), 
             6, _TEXT("rb6") },
    { false, 7, _TEXT("ra7"), 
             7, _TEXT("rb7") },
    { false, 8, _TEXT("ra8"), 
             8, _TEXT("rb8") },
    { false, 9, _TEXT("ra9"), 
             9, _TEXT("rb9") },
    { false, 10, _TEXT("ra10"), 
             10, _TEXT("rb10") },
    { false, 11, _TEXT("ra11"), 
             11, _TEXT("rb11") },
    { false, 12, _TEXT("ra12"), 
             12, _TEXT("rb12") },
    { false, 13, _TEXT("ra13"), 
             13, _TEXT("rb13") },
    { false, 14, _TEXT("ra14"), 
             14, _TEXT("rb14") },
    { false, 15, _TEXT("ra15"), 
             15, _TEXT("rb15") },
    { false, 16, _TEXT("ra16"),
             16, _TEXT("rb16") },
    { false, 17, _TEXT("ra17"),
             17, _TEXT("rb17") },
    { false, 18, _TEXT("ra18"),
             18, _TEXT("rb18") },
    { false, 19, _TEXT("ra19"),
             19, _TEXT("rb19") },
    { false, 20, _TEXT("ra20"),
             20, _TEXT("rb20") },
    { false, 21, _TEXT("ra21"),
             21, _TEXT("rb21") },
    { false, 22, _TEXT("ra22"),
             22, _TEXT("rb22") },
    { false, 23, _TEXT("ra23"),
             23, _TEXT("rb23") },
    { false, 24, _TEXT("ra24"),
             24, _TEXT("rb24") },
    { false, 25, _TEXT("ra25"),
             25, _TEXT("rb25") },
    { false, 26, _TEXT("ra26"),
             26, _TEXT("rb26") },
    { false, 27, _TEXT("ra27"),
             27, _TEXT("rb27") },
    { false, 28, _TEXT("ra28"),
             28, _TEXT("rb28") },
    { false, 29, _TEXT("ra29"),
             29, _TEXT("rb29") },
    { false, 30, _TEXT("ra30"),
             30, _TEXT("rb30") },
    { false, 31, _TEXT("ra31"),
             31, _TEXT("rb31") },
    { true,  VC4_QPU_WADDR_ACC0, _TEXT("r0"),
             VC4_QPU_WADDR_ACC0, _TEXT("r0") },
    { true,  VC4_QPU_WADDR_ACC1, _TEXT("r1"),
             VC4_QPU_WADDR_ACC1, _TEXT("r1") },
    { true,  VC4_QPU_WADDR_ACC2, _TEXT("r2"),
             VC4_QPU_WADDR_ACC2, _TEXT("r2") },
    { true,  VC4_QPU_WADDR_ACC3, _TEXT("r3"),
             VC4_QPU_WADDR_ACC3, _TEXT("r3") },
    { true,  VC4_QPU_WADDR_TMU_NOSWAP, _TEXT("tmu_noswap"),
             VC4_QPU_WADDR_TMU_NOSWAP, _TEXT("tmu_noswap") },
    { false, VC4_QPU_WADDR_ACC5, _TEXT("r5_replicate_pixel_0"),  // A: replicate pixel 0 per quad.
             VC4_QPU_WADDR_ACC5, _TEXT("r5_replicate_SIMD_0") }, // B: replicate SIMD element 0.
    { false, VC4_QPU_WADDR_ACC5, _TEXT("r5"),
             VC4_QPU_WADDR_ACC5, _TEXT("r5") },
    { true,  VC4_QPU_WADDR_HOSTINT, _TEXT("hostint"),
             VC4_QPU_WADDR_HOSTINT, _TEXT("hostint") },
    { true,  VC4_QPU_WADDR_NOP,     _TEXT(""),
             VC4_QPU_WADDR_NOP,     _TEXT("") },
    { true,  VC4_QPU_WADDR_NOP,     _TEXT("nop"),
             VC4_QPU_WADDR_NOP,     _TEXT("nop") },
    { true,  VC4_QPU_WADDR_UNIFORM_ADDRESS, _TEXT("uniform_address"),
             VC4_QPU_WADDR_UNIFORM_ADDRESS, _TEXT("uniform_address") },
    { false, VC4_QPU_WADDR_QUAD_X,  _TEXT("quad_X"),   // X for regfile A.
             VC4_QPU_WADDR_QUAD_Y,  _TEXT("quad_Y") }, // Y for regfile B.
    { false, VC4_QPU_WADDR_MS_FLAGS, _TEXT("ms_flags"),   // regfile A.
             VC4_QPU_WADDR_REV_FLAG, _TEXT("rev_flag") }, // regfile B.
    { true,  VC4_QPU_WADDR_TLB_STENCIL_SETUP, _TEXT("tlb_stencil_setup"),
             VC4_QPU_WADDR_TLB_STENCIL_SETUP, _TEXT("tlb_stencil_setup") },
    { true,  VC4_QPU_WADDR_TLB_Z,     _TEXT("tlb_z"),
             VC4_QPU_WADDR_TLB_Z,     _TEXT("tlb_z") },
    { true,  VC4_QPU_WADDR_TLB_COLOUR_MS, _TEXT("tlb_colour_ms"),
             VC4_QPU_WADDR_TLB_COLOUR_MS, _TEXT("tlb_colour_ms") },
    { true,  VC4_QPU_WADDR_TLB_COLOUR_ALL, _TEXT("tlb_colour"),
             VC4_QPU_WADDR_TLB_COLOUR_ALL, _TEXT("tlb_colour") },
    { true,  VC4_QPU_WADDR_TLB_COLOUR_ALL, _TEXT("tlb_c"),
             VC4_QPU_WADDR_TLB_COLOUR_ALL, _TEXT("tlb_c") },
    { true,  VC4_QPU_WADDR_TLB_ALPHA_MASK, _TEXT("tlb_alpha_mask"),
             VC4_QPU_WADDR_TLB_ALPHA_MASK, _TEXT("tlb_alpha_mask") },
    { true,  VC4_QPU_WADDR_VPM, _TEXT("vpm"),
             VC4_QPU_WADDR_VPM, _TEXT("vpm") },
    { false, VC4_QPU_WADDR_VPMVCD_RD_SETUP, _TEXT("vpm_rd_setup"),   // regfile A
             VC4_QPU_WADDR_VPMVCD_WR_SETUP, _TEXT("vpm_wr_setup") }, // regfile B
    { false, VC4_QPU_WADDR_VPMVCD_RD_SETUP, _TEXT("vr_setup"),   // regfile A
             VC4_QPU_WADDR_VPMVCD_WR_SETUP, _TEXT("vw_setup") }, // regfile B
    { false, VC4_QPU_WADDR_VPM_LD_ADDR, _TEXT("vpm_ld_addr"),   // regfile A
             VC4_QPU_WADDR_VPM_ST_ADDR, _TEXT("vpm_st_addr") }, // regfile B
    { true,  VC4_QPU_WADDR_MUTEX_RELEASE, _TEXT("mutex_release"),
             VC4_QPU_WADDR_MUTEX_RELEASE, _TEXT("mutex_release") },
    { true,  VC4_QPU_WADDR_SFU_RECIP, _TEXT("sfu_recip"),
             VC4_QPU_WADDR_SFU_RECIP, _TEXT("sfu_recip") },
    { true,  VC4_QPU_WADDR_SFU_RECIPSQRT, _TEXT("sfu_recipsqrt"),
             VC4_QPU_WADDR_SFU_RECIPSQRT, _TEXT("sfu_recipsqrt") },
    { true,  VC4_QPU_WADDR_SFU_EXP, _TEXT("sfu_exp"),
             VC4_QPU_WADDR_SFU_EXP, _TEXT("sfu_exp") },
    { true,  VC4_QPU_WADDR_SFU_LOG, _TEXT("sfu_log"),
             VC4_QPU_WADDR_SFU_LOG, _TEXT("sfu_log") },
    { true,  VC4_QPU_WADDR_TMU0_S, _TEXT("tmu0_s"),
             VC4_QPU_WADDR_TMU0_S, _TEXT("tmu0_s") },  // X - retiring
    { true,  VC4_QPU_WADDR_TMU0_T, _TEXT("tmu0_t"),
             VC4_QPU_WADDR_TMU0_T, _TEXT("tmu0_t") },  // Y
    { true,  VC4_QPU_WADDR_TMU0_R, _TEXT("tmu0_r"),
             VC4_QPU_WADDR_TMU0_R, _TEXT("tmu0_r") },  // Z 
    { true,  VC4_QPU_WADDR_TMU0_B, _TEXT("tmu0_b"),
             VC4_QPU_WADDR_TMU0_B, _TEXT("tmu0_b") },  // LOD Bias
    { true,  VC4_QPU_WADDR_TMU0_S, _TEXT("tmu1_s"),
             VC4_QPU_WADDR_TMU0_S, _TEXT("tmu1_s") },  // X - retiring
    { true,  VC4_QPU_WADDR_TMU0_T, _TEXT("tmu1_t"),
             VC4_QPU_WADDR_TMU0_T, _TEXT("tmu1_t") },  // Y
    { true,  VC4_QPU_WADDR_TMU0_R, _TEXT("tmu1_r"),
             VC4_QPU_WADDR_TMU0_R, _TEXT("tmu1_r") },  // Z
    { true,  VC4_QPU_WADDR_TMU0_B, _TEXT("tmu1_b") ,
             VC4_QPU_WADDR_TMU0_B, _TEXT("tmu1_b") },  // LOD Bias
    { true,  VC4_QPU_END_OF_LOOKUPTABLE, NULL,
             VC4_QPU_END_OF_LOOKUPTABLE, NULL },
};

_declspec(selectany) VC4QPU_TOKENLOOKUP_TABLE VC4_QPU_OPCODE_MUL_LOOKUP[] =
{
    { VC4_QPU_OPCODE_MUL_NOP, _TEXT("nop") },
    { VC4_QPU_OPCODE_MUL_FMUL, _TEXT("fmul") },
    { VC4_QPU_OPCODE_MUL_MUL24, _TEXT("mul24") },
    { VC4_QPU_OPCODE_MUL_V8MULD, _TEXT("v8muld") },
    { VC4_QPU_OPCODE_MUL_V8MIN, _TEXT("v8min") },
    { VC4_QPU_OPCODE_MUL_V8MAX, _TEXT("v8max") },
    { VC4_QPU_OPCODE_MUL_V8ADDS, _TEXT("v8add_saturate") },
    { VC4_QPU_OPCODE_MUL_V8SUBS, _TEXT("v8sub_saturate") },
    { VC4_QPU_OPCODE_MUL_MOV, _TEXT("mov") },
    { VC4_QPU_END_OF_LOOKUPTABLE, NULL }
};

_declspec(selectany) VC4QPU_TOKENLOOKUP_TABLE VC4_QPU_OPCODE_ADD_LOOKUP[] =
{
    { VC4_QPU_OPCODE_ADD_NOP, _TEXT("nop") },
    { VC4_QPU_OPCODE_ADD_FADD, _TEXT("fadd") },
    { VC4_QPU_OPCODE_ADD_FSUB, _TEXT("fsub") },
    { VC4_QPU_OPCODE_ADD_FMIN, _TEXT("fmin") },
    { VC4_QPU_OPCODE_ADD_FMAX, _TEXT("fmax") },
    { VC4_QPU_OPCODE_ADD_FMIN_ABS, _TEXT("fmin_abs") },
    { VC4_QPU_OPCODE_ADD_FMAX_ABS, _TEXT("fmax_abs") },
    { VC4_QPU_OPCODE_ADD_FTOI, _TEXT("ftoi") },
    { VC4_QPU_OPCODE_ADD_ITOF, _TEXT("itof") },
    { VC4_QPU_OPCODE_ADD_ADD, _TEXT("add") },
    { VC4_QPU_OPCODE_ADD_SUB, _TEXT("sub") },
    { VC4_QPU_OPCODE_ADD_SHR, _TEXT("shr") },
    { VC4_QPU_OPCODE_ADD_ASR, _TEXT("asr") },
    { VC4_QPU_OPCODE_ADD_ROR, _TEXT("ror") },
    { VC4_QPU_OPCODE_ADD_SHL, _TEXT("shl") },
    { VC4_QPU_OPCODE_ADD_MIN, _TEXT("min") },
    { VC4_QPU_OPCODE_ADD_MAX, _TEXT("max") },
    { VC4_QPU_OPCODE_ADD_AND, _TEXT("and") },
    { VC4_QPU_OPCODE_ADD_OR, _TEXT("or") },
    { VC4_QPU_OPCODE_ADD_XOR, _TEXT("xor") },
    { VC4_QPU_OPCODE_ADD_NOT, _TEXT("not") },
    { VC4_QPU_OPCODE_ADD_CLZ, _TEXT("clz") },
    { VC4_QPU_OPCODE_ADD_V8ADDS, _TEXT("v8add_saturate") },
    { VC4_QPU_OPCODE_ADD_V8SUBS, _TEXT("v8sub_saturate") },
    { VC4_QPU_OPCODE_ADD_MOV, _TEXT("mov") },
    { VC4_QPU_END_OF_LOOKUPTABLE, NULL }
};

_declspec(selectany) VC4QPU_TOKENLOOKUP_ADDR_TABLE VC4_QPU_RADDR_LOOKUP[] =
{
    { false, 0, _TEXT("ra0"), 
             0, _TEXT("rb0") },
    { false, 1, _TEXT("ra1"),
             1, _TEXT("rb1") },
    { false, 2, _TEXT("ra2"),
             2, _TEXT("rb2") },
    { false, 3, _TEXT("ra3"),
             3, _TEXT("rb3") },
    { false, 4, _TEXT("ra4"), 
             4, _TEXT("rb4") },
    { false, 5, _TEXT("ra5"),
             5, _TEXT("rb5") },
    { false, 6, _TEXT("ra6"),
             6, _TEXT("rb6") },
    { false, 7, _TEXT("ra7"),
             7, _TEXT("rb7") },
    { false, 8, _TEXT("ra8"),
             8, _TEXT("rb8") },
    { false, 9, _TEXT("ra9"),
             9, _TEXT("rb9") },
    { false, 10, _TEXT("ra10"),
             10, _TEXT("rb10") },
    { false, 11, _TEXT("ra11"),
             11, _TEXT("rb11") },
    { false, 12, _TEXT("ra12"),
             12, _TEXT("rb12") },
    { false, 13, _TEXT("ra13"), 
             13, _TEXT("rb13") },
    { false, 14, _TEXT("ra14"),
             14, _TEXT("rb14") },
    { false, 15, _TEXT("ra15"),
             15, _TEXT("rb15") },
    { false, 16, _TEXT("ra16"),
             16, _TEXT("rb16") },
    { false, 17, _TEXT("ra17"),
             17, _TEXT("rb17") },
    { false, 18, _TEXT("ra18"),
             18, _TEXT("rb18") },
    { false, 19, _TEXT("ra19"),
             19, _TEXT("rb19") },
    { false, 20, _TEXT("ra20"),
             20, _TEXT("rb20") },
    { false, 21, _TEXT("ra21"),
             21, _TEXT("rb21") },
    { false, 22, _TEXT("ra22"),
             22, _TEXT("rb22") },
    { false, 23, _TEXT("ra23"),
             23, _TEXT("rb23") },
    { false, 24, _TEXT("ra24"),
             24, _TEXT("rb24") },
    { false, 25, _TEXT("ra25"),
             25, _TEXT("rb25") },
    { false, 26, _TEXT("ra26"),
             26, _TEXT("rb26") },
    { false, 27, _TEXT("ra27"),
             27, _TEXT("rb27") },
    { false, 28, _TEXT("ra28"),
             28, _TEXT("rb28") },
    { false, 29, _TEXT("ra29"),
             29, _TEXT("rb29") },
    { false, 30, _TEXT("ra30"),
             30, _TEXT("rb30") },
    { false, 31, _TEXT("ra31"),
             31, _TEXT("rb31") },
    { true,  VC4_QPU_RADDR_UNIFORM, _TEXT("uniform"),
             VC4_QPU_RADDR_UNIFORM, _TEXT("uniform") },
    { true,  VC4_QPU_RADDR_VERYING, _TEXT("varying"),
             VC4_QPU_RADDR_VERYING, _TEXT("varying") },
    { true,  VC4_QPU_RADDR_VERYING, _TEXT("vary"),
             VC4_QPU_RADDR_VERYING, _TEXT("vary") },
    { false, VC4_QPU_RADDR_ELEMENT_NUMBER, _TEXT("element_number"), // regfile A
             VC4_QPU_RADDR_QPU_NUMBER,     _TEXT("qpu_number") },   // regfile B
    { true,  VC4_QPU_RADDR_NOP, _TEXT(""),
             VC4_QPU_RADDR_NOP, _TEXT("") },
    { true,  VC4_QPU_RADDR_NOP, _TEXT("nop"),
             VC4_QPU_RADDR_NOP, _TEXT("nop") },
    { false, VC4_QPU_RADDR_PIXEL_COORD_X, _TEXT("pixel_coord_x"),   // regfile A
             VC4_QPU_RADDR_PIXEL_COORD_Y, _TEXT("pixel_coord_y") }, // regfile B
    { false, VC4_QPU_RADDR_MS_FLAGS, _TEXT("ms_flags"),    // regfile A
             VC4_QPU_RADDR_REV_FLAG, _TEXT("rev_flag") }, // regfile B
    { true,  VC4_QPU_RADDR_VPM, _TEXT("vpm"),
             VC4_QPU_RADDR_VPM, _TEXT("vpm") },
    { true,  VC4_QPU_RADDR_VPM, _TEXT("vpmread"),
             VC4_QPU_RADDR_VPM, _TEXT("vpmread") },
    { false, VC4_QPU_RADDR_VPM_LD_BUSY, _TEXT("vpm_ld_busy"),   // regfile A
             VC4_QPU_RADDR_VPM_ST_BUSY, _TEXT("vpm_st_busy") }, // regfile B
    { false, VC4_QPU_RADDR_VPM_LD_WAIT, _TEXT("vpm_ld_wait"),   // regfile A
             VC4_QPU_RADDR_VPM_ST_WAIT, _TEXT("vpm_st_wait") }, // regfile B
    { true,  VC4_QPU_RADDR_MUTEX_ACQUIRE, _TEXT("mutex_acquire"),
             VC4_QPU_RADDR_MUTEX_ACQUIRE, _TEXT("mutex_acquire") },
    { true,  VC4_QPU_END_OF_LOOKUPTABLE, NULL,
             VC4_QPU_END_OF_LOOKUPTABLE, NULL }
};

_declspec(selectany) VC4QPU_TOKENLOOKUP_TABLE VC4_QPU_ALU_LOOKUP[] =
{
    { VC4_QPU_ALU_R0, _TEXT("r0") },
    { VC4_QPU_ALU_R1, _TEXT("r1") },
    { VC4_QPU_ALU_R2, _TEXT("r2") },
    { VC4_QPU_ALU_R3, _TEXT("r3") },
    { VC4_QPU_ALU_R4, _TEXT("r4") },
    { VC4_QPU_ALU_R5, _TEXT("r5") },
//  { VC4_QPU_ALU_REG_A, _TEXT("ra") },
//  { VC4_QPU_ALU_REG_B, _TEXT("rb") },
    { VC4_QPU_END_OF_LOOKUPTABLE, NULL }
};

_declspec(selectany) VC4QPU_TOKENLOOKUP_TABLE VC4_QPU_BRANCH_COND_LOOKUP[] =
{
    { VC4_QPU_BRANCH_COND_ALL_ZS, _TEXT(".if_all_zs") }, // All Z flags set
    { VC4_QPU_BRANCH_COND_ALL_ZC, _TEXT(".if_all_zc") }, // All Z flags clear
    { VC4_QPU_BRANCH_COND_ANY_ZS, _TEXT(".if_any_zs") }, // Any Z flags set
    { VC4_QPU_BRANCH_COND_ANY_ZC, _TEXT(".if_any_zc") }, // Any Z flags clear
    { VC4_QPU_BRANCH_COND_ALL_NS, _TEXT(".if_all_ns") }, // All N flags set
    { VC4_QPU_BRANCH_COND_ALL_NC, _TEXT(".if_all_nc") }, // All N flags clear
    { VC4_QPU_BRANCH_COND_ANY_NS, _TEXT(".if_any_ns") }, // Any N flags set
    { VC4_QPU_BRANCH_COND_ANY_NC, _TEXT(".if_any_nc") }, // Any N flags clear
    { VC4_QPU_BRANCH_COND_ALL_CS, _TEXT(".if_all_cs") }, // All C flags set
    { VC4_QPU_BRANCH_COND_ALL_CC, _TEXT(".if_all_cc") }, // All C flags clear
    { VC4_QPU_BRANCH_COND_ANY_CS, _TEXT(".if_any_cs") }, // Any C flags set
    { VC4_QPU_BRANCH_COND_ANY_CC, _TEXT(".if_any_cc") }, // Any C flags clear
    { VC4_QPU_BRANCH_COND_ALWAYS, _TEXT("")           }, // Always execute
    { VC4_QPU_END_OF_LOOKUPTABLE, NULL }
};

