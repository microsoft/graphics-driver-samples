#ifndef _VC4DEBUG_HPP_
#define  _VC4DEBUG_HPP_ 1
//
// Copyright (C) Microsoft.  All rights reserved.
//
//
// Module Name:
//
//  Vc4Debug.h
//
// Abstract:
//
//    Debugging utilities for vc4dod
//
// Author:
//
//    Jordan Rhee (jordanrh) November 2015
//
// Environment:
//
//    Kernel mode only.
//

struct VC4HVS_REGISTERS;
struct VC4PIXELVALVE_REGISTERS;

struct VC4_DEBUG {
public: // NONPAGED

    __forceinline VC4_DEBUG (const DXGKRNL_INTERFACE& DxgkInterface) :
        dxgkInterface(DxgkInterface) {}

private: // NONPAGED

    const DXGKRNL_INTERFACE& dxgkInterface;

    VC4_DEBUG (const VC4_DEBUG&) = delete;
    VC4_DEBUG& operator= (const VC4_DEBUG&) = delete;

public: // PAGED

    _IRQL_requires_max_(PASSIVE_LEVEL)
    static void DumpMonitorModes (
        const DXGKARG_RECOMMENDMONITORMODES* MonitorModesPtr
        );

    _IRQL_requires_max_(PASSIVE_LEVEL)
    static void DumpSourceModeSet (
        D3DKMDT_HVIDPN VidPnHandle,
        const DXGK_VIDPN_INTERFACE* VidPnInterfacePtr,
        D3DKMDT_VIDEO_PRESENT_SOURCE_MODE_ID Id
        );

    _IRQL_requires_max_(PASSIVE_LEVEL)
    static void DumpTargetModeSet (
        D3DKMDT_HVIDPN VidPnHandle,
        const DXGK_VIDPN_INTERFACE* VidPnInterfacePtr,
        D3DKMDT_VIDEO_PRESENT_TARGET_MODE_ID Id
        );

    _IRQL_requires_max_(PASSIVE_LEVEL)
    void DumpVidPn (D3DKMDT_HVIDPN VidPnHandle);

    _IRQL_requires_max_(PASSIVE_LEVEL)
    void DumpHvsRegisters (VC4HVS_REGISTERS* RegistersPtr);

    _IRQL_requires_max_(PASSIVE_LEVEL)
    void DumpPixelValveRegisters (VC4PIXELVALVE_REGISTERS* RegistersPtr);

private: // PAGED

};

#endif // _VC4DEBUG_HPP_
