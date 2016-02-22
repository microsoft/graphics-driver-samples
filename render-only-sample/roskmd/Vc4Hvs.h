#ifndef _VC4HVS_HPP_
#define  _VC4HVS_HPP_ 1
//
// Copyright (C) Microsoft.  All rights reserved.
//
//
// Module Name:
//
//  Vc4Hvs.h
//
// Abstract:
//
//    VC4 Hardware Video Scaler (HVS) Register Definitions
//
// Author:
//
//    Jordan Rhee (jordanrh) December 2015
//
// Environment:
//
//    Kernel mode only.
//

#pragma warning(disable:4201)   // nameless struct/union

// Despite what the documentation says this is reading back as lowercase ddrv
// instead of upercase DDRV
enum : ULONG { VC4HVS_DISPID = 'ddrv' };

//
// DISPCTRL Register Field Definitions
//

enum VC4HVS_BUS_PRIORITY_MODE : ULONG {
    VC4HVS_BUS_PRIORITY_MODE_NEVER_PANIC,
    VC4HVS_BUS_PRIORITY_MODE_ALWAYS_RAISE,
    VC4HVS_BUS_PRIORITY_MODE_PANIC_LOW_FIFO,
    VC4HVS_BUS_PRIORITY_MODE_ALWAYS_PANIC,
};

union VC4HVS_DISPCTRL {
    ULONG AsUlong;
    struct {
        // LSB
        ULONG SCLEIRQ : 1;
        ULONG DISP0EIRQ : 1;
        ULONG DISP1EIRQ : 1;
        ULONG DISP2EIRQ : 1;
        ULONG DMAEIRQ : 1;
        ULONG SLVWREIRQ : 1;
        ULONG SLVRDEIRQ : 1;
        ULONG DSP0EIEOF : 1;
        ULONG DSP0EIEOLN : 1;
        ULONG DSP1EIEOF : 1;
        ULONG DSP1EIEOLN : 1;
        ULONG DSP2EIEOF : 1;
        ULONG DSP2EIEOLN : 1;
        ULONG DSP0EISLUR : 1;
        ULONG DSP1EISLUR : 1;
        ULONG DSP2EISLUR : 1;
        ULONG H2O_MARK : 2;
        ULONG DSP3_MUX : 2;
        ULONG reserved : 4;
        VC4HVS_BUS_PRIORITY_MODE PRIORITY0 : 2;
        VC4HVS_BUS_PRIORITY_MODE PRIORITY1 : 2;
        VC4HVS_BUS_PRIORITY_MODE PRIORITY2 : 2;
        ULONG VSCLPDOWN : 1;
        ULONG ENABLE : 1;
        // MSB
    } DUMMYSTRUCTNAME;
};

static_assert(
    sizeof(VC4HVS_DISPCTRL) == sizeof(ULONG),
    "Sanity check on size of VC4HVS_DISPCTRL");

enum VC4HVS_DMA_RESPONSE_ERROR : ULONG {
    VC4HVS_DMA_RESPONSE_ERROR_OK,
    VC4HVS_DMA_RESPONSE_ERROR_EXOK,
    VC4HVS_DMA_RESPONSE_ERROR_SLVERR,
    VC4HVS_DMA_RESPONSE_ERROR_DECERR,
};

union VC4HVS_DISPSTAT {
    ULONG AsUlong;
    struct {
        // LSB
        ULONG IRQSCL : 1;
        ULONG IRSDISP0 : 1;
        ULONG IRQDISP1 : 1;
        ULONG IRQDISP2 : 1;
        ULONG IRQDMA : 1;
        ULONG IRQSLVWR : 1;
        ULONG IRQSLVRD : 1;
        ULONG DMA_ERROR : 1;
        ULONG EOF0 : 1;
        ULONG EUFLOW0 : 1;
        ULONG ESLINE0 : 1;
        ULONG ESFRAME0 : 1;
        ULONG EOLN0 : 1;
        ULONG COBLOW0 : 1;
        VC4HVS_DMA_RESPONSE_ERROR RESP_ERROR : 2;
        ULONG EOF1 : 1;
        ULONG EUFLOW1 : 1;
        ULONG ESLINE1 : 1;
        ULONG ESFRAME1 : 1;
        ULONG EOLN1 : 1;
        ULONG COBLOW1 : 1;
        ULONG reserved1 : 2;
        ULONG EOF2 : 1;
        ULONG EUFLOW2 : 1;
        ULONG ESLINE2 : 1;
        ULONG ESFRAME2 : 1;
        ULONG EOL2 : 1;
        ULONG COBLOW2 : 1;
        ULONG reserved2 : 2;
        // MSB
    } DUMMYSTRUCTNAME;
};

enum : ULONG { VC4HVS_DISPSTAT_WRITABLE_BITS_MASK = 0x3F3F3FE0UL };

static_assert(
    sizeof(VC4HVS_DISPSTAT) == sizeof(ULONG),
    "Sanity check on size of VC4HVS_DISPSTAT");

enum VC4HVS_MAX_BURST_LENGTH : ULONG {
    VC4HVS_MAX_BURST_LENGTH_2_BEATS,
    VC4HVS_MAX_BURST_LENGTH_4_BEATS,
    VC4HVS_MAX_BURST_LENGTH_8_BEATS,
    VC4HVS_MAX_BURST_LENGTH_16_BEATS,
};

enum VC4HVS_PROFILE_TYPE : ULONG {
    VC4HVS_PROFILE_TYPE_COMPOSITE_OUTPUT_LINE,
    VC4HVS_PROFILE_TYPE_STALLED_BY_MEMORY,
    VC4HVS_PROFILE_TYPE_ACTIVE,
    VC4HVS_PROFILE_TYPE_OVERHEAD,
};

union VC4HVS_DISPECTRL {
    ULONG AsUlong;
    struct {
        // LSB
        ULONG Y_PANIC_MD : 1;
        ULONG Y_PANIC_EN : 1;
        ULONG CB_PANIC_MD : 1;
        ULONG CB_PANIC_EN : 1;
        ULONG CR_PANIC_MD : 1;
        ULONG CR_PANIC_EN : 1;
        ULONG FORCE_PANIC : 1;
        ULONG reserved1 : 1;
        ULONG ANY_BUSY : 1;
        ULONG Y_BUSY : 1;
        ULONG CB_BUSY : 1;
        ULONG CR_BUSY : 1;
        ULONG Y_OUT_REQ : 1;
        ULONG CB_OUT_REQ : 1;
        ULONG CR_OUT_REQ : 1;
        ULONG reserved2 : 1;
        ULONG Y_OUTSTAND : 2;
        ULONG CB_OUTSTAND : 2;
        ULONG CR_OUTSTAND : 2;
        ULONG NO_STBY : 1;
        VC4HVS_MAX_BURST_LENGTH MAX_BURST : 2;
        ULONG reserved3 : 1;
        VC4HVS_PROFILE_TYPE PROF_TYPE : 2;
        ULONG reserved4 : 3;
        ULONG SECURE_CTRL : 1;
        // MSB
    } DUMMYSTRUCTNAME;
};

static_assert(
    sizeof(VC4HVS_DISPECTRL) == sizeof(ULONG),
    "Sanity check on size of VC4HVS_DISPECTRL");

enum VC4HVS_PROFILE_MODE : ULONG {
    VC4HVS_PROFILE_MODE_COUNT_FIFO0_LINE_GEN,
    VC4HVS_PROFILE_MODE_COUNT_FIFO1_LINE_GEN,
    VC4HVS_PROFILE_MODE_COUNT_FIFO2_LINE_GEN,
    VC4HVS_PROFILE_MODE_COUNT_PASS,
};

union VC4HVS_DISPPROF {
    ULONG AsUlong;
    struct {
        // LSB
        ULONG MAX : 24;
        VC4HVS_PROFILE_MODE MODE : 2;
        ULONG ENBIRQ : 1;
        ULONG OVER : 1;
        ULONG reserved : 1;
        ULONG PANIC : 1;
        ULONG CLEAR : 1;
        ULONG ENB : 1;
        // MSB
    } DUMMYSTRUCTNAME;
};

static_assert(
    sizeof(VC4HVS_DISPPROF) == sizeof(ULONG),
    "Sanity check on size of VC4HVS_DISPPROF");

enum VC4HVS_DESTINATION_DISPLAY_BIT_DEPTH : ULONG {
    VC4HVS_DESTINATION_DISPLAY_BIT_DEPTH_RGB888,
    VC4HVS_DESTINATION_DISPLAY_BIT_DEPTH_RGB666,
    VC4HVS_DESTINATION_DISPLAY_BIT_DEPTH_RGB565,
    VC4HVS_DESTINATION_DISPLAY_BIT_DEPTH_RGB555,
};

enum VC4HVS_DITHER_TYPE : ULONG {
    VC4HVS_DITHER_TYPE_NONE,
    VC4HVS_DITHER_TYPE_ACCUM_HORIZ,
    VC4HVS_DITHER_TYPE_ACCUM_HORIZ_WITH_NOISE,
    VC4HVS_DITHER_TYPE_ACCUM_HORIZ_WITH_NOISE_FREE_RUNNING,
};

union VC4HVS_DISPDITHER {
    ULONG AsUlong;
    struct {
        // LSB
        VC4HVS_DITHER_TYPE TYPE0 : 2;
        VC4HVS_DESTINATION_DISPLAY_BIT_DEPTH DEPTH0 : 2;
        ULONG ABORT0 : 1;
        ULONG reserved1 : 3;
        VC4HVS_DITHER_TYPE TYPE1 : 2;
        VC4HVS_DESTINATION_DISPLAY_BIT_DEPTH DEPTH1 : 2;
        ULONG ABORT1 : 1;
        ULONG reserved2 : 3;
        VC4HVS_DITHER_TYPE TYPE2 : 2;
        VC4HVS_DESTINATION_DISPLAY_BIT_DEPTH DEPTH2 : 2;
        ULONG ABORT2 : 1;
        ULONG reserved3 : 11;
        // MSB
    } DUMMYSTRUCTNAME;
};

static_assert(
    sizeof(VC4HVS_DISPDITHER) == sizeof(ULONG),
    "Sanity check on size of VC4HVS_DISPDITHER");

union VC4HVS_DISPEOLN {
    ULONG AsUlong;
    struct {
        // LSB
        ULONG EOLN0 : 9;
        ULONG EOLN1 : 9;
        ULONG EOLN2 : 9;
        ULONG reserved : 5;
        // MSB
    } DUMMYSTRUCTNAME;
};

static_assert(
    sizeof(VC4HVS_DISPEOLN) == sizeof(ULONG),
    "Sanity check on size of VC4HVS_DISPEOLN");

union VC4HVS_DISPLIST {
    ULONG AsUlong;
    struct {
        // LSB
        ULONG HEADE : 12;
        ULONG reserved1 : 4;
        ULONG HEADO : 12;
        ULONG reserved2 : 3;
        ULONG USEODD : 1;
        // MSB
    } DUMMYSTRUCTNAME;
};

static_assert(
    sizeof(VC4HVS_DISPLIST) == sizeof(ULONG),
    "Sanity check on size of VC4HVS_DISPLIST");

enum VC4HVS_VERSION : ULONG {
    VC4HVS_VERSION_VC3A = 0,
    VC4HVS_VERSION_VC3B = 0x20,
    VC4HVS_VERSION_VC4 = 0x30,
};

union VC4HVS_DISPLSTAT {
    ULONG AsUlong;
    struct {
        // LSB
        ULONG DLP0PFTILE : 1;
        ULONG DL0SIZELO : 1;
        ULONG DL0SIZEHI : 1;
        ULONG DL0PIXFMT : 1;
        ULONG reserved1 : 4;
        ULONG DLP1PFTILE : 1;
        ULONG DL1SIZELO : 1;
        ULONG DL1SIZEHI : 1;
        ULONG DL1PIXFMT : 1;
        ULONG reserved2 : 4;
        ULONG DLP2PFTILE : 1;
        ULONG DL2SIZELO : 1;
        ULONG DL2SIZEHI : 1;
        ULONG DL2PIXFMT : 1;
        ULONG reserved3 : 4;
        ULONG VERSION : 8;
        // MSB
    } DUMMYSTRUCTNAME;
};

static_assert(
    sizeof(VC4HVS_DISPLSTAT) == sizeof(ULONG),
    "Sanity check on size of VC4HVS_DISPLSTAT");

union VC4HVS_DISPLACT {
    ULONG AsUlong;
    struct {
        // LSB
        ULONG LACT : 16;        // Active Display List Address
        ULONG reserved : 16;
        // MSB
    } DUMMYSTRUCTNAME;
};

static_assert(
    sizeof(VC4HVS_DISPLACT) == sizeof(ULONG),
    "Sanity check on size of VC4HVS_DISPLACT");

enum VC4HVS_FIFO_OUTPUT_PIXEL_FORMAT : ULONG {
    VC4HVS_FIFO_OUTPUT_PIXEL_FORMAT_RGB888,
    VC4HVS_FIFO_OUTPUT_PIXEL_FORMAT_RGB565,
    VC4HVS_FIFO_OUTPUT_PIXEL_FORMAT_RGB666,
    VC4HVS_FIFO_OUTPUT_PIXEL_FORMAT_YCBCR,
};

union VC4HVS_DISPFIFOCTRL {
    ULONG AsUlong;
    struct {
        // LSB
        ULONG LINES : 12;
        ULONG WIDTH : 12;
        VC4HVS_FIFO_OUTPUT_PIXEL_FORMAT RGBFMT : 2;
        ULONG FIFOREG : 1;
        ULONG FIFO32 : 1;
        ULONG ONECTX : 1;
        ULONG ONESHOT : 1;
        ULONG RESET : 1;
        ULONG ENB : 1;
        // MSB
    } DUMMYSTRUCTNAME;
};

static_assert(
    sizeof(VC4HVS_DISPFIFOCTRL) == sizeof(ULONG),
    "Sanity check on size of VC4HVS_DISPFIFOCTRL");

enum VC4HVS_VIDEO_TEST_MODE : ULONG {
    VC4HVS_VIDEO_TEST_MODE_DISABLE,
    VC4HVS_VIDEO_TEST_MODE_SOLID_COLOR,
    VC4HVS_VIDEO_TEST_MODE_SOLID_RGB_VERT_BARS,
    VC4HVS_VIDEO_TEST_MODE_SHADED_RGB_VERT_BARS,
    VC4HVS_VIDEO_TEST_MODE_SHADED_WHITE_VERT_BARS,
    VC4HVS_VIDEO_TEST_MODE_SHADED_WHITE_HORIZ_BARS,
    VC4HVS_VIDEO_TEST_MODE_SHADED_RGB_AND_WHITE_HORIZ_BARS,
    VC4HVS_VIDEO_TEST_MODE_WALKING_ONE,
    VC4HVS_VIDEO_TEST_MODE_DELAYED_SHADED_RGB_VERT_BARS,
    VC4HVS_VIDEO_TEST_MODE_HORIZ_GREEN_VERT_BLUE_DIAG_RED_BARS,
    VC4HVS_VIDEO_TEST_MODE_ODD_FIELD_CROSSHAIRS,
    VC4HVS_VIDEO_TEST_MODE_EVEN_FIELD_CROSSHAIRS,
    VC4HVS_VIDEO_TEST_MODE_WHITE_32X32_GRID,
    VC4HVS_VIDEO_TEST_MODE_SOLID_WYCGMRBK_VERT_BARS,
    VC4HVS_VIDEO_TEST_MODE_SHADED_WYCGMRBK_VERT_BARS,
    VC4HVS_VIDEO_TEST_MODE_WHITE_32X32_DIAG_GRID,
};

union VC4HVS_DISPBKGND {
    ULONG AsUlong;
    struct {
        // LSB
        ULONG BGBLU : 8;
        ULONG BGGRN : 8;
        ULONG BGRED : 8;
        ULONG BGENB : 1;
        VC4HVS_VIDEO_TEST_MODE TESTM : 4;
        ULONG GAMMA : 1;
        ULONG INTLACE : 1;
        ULONG AUTOHS : 1;
        // MSB
    } DUMMYSTRUCTNAME;
};

static_assert(
    sizeof(VC4HVS_DISPBKGND) == sizeof(ULONG),
    "Sanity check on size of VC4HVS_DISPBKGND");

enum VC4HVS_WALKSTATE : ULONG {
    VC4HVS_WALKSTATE_WAIT,
    VC4HVS_WALKSTATE_PARSING,
    VC4HVS_WALKSTATE_SCALING,
    VC4HVS_WALKSTATE_DONE,
};

enum VC4HVS_FRAME_GENERATION_STATE : ULONG {
    VC4HVS_FRAME_GENERATION_STATE_DISABLE,
    VC4HVS_FRAME_GENERATION_STATE_INIT,
    VC4HVS_FRAME_GENERATION_STATE_RUN,
    VC4HVS_FRAME_GENERATION_STATE_EOF,
};

union VC4HVS_DISPFIFOSTAT {
    ULONG AsUlong;
    struct {
        // LSB
        ULONG YLINE : 12;
        VC4HVS_WALKSTATE WALKSTATE : 2;
        ULONG CUR_IDLE : 13;
        ULONG FIELD : 1;
        ULONG EMPTY : 1;
        ULONG FULL : 1;
        VC4HVS_FRAME_GENERATION_STATE MODE : 2;
        // MSB
    } DUMMYSTRUCTNAME;
};

static_assert(
    sizeof(VC4HVS_DISPFIFOSTAT) == sizeof(ULONG),
    "Sanity check on size of VC4HVS_DISPFIFOSTAT");

union VC4HVS_DISPFIFOBASE {
    ULONG AsUlong;
    struct {
        // LSB
        ULONG BASE : 16;
        ULONG TOP : 16;
        // MSB
    } DUMMYSTRUCTNAME;
};

static_assert(
    sizeof(VC4HVS_DISPFIFOBASE) == sizeof(ULONG),
    "Sanity check on size of VC4HVS_DISPFIFOBASE");

union VC4HVS_DISPALPHA2 {
    ULONG AsUlong;
    struct {
        // LSB
        ULONG ALPHA : 8;
        ULONG reserved : 24;
        // MSB
    } DUMMYSTRUCTNAME;
};

static_assert(
    sizeof(VC4HVS_DISPALPHA2) == sizeof(ULONG),
    "Sanity check on size of VC4HVS_DISPALPHA2");

union VC4HVS_GAMADDR {
    ULONG AsUlong;
    struct {
        // LSB
        ULONG ADDR : 18;
        ULONG reserved : 12;
        ULONG SRAMENB : 1;
        ULONG AUTOINC : 1;
        // MSB
    } DUMMYSTRUCTNAME;
};

static_assert(
    sizeof(VC4HVS_GAMADDR) == sizeof(ULONG),
    "Sanity check on size of VC4HVS_GAMADDR");

//
// Display List Related Definitions
//

enum VC4HVS_SOURCE_PIXEL_FORMAT : ULONG {
    VC4HVS_SOURCE_PIXEL_FORMAT_RGB332,
    VC4HVS_SOURCE_PIXEL_FORMAT_RGBA4444,
    VC4HVS_SOURCE_PIXEL_FORMAT_RGB555,
    VC4HVS_SOURCE_PIXEL_FORMAT_RGBA5551,
    VC4HVS_SOURCE_PIXEL_FORMAT_RGB565,
    VC4HVS_SOURCE_PIXEL_FORMAT_RGB888,
    VC4HVS_SOURCE_PIXEL_FORMAT_RGBA6666,
    VC4HVS_SOURCE_PIXEL_FORMAT_RGBA8888,
    VC4HVS_SOURCE_PIXEL_FORMAT_YCBCR420,
    VC4HVS_SOURCE_PIXEL_FORMAT_YCBCR420_INTERLEAVED_CHROMA,
    VC4HVS_SOURCE_PIXEL_FORMAT_YCBCR422,
    VC4HVS_SOURCE_PIXEL_FORMAT_YCBCR422_INTERLEAVED_CHROMA,
    VC4HVS_SOURCE_PIXEL_FORMAT_H264,
    VC4HVS_SOURCE_PIXEL_FORMAT_PALETTE,
    VC4HVS_SOURCE_PIXEL_FORMAT_YCBCR888_COLOR_CONVERTED,
    VC4HVS_SOURCE_PIXEL_FORMAT_YCBCR6666_COLOR_CONVERTED,
};

enum VC4HVS_SCALAR_CHANNEL_MODE : ULONG {
    VC4HVS_SCALAR_CHANNEL_MODE_VPPF_HPPF,
    VC4HVS_SCALAR_CHANNEL_MODE_HTPZ_VPPF,
    VC4HVS_SCALAR_CHANNEL_MODE_VTPZ_HPPF,
    VC4HVS_SCALAR_CHANNEL_MODE_HTPZ_VTPZ,
    VC4HVS_SCALAR_CHANNEL_MODE_HPPF,
    VC4HVS_SCALAR_CHANNEL_MODE_VPPF,
    VC4HVS_SCALAR_CHANNEL_MODE_VTPZ,
    VC4HVS_SCALAR_CHANNEL_MODE_HTPZ,
};

enum VC4HVS_RGBA_EXPAND : ULONG {
    VC4HVS_RGBA_EXPAND_ZERO_PAD,
    VC4HVS_RGBA_EXPAND_REPEAT_LSB,
    VC4HVS_RGBA_EXPAND_REPEAT_MSB,
    VC4HVS_RGBA_EXPAND_REPEAT_AND_ROUND,
};

enum VC4HVS_RGBA_ORDER : ULONG {
    VC4HVS_RGBA_ORDER_RGBA,
    VC4HVS_RGBA_ORDER_BGRA,
    VC4HVS_RGBA_ORDER_ARGB,
    VC4HVS_RGBA_ORDER_ABGR,

    VC4HVS_RGBA_ORDER_BRG = 0,
    VC4HVS_RGBA_ORDER_RBG = 1,
    VC4HVS_RGBA_ORDER_RGB = 2,
    VC4HVS_RGBA_ORDER_BGR = 3,

    VC4HVS_RGBA_ORDER_XYCBCR = 0,
    VC4HVS_RGBA_ORDER_XYCRCB = 1,
    VC4HVS_RGBA_ORDER_YXCBCR = 2,
    VC4HVS_RGBA_ORDER_YXCRCB = 3,
};

enum VC4HVS_KEY_MODE : ULONG {
    VC4HVS_KEY_MODE_DISABLE,
    VC4HVS_KEY_MODE_LUMA_TANSPARENT,
    VC4HVS_KEY_MODE_LUMA_CHROMA_TRANSPARENT,
    VC4HVS_KEY_MODE_LUMA_CHROMA_REPLACE,
};

enum VC4HVS_TILE_ADDRESS_MODE : ULONG {
    VC4HVS_TILE_ADDRESS_MODE_LINEAR,
    VC4HVS_TILE_ADDRESS_MODE_TILE64,
    VC4HVS_TILE_ADDRESS_MODE_TILE128,
    VC4HVS_TILE_ADDRESS_MODE_TILE256,
};

union VC4HVS_DLIST_CONTROL_WORD_0 {
    ULONG AsUlong;
    struct {
        // LSB
        VC4HVS_SOURCE_PIXEL_FORMAT pixel_format : 4;
        ULONG unity : 1;
        VC4HVS_SCALAR_CHANNEL_MODE scl0_mode : 3;
        ULONG scl1_mode : 3;
        VC4HVS_RGBA_EXPAND rgba_expand : 2;
        VC4HVS_RGBA_ORDER rgba_order : 2;
        ULONG vflip : 1;
        ULONG hflip : 1;
        VC4HVS_KEY_MODE key_mode : 2;
        ULONG alpha_mask : 1;
        VC4HVS_TILE_ADDRESS_MODE tile_mode : 2;
        ULONG rgb_trans : 1;
        ULONG reserved : 1;
        ULONG next : 6;
        ULONG valid : 1;
        ULONG end : 1;
        // MSB
    } DUMMYSTRUCTNAME;
};

union VC4HVS_DLIST_POSITION_WORD_0 {
    ULONG AsUlong;
    struct {
        // LSB
        ULONG start_x : 12;
        ULONG start_y : 12;
        ULONG alpha : 8;
        // MSB
    } DUMMYSTRUCTNAME;
};

union VC4HVS_DLIST_POSITION_WORD_1 {
    ULONG AsUlong;
    struct {
        // LSB
        ULONG scl_width : 12;
        ULONG reserved1 : 4;
        ULONG sd_lines : 12;
        ULONG reserved2 : 4;
        // MSB
    } DUMMYSTRUCTNAME;
};

enum VC4VS_DLIST_ALPHA_MODE : ULONG {
    VC4VS_DLIST_ALPHA_MODE_SCALING_PIPELINE,
    VC4VS_DLIST_ALPHA_MODE_FIXED_FOR_ALL,
    VC4VS_DLIST_ALPHA_MODE_FIXED_FOR_NONZERO_ALPHA,
    VC4VS_DLIST_ALPHA_MODE_FIXED_FOR_GREATER7,
};

union VC4HVS_DLIST_POSITION_WORD_2 {
    ULONG AsUlong;
    struct {
        // LSB
        ULONG src_width : 12;
        ULONG reserved : 2;
        ULONG dma_3fifo : 1;
        ULONG cbcr_vrep : 1;
        ULONG src_lines : 12;
        ULONG alpha_mix : 1;
        ULONG alpha_premult : 1;
        VC4VS_DLIST_ALPHA_MODE alpha_mode : 2;
        // MSB
    } DUMMYSTRUCTNAME;
};

union VC4HVS_DLIST_POSITION_WORD_3 {
    ULONG AsUlong;
    struct {
        // LSB
        ULONG ctx_scl0_line : 12;       // updated by scaler
        ULONG reserved1 : 4;
        ULONG ctx_scl1_line : 12;       // updated by scaler
        ULONG reserved2 : 4;
        // MSB
    } DUMMYSTRUCTNAME;
};

union VC4HVS_DLIST_PITCH_WORD_0 {
    ULONG AsUlong;
    struct {
        ULONG src_pitch_0 : 16;
        ULONG reserved : 9;
        ULONG alpha_byte : 1;
        ULONG sink_pix : 5;
    } DUMMYSTRUCTNAME;
};

#include <pshpack4.h> //======================================================

struct VC4HVS_REGISTERS {
    ULONG DISPCTRL;         // Display Driver Control
    ULONG DISPSTAT;         // Display Driver Status
    ULONG DISPID;           // Display Driver Identification Register (VC4HVS_DISPID)
    ULONG DISPECTRL;        // Display Driver Extended Control Register
    ULONG DISPPROF;         // Display Driver Profiling Counter
    ULONG DISPDITHER;       // Display Dither and OLED Control
    ULONG DISPEOLN;         // Display End-of-Line-N Counters
    ULONG reserved1;
    ULONG DISPLIST0;        // Display List 0 Head Pointer
    ULONG DISPLIST1;        // Display List 1 Head Pointer
    ULONG DISPLIST2;        // Display List 2 Head Pointer
    ULONG DISPLSTAT;        // Display List Status
    ULONG DISPLACT0;        // Active Display List 0 Head Pointer
    ULONG DISPLACT1;        // Active Display List 1 Head Pointer
    ULONG DISPLACT2;        // Active Display List 2 Head Pointer
    ULONG reserved2;
    ULONG DISPCTRL0;        // Display FIFO 0 Control
    ULONG DISPBKGND0;       // Display FIFO 0 Background Color
    ULONG DISPSTAT0;        // Display FIFO 0 Status
    ULONG DISPBASE0;        // Display FIFO 0 COB Base Addresses
    ULONG DISPCTRL1;        // Display FIFO 1 Control
    ULONG DISPBKGND1;       // Display FIFO 1 Background Color
    ULONG DISPSTAT1;        // Display FIFO 1 Status
    ULONG DISPBASE1;        // Display FIFO 1 COB Base Addresses
    ULONG DISPCTRL2;        // Display FIFO 2 Control
    ULONG DISPBKGND2;       // Display FIFO 2 Background Color
    ULONG DISPSTAT2;        // Display FIFO 2 Status
    ULONG DISPBASE2;        // Display FIFO 2 COB Base Addresses
    ULONG DISPALPHA2;       // Display FIFO 2 Background Alpha
    ULONG reserved3;
    ULONG GAMADDR;          // Gamma Mapping Access Addresses
    ULONG reserved4;
    ULONG OLEDOFFS;         // OLED Matrix Offsets and Control
    ULONG OLEDCOEF0;        // OLED Matrix Coefficients for Row 0
    ULONG OLEDCOEF1;        // OLED Matrix Coefficients for Row 1
    ULONG OLEDCOEF2;        // OLED Matrix Coefficients for Row 2
    ULONG reserved5[12];

    // The following registers are defined in the datasheet but produce a
    // bus error when accessed
    // ULONG DISPSLAVE0;       // Display FIFO 0 Slave Access
    // ULONG reserved6;
    // ULONG DISPSLAVE1;       // Display FIFO 1 Slave Access
    // ULONG reserved7;
    // ULONG DISPSLAVE2;       // Display FIFO 2 Slave Access
    // ULONG reserved8[3];
    ULONG reserved6[8];
    ULONG GAMDATA0;         // Gamma Mapping Access Read/Write Data
    ULONG GAMDATA1;         // Gamma Mapping Access Read/Write Data
    ULONG GAMDATA2;         // Gamma Mapping Access Read/Write Data
    ULONG GAMDATA3;         // Gamma Mapping Access Read/Write Data

    ULONG reserved9[0x7C4];

    ULONG DLISTMEM[0x1000]; // Display list context memory
};

static_assert(
    sizeof(VC4HVS_REGISTERS) == 0x6000,
    "Sanity check on size of HVS register block");

static_assert(
    FIELD_OFFSET(VC4HVS_REGISTERS, GAMDATA3) == 0xec,
    "Sanity check for GAMDATA3 register offset");

static_assert(
    FIELD_OFFSET(VC4HVS_REGISTERS, DLISTMEM) == 0x2000,
    "Sanity check for DLISTMEM offset");

union VC4HVS_DLIST_ENTRY_UNITY {
    struct {
        VC4HVS_DLIST_CONTROL_WORD_0 ControlWord0;       // Element size, source image type, scaling options
        VC4HVS_DLIST_POSITION_WORD_0 PositionWord0;     // Start (x, y) and alpha
        VC4HVS_DLIST_POSITION_WORD_2 PositionWord2;     // Source size (w, h) and alpha mode
        VC4HVS_DLIST_POSITION_WORD_3 PositionWord3;     // Context (updated by scaler hardware)
        ULONG PointerWord0;                             // Source image pointer
        ULONG PointerContextWord0;                      // Stores context related to source iamge
        VC4HVS_DLIST_PITCH_WORD_0 PitchWord0;           // Source image
    } DUMMYSTRUCTNAME;
    ULONG AsUlong[7];
};

static_assert(
    sizeof(VC4HVS_DLIST_ENTRY_UNITY) == 28,
    "Sanity check for size of unity display list entry");

#include <poppack.h> //=======================================================

#pragma warning(default:4201) // nameless struct/union

#endif // _VC4HVS_HPP_
