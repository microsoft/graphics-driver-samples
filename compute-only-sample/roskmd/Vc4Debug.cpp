//
// Copyright (C) Microsoft. All rights reserved.
//

#include "precomp.h"

#include "RosKmdLogging.h"
#include "Vc4Debug.tmh"

#include "RosKmdUtil.h"
#include "Vc4Hvs.h"
#include "Vc4PixelValve.h"
#include "Vc4Debug.h"

ROS_NONPAGED_SEGMENT_BEGIN; //================================================

ROS_NONPAGED_SEGMENT_END; //==================================================
ROS_PAGED_SEGMENT_BEGIN; //===================================================

_Use_decl_annotations_
void VC4_DEBUG::DumpMonitorModes (
    const DXGKARG_RECOMMENDMONITORMODES* MonitorModesPtr
    )
{
    const auto& tbl = *MonitorModesPtr->pMonitorSourceModeSetInterface;

    const D3DKMDT_MONITOR_SOURCE_MODE* sourceModePtr;
    NTSTATUS status = tbl.pfnAcquireFirstModeInfo(
            MonitorModesPtr->hMonitorSourceModeSet,
            &sourceModePtr);
    if (!NT_SUCCESS(status)) {
        ROS_LOG_ERROR(
            "pfnAcquireFirstModeInfo() failed. (status = %!STATUS!, MonitorModesPtr->hMonitorSourceModeSet = %p)",
            status,
            MonitorModesPtr->hMonitorSourceModeSet);
        return;
    }

    while (status == STATUS_SUCCESS) {
        NT_ASSERT(sourceModePtr);
        ROS_LOG_TRACE(
            "Dumping monitor source mode. (Id = %d, ColorBasis = %d, Origin = %d, Preference = %d)",
            sourceModePtr->Id,
            sourceModePtr->ColorBasis,
            sourceModePtr->Origin,
            sourceModePtr->Preference);

        ROS_LOG_TRACE(
            "VideoSignalInfo: VideoStandard = %d, TotalSize = (%d,%d), ActiveSize = (%d,%d), VSyncFreq = %d, HSyncFreq = %d, PixelRate = %Id",
            sourceModePtr->VideoSignalInfo.VideoStandard,
            sourceModePtr->VideoSignalInfo.TotalSize.cx,
            sourceModePtr->VideoSignalInfo.TotalSize.cy,
            sourceModePtr->VideoSignalInfo.ActiveSize.cx,
            sourceModePtr->VideoSignalInfo.ActiveSize.cy,
            sourceModePtr->VideoSignalInfo.VSyncFreq.Numerator /
                sourceModePtr->VideoSignalInfo.VSyncFreq.Denominator,
            sourceModePtr->VideoSignalInfo.HSyncFreq.Numerator /
                sourceModePtr->VideoSignalInfo.HSyncFreq.Denominator,
            sourceModePtr->VideoSignalInfo.PixelRate);

        // get next source mode
        const D3DKMDT_MONITOR_SOURCE_MODE* nextSourceModePtr;
        status = tbl.pfnAcquireNextModeInfo(
                MonitorModesPtr->hMonitorSourceModeSet,
                sourceModePtr,
                &nextSourceModePtr);

        // release current source mode
        {
            NTSTATUS releaseStatus = tbl.pfnReleaseModeInfo(
                    MonitorModesPtr->hMonitorSourceModeSet,
                    sourceModePtr);
            UNREFERENCED_PARAMETER(releaseStatus);
            NT_ASSERT(NT_SUCCESS(releaseStatus));
        }

        if (!NT_SUCCESS(status)) {
            ROS_LOG_ERROR(
                "pfnAcquireNextModeInfo() failed. (status = %!STATUS!, MonitorModesPtr->hMonitorSourceModeSet = %p)",
                status,
                MonitorModesPtr->hMonitorSourceModeSet);
            return;
        }

        sourceModePtr = nextSourceModePtr;
    }
}

_Use_decl_annotations_
void VC4_DEBUG::DumpSourceModeSet (
    D3DKMDT_HVIDPN VidPnHandle,
    const DXGK_VIDPN_INTERFACE* VidPnInterfacePtr,
    D3DKMDT_VIDEO_PRESENT_SOURCE_MODE_ID Id
    )
{
    PAGED_CODE();
    ROS_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    // Get source mode set handle
    D3DKMDT_HVIDPNSOURCEMODESET sourceModeSetHandle;
    const DXGK_VIDPNSOURCEMODESET_INTERFACE* smsInterfacePtr;
    NTSTATUS status = VidPnInterfacePtr->pfnAcquireSourceModeSet(
            VidPnHandle,
            Id,
            &sourceModeSetHandle,
            &smsInterfacePtr);
    if (!NT_SUCCESS(status)) {
        ROS_LOG_ASSERTION(
            "vidPnInterfacePtr->pfnAcquireSourceModeSet failed. (status = %!STATUS!)",
            status);
        return;
    }
    auto releaseSms = ROS_FINALLY::Do([&] () {
        PAGED_CODE();
        NTSTATUS releaseStatus =
            VidPnInterfacePtr->pfnReleaseSourceModeSet(
                VidPnHandle,
                sourceModeSetHandle);
        UNREFERENCED_PARAMETER(releaseStatus);
        NT_ASSERT(NT_SUCCESS(releaseStatus));
    });

    const D3DKMDT_VIDPN_SOURCE_MODE* modeInfo;
    status = smsInterfacePtr->pfnAcquireFirstModeInfo(
            sourceModeSetHandle,
            &modeInfo);
    if (!NT_SUCCESS(status)) {
        ROS_LOG_ASSERTION(
            "smsInterfacePtr->pfnAcquireFirstModeInfo() failed. (status = %!STATUS!)",
            status);
        return;
    }

    DbgPrintEx(
        DPFLTR_IHVVIDEO_ID,
        DPFLTR_TRACE_LEVEL,
        "Dumping source mode set for Source Mode Id = %d\n",
        Id);
    while (status == STATUS_SUCCESS) {
        NT_ASSERT(modeInfo);
        auto releaseModeInfo = ROS_FINALLY::Do([&, modeInfo] () {
            PAGED_CODE();
            NTSTATUS releaseStatus =
                smsInterfacePtr->pfnReleaseModeInfo(
                    sourceModeSetHandle,
                    modeInfo);
            UNREFERENCED_PARAMETER(releaseStatus);
            NT_ASSERT(NT_SUCCESS(releaseStatus));
        });

        // dump mode info here
        switch (modeInfo->Type) {
        case D3DKMDT_RMT_GRAPHICS:
            DbgPrintEx(
                DPFLTR_IHVVIDEO_ID,
                DPFLTR_TRACE_LEVEL,
                "  modeInfo->Id = %d (Graphics)\n"
                "    PrimSurfSize = %d, %d\n"
                "    VisibleRegionSize = %d, %d\n"
                "    Stride = %d\n"
                "    PixelFormat = %d\n"
                "    ColorBasis = %d\n"
                "    PixelValueAccessMode = %d\n",
                modeInfo->Id,
                modeInfo->Format.Graphics.PrimSurfSize.cx,
                modeInfo->Format.Graphics.PrimSurfSize.cy,
                modeInfo->Format.Graphics.VisibleRegionSize.cx,
                modeInfo->Format.Graphics.VisibleRegionSize.cy,
                modeInfo->Format.Graphics.Stride,
                modeInfo->Format.Graphics.PixelFormat,
                modeInfo->Format.Graphics.ColorBasis,
                modeInfo->Format.Graphics.PixelValueAccessMode);
            break;
        case D3DKMDT_RMT_TEXT:
            DbgPrintEx(
                DPFLTR_IHVVIDEO_ID,
                DPFLTR_TRACE_LEVEL,
                "modeInfo->Id = %d (Text)\n",
                modeInfo->Id);
            break;
        default:
            NT_ASSERT(!"Invalid D3DKMDT_VIDPN_SOURCE_MODE_TYPE value");
        }

        const D3DKMDT_VIDPN_SOURCE_MODE* nextModeInfo;
        status = smsInterfacePtr->pfnAcquireNextModeInfo(
                sourceModeSetHandle,
                modeInfo,
                &nextModeInfo);
        if (!NT_SUCCESS(status)) {
            ROS_LOG_ASSERTION(
                "smsInterfacePtr->pfnAcquireNextModeInfo() failed. (status = %!STATUS!)",
                status);
            return;
        }

        modeInfo = nextModeInfo;
    }
}

_Use_decl_annotations_
void VC4_DEBUG::DumpTargetModeSet (
    D3DKMDT_HVIDPN VidPnHandle,
    const DXGK_VIDPN_INTERFACE* VidPnInterfacePtr,
    D3DKMDT_VIDEO_PRESENT_TARGET_MODE_ID Id
    )
{
    PAGED_CODE();
    ROS_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    // Get source mode set handle
    D3DKMDT_HVIDPNTARGETMODESET targetModeSetHandle;
    const DXGK_VIDPNTARGETMODESET_INTERFACE* tmsInterfacePtr;
    NTSTATUS status = VidPnInterfacePtr->pfnAcquireTargetModeSet(
            VidPnHandle,
            Id,
            &targetModeSetHandle,
            &tmsInterfacePtr);
    if (!NT_SUCCESS(status)) {
        ROS_LOG_ASSERTION(
            "VidPnInterfacePtr->pfnAcquireTargetModeSet() failed. (status = %!STATUS!)",
            status);
        return;
    }
    auto releaseTms = ROS_FINALLY::Do([&] () {
        PAGED_CODE();
        NTSTATUS releaseStatus =
            VidPnInterfacePtr->pfnReleaseTargetModeSet(
                VidPnHandle,
                targetModeSetHandle);
        UNREFERENCED_PARAMETER(releaseStatus);
        NT_ASSERT(NT_SUCCESS(releaseStatus));
    });

    const D3DKMDT_VIDPN_TARGET_MODE* modeInfo;
    status = tmsInterfacePtr->pfnAcquireFirstModeInfo(
            targetModeSetHandle,
            &modeInfo);
    if (!NT_SUCCESS(status)) {
        ROS_LOG_ASSERTION(
            "tmsInterfacePtr->pfnAcquireFirstModeInfo() failed. (status = %!STATUS!)",
            status);
        return;
    }

    DbgPrintEx(
        DPFLTR_IHVVIDEO_ID,
        DPFLTR_TRACE_LEVEL,
        "Dumping target mode set for Target Mode Id = %d\n",
        Id);
    while (status == STATUS_SUCCESS) {
        NT_ASSERT(modeInfo);
        auto releaseModeInfo = ROS_FINALLY::Do([&, modeInfo] () {
            PAGED_CODE();
            NTSTATUS releaseStatus =
                tmsInterfacePtr->pfnReleaseModeInfo(
                    targetModeSetHandle,
                    modeInfo);
            UNREFERENCED_PARAMETER(releaseStatus);
            NT_ASSERT(NT_SUCCESS(releaseStatus));
        });

        // dump mode info here
        DbgPrintEx(
            DPFLTR_IHVVIDEO_ID,
            DPFLTR_TRACE_LEVEL,
            "  modeInfo->Id = %d\n"
            "    VideoSignalInfo.VideoStandard = %d\n"
            "    VideoSignalInfo.TotalSize = %d, %d\n"
            "    VideoSignalInfo.ActiveSize = %d, %d\n"
            "    VideoSignalInfo.VSyncFreq = %d\n"
            "    VideoSignalInfo.HSyncFreq = %d\n"
            "    VideoSignalInfo.PixelRate = %Id\n",
            modeInfo->Id,
            modeInfo->VideoSignalInfo.VideoStandard,
            modeInfo->VideoSignalInfo.TotalSize.cx,
            modeInfo->VideoSignalInfo.TotalSize.cy,
            modeInfo->VideoSignalInfo.ActiveSize.cx,
            modeInfo->VideoSignalInfo.ActiveSize.cy,
            modeInfo->VideoSignalInfo.VSyncFreq.Numerator /
                modeInfo->VideoSignalInfo.VSyncFreq.Denominator,
            modeInfo->VideoSignalInfo.HSyncFreq.Numerator /
                modeInfo->VideoSignalInfo.HSyncFreq.Denominator,
            modeInfo->VideoSignalInfo.PixelRate);

        const D3DKMDT_VIDPN_TARGET_MODE* nextModeInfo;
        status = tmsInterfacePtr->pfnAcquireNextModeInfo(
                targetModeSetHandle,
                modeInfo,
                &nextModeInfo);
        if (!NT_SUCCESS(status)) {
            ROS_LOG_ASSERTION(
                "tmsInterfacePtr->pfnAcquireNextModeInfo() failed. (status = %!STATUS!)",
                status);
            return;
        }

        modeInfo = nextModeInfo;
    }
}

//
// To see the output in the debugger, break in and run the following command:
// ed nt!Kd_IHVVIDEO_Mask 0xffffffff
//
_Use_decl_annotations_
void VC4_DEBUG::DumpVidPn (D3DKMDT_HVIDPN VidPnHandle)
{
    PAGED_CODE();
    ROS_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    //
    // a VidPnHandle has exactly one topology
    // a topology consists of a set of paths from sources to targets
    // Each source and target has a set of supported modes, of which one is pinned (preferred)
    //

    DbgPrintEx(
        DPFLTR_IHVVIDEO_ID,
        DPFLTR_TRACE_LEVEL,
        "Dumping VidPn %p\n",
        VidPnHandle);

    const DXGK_VIDPN_INTERFACE* vidPnInterfacePtr;
    NTSTATUS status = this->dxgkInterface.DxgkCbQueryVidPnInterface(
            VidPnHandle,
            DXGK_VIDPN_INTERFACE_VERSION_V1,
            &vidPnInterfacePtr);
    if (!NT_SUCCESS(status)) {
        ROS_LOG_ASSERTION(
            "DxgkCbQueryVidPnInterface() failed. (status = %!STATUS!, VidPnHandle = %p, this = %p)",
            status,
            VidPnHandle,
            this);
        return;
    }

    D3DKMDT_HVIDPNTOPOLOGY vidPnTopologyHandle;
    const DXGK_VIDPNTOPOLOGY_INTERFACE* topologyInterfacePtr;
    status = vidPnInterfacePtr->pfnGetTopology(
        VidPnHandle,
        &vidPnTopologyHandle,
        &topologyInterfacePtr);
    if (!NT_SUCCESS(status)) {
        ROS_LOG_ASSERTION(
            "pfnGetTopology() failed. (status = %!STATUS!, VidPnHandle = %p, vidPnInterfacePtr = %p)",
            status,
            VidPnHandle,
            vidPnInterfacePtr);
        return;
    }

    // iterate through each path in the topology
    const D3DKMDT_VIDPN_PRESENT_PATH* presentPathPtr;
    status = topologyInterfacePtr->pfnAcquireFirstPathInfo(
            vidPnTopologyHandle,
            &presentPathPtr);
    if (!NT_SUCCESS(status)) {
        ROS_LOG_ASSERTION(
            "Failed to get first path info from vidpn topology. (status = %!STATUS!, VidPnHandle = %p, vidPnTopologyHandle = %p)",
            status,
            VidPnHandle,
            vidPnTopologyHandle);
        return;
    }

    ULONG pathIndex = 0;
    while (status == STATUS_SUCCESS) {
        NT_ASSERT(presentPathPtr);
        auto releasePathInfo = ROS_FINALLY::Do([&, presentPathPtr] () {
            PAGED_CODE();
            NTSTATUS releaseStatus = topologyInterfacePtr->pfnReleasePathInfo(
                    vidPnTopologyHandle,
                    presentPathPtr);
            UNREFERENCED_PARAMETER(releaseStatus);
            NT_ASSERT(NT_SUCCESS(releaseStatus));
        });

        // dump the path structure. Use DbgPrintEx so that we get correct
        // formatting
        DbgPrintEx(
            DPFLTR_IHVVIDEO_ID,
            DPFLTR_TRACE_LEVEL,
            "Dumping path %d for topology %p\n"
            "VidPnSourceId = %d, VidPnTargetId = %d\n"
            "  ImportanceOrdinal = %d\n"
            "  ContentTransformation.Scaling = %d\n"
            "  ContentTransformation.Rotation = %d\n"
            "  VisibleFormActiveTLOffset = %d, %d\n"
            "  VisibleFromActiveBROffset = %d, %d\n"
            "  VidPnTargetColorBasis = %d\n"
            "  VidPnTargetColorCoeffDynamicRanges = %d, %d, %d, %d\n"
            "  Content = %d\n"
            "  CopyProtection.CopyProtectionType = %d\n"
            "  GammaRamp.Type = %d\n",
            pathIndex,
            vidPnTopologyHandle,
            presentPathPtr->VidPnSourceId,
            presentPathPtr->VidPnTargetId,
            presentPathPtr->ImportanceOrdinal,
            presentPathPtr->ContentTransformation.Scaling,
            presentPathPtr->ContentTransformation.Rotation,
            presentPathPtr->VisibleFromActiveTLOffset.cx,
            presentPathPtr->VisibleFromActiveTLOffset.cy,
            presentPathPtr->VisibleFromActiveBROffset.cx,
            presentPathPtr->VisibleFromActiveBROffset.cy,
            presentPathPtr->VidPnTargetColorBasis,
            presentPathPtr->VidPnTargetColorCoeffDynamicRanges.FirstChannel,
            presentPathPtr->VidPnTargetColorCoeffDynamicRanges.SecondChannel,
            presentPathPtr->VidPnTargetColorCoeffDynamicRanges.ThirdChannel,
            presentPathPtr->VidPnTargetColorCoeffDynamicRanges.FourthChannel,
            presentPathPtr->Content,
            presentPathPtr->CopyProtection.CopyProtectionType,
            presentPathPtr->GammaRamp.Type);

        this->DumpSourceModeSet(
            VidPnHandle,
            vidPnInterfacePtr,
            presentPathPtr->VidPnSourceId);

        this->DumpTargetModeSet(
            VidPnHandle,
            vidPnInterfacePtr,
            presentPathPtr->VidPnTargetId);

        const D3DKMDT_VIDPN_PRESENT_PATH* nextPresentPathPtr;
        status = topologyInterfacePtr->pfnAcquireNextPathInfo(
            vidPnTopologyHandle,
            presentPathPtr,
            &nextPresentPathPtr);
        if (!NT_SUCCESS(status)) {
            ROS_LOG_ASSERTION(
                "pfnAcquireNextPathInfo() failed. (status = %!STATUS!, VidPnHandle = %p, vidPnTopologyHandle = %p)",
                status,
                VidPnHandle,
                vidPnTopologyHandle);
            return;
        }

        presentPathPtr = nextPresentPathPtr;
        ++pathIndex;
    }
}

_Use_decl_annotations_
void VC4_DEBUG::DumpHvsRegisters (VC4HVS_REGISTERS* RegistersPtr)
{
    PAGED_CODE();
    ROS_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    // Read the registers into a local copy and use the debugger
    // to interpret the bitfields
    struct {
        VC4HVS_DISPCTRL DISPCTRL;
        VC4HVS_DISPSTAT DISPSTAT;
        ULONG DISPID;
        VC4HVS_DISPECTRL DISPECTRL;
        VC4HVS_DISPPROF DISPPROF;
        VC4HVS_DISPDITHER DISPDITHER;
        VC4HVS_DISPEOLN DISPEOLN;
        VC4HVS_DISPLIST DISPLIST0;
        VC4HVS_DISPLIST DISPLIST1;
        VC4HVS_DISPLIST DISPLIST2;
        VC4HVS_DISPLSTAT DISPLSTAT;
        VC4HVS_DISPLACT DISPLACT0;
        VC4HVS_DISPLACT DISPLACT1;
        VC4HVS_DISPLACT DISPLACT2;
        VC4HVS_DISPFIFOCTRL DISPCTRL0;
        VC4HVS_DISPBKGND DISPBKGND0;
        VC4HVS_DISPFIFOSTAT DISPSTAT0;
        VC4HVS_DISPFIFOBASE DISPBASE0;
        VC4HVS_DISPFIFOCTRL DISPCTRL1;
        VC4HVS_DISPBKGND DISPBKGND1;
        VC4HVS_DISPFIFOSTAT DISPSTAT1;
        VC4HVS_DISPFIFOBASE DISPBASE1;
        VC4HVS_DISPFIFOCTRL DISPCTRL2;
        VC4HVS_DISPBKGND DISPBKGND2;
        VC4HVS_DISPFIFOSTAT DISPSTAT2;
        VC4HVS_DISPFIFOBASE DISPBASE2;
        VC4HVS_DISPALPHA2 DISPALPHA2;
        VC4HVS_GAMADDR GAMADDR;
    } registers;

    registers.DISPCTRL.AsUlong = READ_REGISTER_NOFENCE_ULONG(&RegistersPtr->DISPCTRL);
    registers.DISPSTAT.AsUlong = READ_REGISTER_NOFENCE_ULONG(&RegistersPtr->DISPSTAT);
    registers.DISPID = READ_REGISTER_NOFENCE_ULONG(&RegistersPtr->DISPID);
    registers.DISPECTRL.AsUlong = READ_REGISTER_NOFENCE_ULONG(&RegistersPtr->DISPECTRL);
    registers.DISPPROF.AsUlong = READ_REGISTER_NOFENCE_ULONG(&RegistersPtr->DISPPROF);
    registers.DISPDITHER.AsUlong = READ_REGISTER_NOFENCE_ULONG(&RegistersPtr->DISPDITHER);
    registers.DISPEOLN.AsUlong = READ_REGISTER_NOFENCE_ULONG(&RegistersPtr->DISPEOLN);
    registers.DISPLIST0.AsUlong = READ_REGISTER_NOFENCE_ULONG(&RegistersPtr->DISPLIST0);
    registers.DISPLIST1.AsUlong = READ_REGISTER_NOFENCE_ULONG(&RegistersPtr->DISPLIST1);
    registers.DISPLIST2.AsUlong = READ_REGISTER_NOFENCE_ULONG(&RegistersPtr->DISPLIST2);
    registers.DISPLSTAT.AsUlong = READ_REGISTER_NOFENCE_ULONG(&RegistersPtr->DISPLSTAT);
    registers.DISPLACT0.AsUlong = READ_REGISTER_NOFENCE_ULONG(&RegistersPtr->DISPLACT0);
    registers.DISPLACT1.AsUlong = READ_REGISTER_NOFENCE_ULONG(&RegistersPtr->DISPLACT1);
    registers.DISPLACT2.AsUlong = READ_REGISTER_NOFENCE_ULONG(&RegistersPtr->DISPLACT2);
    registers.DISPCTRL0.AsUlong = READ_REGISTER_NOFENCE_ULONG(&RegistersPtr->DISPCTRL0);
    registers.DISPBKGND0.AsUlong = READ_REGISTER_NOFENCE_ULONG(&RegistersPtr->DISPBKGND0);
    registers.DISPSTAT0.AsUlong = READ_REGISTER_NOFENCE_ULONG(&RegistersPtr->DISPSTAT0);
    registers.DISPBASE0.AsUlong = READ_REGISTER_NOFENCE_ULONG(&RegistersPtr->DISPBASE0);
    registers.DISPCTRL1.AsUlong = READ_REGISTER_NOFENCE_ULONG(&RegistersPtr->DISPCTRL1);
    registers.DISPBKGND1.AsUlong = READ_REGISTER_NOFENCE_ULONG(&RegistersPtr->DISPBKGND1);
    registers.DISPSTAT1.AsUlong = READ_REGISTER_NOFENCE_ULONG(&RegistersPtr->DISPSTAT1);
    registers.DISPBASE1.AsUlong = READ_REGISTER_NOFENCE_ULONG(&RegistersPtr->DISPBASE1);
    registers.DISPCTRL2.AsUlong = READ_REGISTER_NOFENCE_ULONG(&RegistersPtr->DISPCTRL2);
    registers.DISPBKGND2.AsUlong = READ_REGISTER_NOFENCE_ULONG(&RegistersPtr->DISPBKGND2);
    registers.DISPSTAT2.AsUlong = READ_REGISTER_NOFENCE_ULONG(&RegistersPtr->DISPSTAT2);
    registers.DISPBASE2.AsUlong = READ_REGISTER_NOFENCE_ULONG(&RegistersPtr->DISPBASE2);
    registers.DISPALPHA2.AsUlong = READ_REGISTER_NOFENCE_ULONG(&RegistersPtr->DISPALPHA2);
    registers.GAMADDR.AsUlong = READ_REGISTER_NOFENCE_ULONG(&RegistersPtr->GAMADDR);

    // Read context memory into local variable
    ULONG* contextMem = new (PagedPool, ROS_ALLOC_TAG::TEMP) ULONG[0x1000];
    for (ULONG i = 0; i < ARRAYSIZE(RegistersPtr->DLISTMEM); ++i) {
        contextMem[i] = READ_REGISTER_NOFENCE_ULONG(&RegistersPtr->DLISTMEM[i]);
    }
    auto freeContextMem = ROS_FINALLY::Do([&] {
        PAGED_CODE();
        delete[] contextMem;
    });

    auto entryPtr = reinterpret_cast<const VC4HVS_DLIST_ENTRY_UNITY*>(
        &contextMem[registers.DISPLACT1.LACT]);
    UNREFERENCED_PARAMETER(entryPtr);
}

_Use_decl_annotations_
void VC4_DEBUG::DumpPixelValveRegisters (VC4PIXELVALVE_REGISTERS* RegistersPtr)
{
    PAGED_CODE();
    ROS_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    // Read the registers into a local copy and use the debugger
    // to interpret the bitfields
    struct {
        VC4PIXELVALVE_CONTROL Control;
        VC4PIXELVALVE_VCONTROL VControl;
        ULONG VSyncD;
        VC4PIXELVALVE_HORZA HorzA;
        VC4PIXELVALVE_HORZB HorzB;
        VC4PIXELVALVE_VERTA VertA;
        VC4PIXELVALVE_VERTB VertB;
        ULONG VertAEven;
        ULONG VertBEven;
        VC4PIXELVALVE_INTERRUPT IntEn;
        VC4PIXELVALVE_INTERRUPT IntStat;
        ULONG Status;
        ULONG HactAct;
    } registers;

    registers.Control.AsUlong = READ_REGISTER_NOFENCE_ULONG(&RegistersPtr->Control);
    registers.VControl.AsUlong = READ_REGISTER_NOFENCE_ULONG(&RegistersPtr->VControl);
    registers.VSyncD = READ_REGISTER_NOFENCE_ULONG(&RegistersPtr->VSyncD);
    registers.HorzA.AsUlong = READ_REGISTER_NOFENCE_ULONG(&RegistersPtr->HorzA);
    registers.HorzB.AsUlong = READ_REGISTER_NOFENCE_ULONG(&RegistersPtr->HorzB);
    registers.VertA.AsUlong = READ_REGISTER_NOFENCE_ULONG(&RegistersPtr->VertA);
    registers.VertB.AsUlong = READ_REGISTER_NOFENCE_ULONG(&RegistersPtr->VertB);
    registers.VertAEven = READ_REGISTER_NOFENCE_ULONG(&RegistersPtr->VertAEven);
    registers.VertBEven = READ_REGISTER_NOFENCE_ULONG(&RegistersPtr->VertBEven);
    registers.IntEn.AsUlong = READ_REGISTER_NOFENCE_ULONG(&RegistersPtr->IntEn);
    registers.IntStat.AsUlong = READ_REGISTER_NOFENCE_ULONG(&RegistersPtr->IntStat);
    registers.Status = READ_REGISTER_NOFENCE_ULONG(&RegistersPtr->Status);
    registers.HactAct = READ_REGISTER_NOFENCE_ULONG(&RegistersPtr->HactAct);
}

ROS_PAGED_SEGMENT_END; //=====================================================
