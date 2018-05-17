#ifndef _VC4PIXELVALVE_HPP_
#define  _VC4PIXELVALVE_HPP_ 1
//
// Copyright (C) Microsoft.  All rights reserved.
//
//
// Module Name:
//
//  Vc4PixelValve.h
//
// Abstract:
//
//    VC4 PixelValve (PV) Register Definitions
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

#include <pshpack4.h> //======================================================

enum VC4PIXELVALVE_CONTROL_CLOCK_SELECT : ULONG {
    VC4PIXELVALVE_CONTROL_CLOCK_SELECT_DSI_VEC,
    VC4PIXELVALVE_CONTROL_CLOCK_SELECT_DPI_SMI_HDMI,
};

enum VC4PIXELVALVE_CONTROL_FORMAT : ULONG {
    VC4PIXELVALVE_CONTROL_FORMAT_24,
    VC4PIXELVALVE_CONTROL_FORMAT_DSIV_16,
    VC4PIXELVALVE_CONTROL_FORMAT_DSIC_16,
    VC4PIXELVALVE_CONTROL_FORMAT_DSIV_18,
    VC4PIXELVALVE_CONTROL_FORMAT_DSIV_24,
};

union VC4PIXELVALVE_CONTROL {
    ULONG AsUlong;
    struct {
        // LSB
        ULONG Enable : 1;
        ULONG FifoClr : 1;
        VC4PIXELVALVE_CONTROL_CLOCK_SELECT ClockSelect : 2;
        ULONG reserved1 : 8;
        ULONG WaitHStart : 1;
        ULONG TriggerUnderflow : 1;
        ULONG ClearAtStart : 1;
        ULONG FifoLevel : 6;
        VC4PIXELVALVE_CONTROL_FORMAT Format : 3;
        ULONG reserved2: 8;
        // MSB
    } DUMMYSTRUCTNAME;
};

static_assert(
    sizeof(VC4PIXELVALVE_CONTROL) == sizeof(ULONG),
    "Sanity check on size of VC4PIXELVALVE_CONTROL");

union VC4PIXELVALVE_VCONTROL {
    ULONG AsUlong;
    struct {
        // LSB
        ULONG VidEn : 1;
        ULONG Continuous : 1;
        ULONG reserved1 : 2;
        ULONG Interlace : 1;
        ULONG reserved2 : 27;
        // MSB
    } DUMMYSTRUCTNAME;
};

static_assert(
    sizeof(VC4PIXELVALVE_VCONTROL) == sizeof(ULONG),
    "Sanity check on size of VC4PIXELVALVE_VCONTROL");

union VC4PIXELVALVE_HORZA {
    ULONG AsUlong;
    struct {
        // LSB
        ULONG HSync : 16;
        ULONG Hbp : 16;
        // MSB
    } DUMMYSTRUCTNAME;
};

static_assert(
    sizeof(VC4PIXELVALVE_HORZA) == sizeof(ULONG),
    "Sanity check on size of VC4PIXELVALVE_HORZA");

union VC4PIXELVALVE_HORZB {
    ULONG AsUlong;
    struct {
        // LSB
        ULONG HActive : 16;
        ULONG Hfp : 16;
        // MSB
    } DUMMYSTRUCTNAME;
};

static_assert(
    sizeof(VC4PIXELVALVE_HORZB) == sizeof(ULONG),
    "Sanity check on size of VC4PIXELVALVE_HORZB");

union VC4PIXELVALVE_VERTA {
    ULONG AsUlong;
    struct {
        // LSB
        ULONG VSync : 16;
        ULONG Vbp : 16;
        // MSB
    } DUMMYSTRUCTNAME;
};

static_assert(
    sizeof(VC4PIXELVALVE_VERTA) == sizeof(ULONG),
    "Sanity check on size of VC4PIXELVALVE_VERTA");

union VC4PIXELVALVE_VERTB {
    ULONG AsUlong;
    struct {
        // LSB
        ULONG VActive : 16;
        ULONG Vfp : 16;
        // MSB
    } DUMMYSTRUCTNAME;
};

static_assert(
    sizeof(VC4PIXELVALVE_VERTB) == sizeof(ULONG),
    "Sanity check on size of VC4PIXELVALVE_VERTB");

union VC4PIXELVALVE_INTERRUPT {
    ULONG AsUlong;
    struct {
        // LSB
        ULONG HSyncStart : 1;
        ULONG HbpStart : 1;
        ULONG HActStart : 1;
        ULONG HfpStart : 1;
        ULONG VSyncStart : 1;
        ULONG VbpStart : 1;
        ULONG VActStart : 1;
        ULONG VfpStart : 1;
        ULONG VfpEnd : 1;
        ULONG VidIdle : 1;
        ULONG reserved : 22;
        // MSB
    } DUMMYSTRUCTNAME;
};

static_assert(
    sizeof(VC4PIXELVALVE_VERTB) == sizeof(ULONG),
    "Sanity check on size of VC4PIXELVALVE_VERTB");

struct VC4PIXELVALVE_REGISTERS {
    ULONG Control;
    ULONG VControl;
    ULONG VSyncD;
    ULONG HorzA;
    ULONG HorzB;
    ULONG VertA;
    ULONG VertB;
    ULONG VertAEven;
    ULONG VertBEven;
    ULONG IntEn;
    ULONG IntStat;
    ULONG Status;
    ULONG HactAct;
};

static_assert(
    FIELD_OFFSET(VC4PIXELVALVE_REGISTERS, HactAct) == 0x30,
    "Sanity check on PixelValve structure offsets");

#include <poppack.h> //=======================================================

#pragma warning(default:4201) // nameless struct/union

#endif // _VC4PIXELVALVE_HPP_
