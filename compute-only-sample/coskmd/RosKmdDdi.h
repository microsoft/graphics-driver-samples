#pragma once

#include "RosKmd.h"

class RosKmdDdi
{
public:

    static NTSTATUS __stdcall DdiAddAdapter(
        IN_CONST_PDEVICE_OBJECT     PhysicalDeviceObject,
        OUT_PPVOID                  MiniportDeviceContext);

    static NTSTATUS __stdcall DdiStartAdapter(
        IN_CONST_PVOID          MiniportDeviceContext,
        IN_PDXGK_START_INFO     DxgkStartInfo,
        IN_PDXGKRNL_INTERFACE   DxgkInterface,
        OUT_PULONG              NumberOfVideoPresentSources,
        OUT_PULONG              NumberOfChildren);

    static NTSTATUS __stdcall DdiStopAdapter(
        IN_CONST_PVOID  MiniportDeviceContext);

    static NTSTATUS __stdcall DdiRemoveAdapter(
        IN_CONST_PVOID  MiniportDeviceContext);

    static void __stdcall DdiDpcRoutine(
        IN_CONST_PVOID  MiniportDeviceContext);

    static NTSTATUS
        DdiDispatchIoRequest(
            IN_CONST_PVOID              MiniportDeviceContext,
            IN_ULONG                    VidPnSourceId,
            IN_PVIDEO_REQUEST_PACKET    VideoRequestPacket);

    static BOOLEAN
        DdiInterruptRoutine(
            IN_CONST_PVOID  MiniportDeviceContext,
            IN_ULONG        MessageNumber);

    static NTSTATUS __stdcall DdiBuildPagingBuffer(
        IN_CONST_HANDLE                 hAdapter,
        IN_PDXGKARG_BUILDPAGINGBUFFER   pArgs);

    static NTSTATUS __stdcall DdiSubmitCommand(
        IN_CONST_HANDLE                     hAdapter,
        IN_CONST_PDXGKARG_SUBMITCOMMAND     pSubmitCommand);

    static NTSTATUS __stdcall DdiPatch(
        IN_CONST_HANDLE             hAdapter,
        IN_CONST_PDXGKARG_PATCH     pPatch);

    static NTSTATUS __stdcall DdiCreateAllocation(
        IN_CONST_HANDLE                     hAdapter,
        INOUT_PDXGKARG_CREATEALLOCATION     pCreateAllocation);

    static NTSTATUS __stdcall DdiDestroyAllocation(
        IN_CONST_HANDLE                         hAdapter,
        IN_CONST_PDXGKARG_DESTROYALLOCATION     pDestroyAllocation);

    static NTSTATUS __stdcall DdiQueryAdapterInfo(
        IN_CONST_HANDLE                         hAdapter,
        IN_CONST_PDXGKARG_QUERYADAPTERINFO      pQueryAdapterInfo);

    static NTSTATUS __stdcall DdiDescribeAllocation(
        IN_CONST_HANDLE                         hAdapter,
        INOUT_PDXGKARG_DESCRIBEALLOCATION       pDescribeAllocation);

    static NTSTATUS __stdcall DdiGetNodeMetadata(
        IN_CONST_HANDLE                 hAdapter,
        UINT                            NodeOrdinal,
        OUT_PDXGKARG_GETNODEMETADATA    pGetNodeMetadata);

    static NTSTATUS __stdcall DdiGetStandardAllocationDriverData(
        IN_CONST_HANDLE                                 hAdapter,
        INOUT_PDXGKARG_GETSTANDARDALLOCATIONDRIVERDATA  pGetStandardAllocationDriverData);

    static NTSTATUS
        __stdcall
        DdiSubmitCommandVirtual(
            IN_CONST_HANDLE                         hAdapter,
            IN_CONST_PDXGKARG_SUBMITCOMMANDVIRTUAL  pSubmitCommandVirtual);

    static NTSTATUS
        __stdcall
        DdiPreemptCommand(
            IN_CONST_HANDLE                     hAdapter,
            IN_CONST_PDXGKARG_PREEMPTCOMMAND    pPreemptCommand);

    static NTSTATUS
        __stdcall CALLBACK
        DdiRestartFromTimeout(
            IN_CONST_HANDLE     hAdapter);

    static NTSTATUS
        __stdcall
        DdiCancelCommand(
            IN_CONST_HANDLE                 hAdapter,
            IN_CONST_PDXGKARG_CANCELCOMMAND pCancelCommand);

    static NTSTATUS
        __stdcall
        DdiQueryCurrentFence(
            IN_CONST_HANDLE                    hAdapter,
            INOUT_PDXGKARG_QUERYCURRENTFENCE   pCurrentFence);

    static NTSTATUS
        __stdcall
        DdiResetEngine(
            IN_CONST_HANDLE             hAdapter,
            INOUT_PDXGKARG_RESETENGINE  pResetEngine);

    static NTSTATUS
        __stdcall
        DdiQueryEngineStatus(
            IN_CONST_HANDLE                     hAdapter,
            INOUT_PDXGKARG_QUERYENGINESTATUS    pQueryEngineStatus);

    static NTSTATUS
        __stdcall
        DdiControlInterrupt(
            IN_CONST_HANDLE                 hAdapter,
            IN_CONST_DXGK_INTERRUPT_TYPE    InterruptType,
            IN_BOOLEAN                      EnableInterrupt);

    static NTSTATUS
        __stdcall
        DdiCollectDbgInfo(
            IN_CONST_HANDLE                         hAdapter,
            IN_CONST_PDXGKARG_COLLECTDBGINFO        pCollectDbgInfo);

    static NTSTATUS
        __stdcall
        DdiPresent(
            IN_CONST_HANDLE         hContext,
            INOUT_PDXGKARG_PRESENT  pPresent);

    static NTSTATUS
        __stdcall
        DdiCreateProcess(
            IN_PVOID  pMiniportDeviceContext,
            IN DXGKARG_CREATEPROCESS* pArgs);

    static NTSTATUS
        __stdcall
        DdiDestroyProcess(
            IN_PVOID pMiniportDeviceContext,
            IN HANDLE KmdProcessHandle);

    static void
        __stdcall
        DdiSetStablePowerState(
            IN_CONST_HANDLE                        hAdapter,
            IN_CONST_PDXGKARG_SETSTABLEPOWERSTATE  pArgs);

    static NTSTATUS
        __stdcall
        DdiCalibrateGpuClock(
            IN_CONST_HANDLE                             hAdapter,
            IN UINT32                                   NodeOrdinal,
            IN UINT32                                   EngineOrdinal,
            OUT_PDXGKARG_CALIBRATEGPUCLOCK              pClockCalibration
            );

    static NTSTATUS
        __stdcall
        DdiRenderKm(
            IN_CONST_HANDLE         hContext,
            INOUT_PDXGKARG_RENDER   pRender);

    static NTSTATUS
        __stdcall
        DdiEscape(
            IN_CONST_HANDLE                 hAdapter,
            IN_CONST_PDXGKARG_ESCAPE        pEscape);

    static NTSTATUS
        __stdcall
        DdiResetFromTimeout(
            IN_CONST_HANDLE     hAdapter);

    static NTSTATUS
        DdiQueryInterface(
            IN_CONST_PVOID          MiniportDeviceContext,
            IN_PQUERY_INTERFACE     QueryInterface);

    static NTSTATUS
        DdiQueryChildRelations(
            IN_CONST_PVOID                  MiniportDeviceContext,
            INOUT_PDXGK_CHILD_DESCRIPTOR    ChildRelations,
            IN_ULONG                        ChildRelationsSize);

    static NTSTATUS
        DdiQueryChildStatus(
            IN_CONST_PVOID          MiniportDeviceContext,
            IN_PDXGK_CHILD_STATUS   ChildStatus,
            IN_BOOLEAN              NonDestructiveOnly);
    static NTSTATUS
        DdiQueryDeviceDescriptor(
            IN_CONST_PVOID                  MiniportDeviceContext,
            IN_ULONG                        ChildUid,
            INOUT_PDXGK_DEVICE_DESCRIPTOR   pDeviceDescriptor);

    static NTSTATUS
        DdiSetPowerState(
            IN_CONST_PVOID          MiniportDeviceContext,
            IN_ULONG                DeviceUid,
            IN_DEVICE_POWER_STATE   DevicePowerState,
            IN_POWER_ACTION         ActionType);

    static NTSTATUS
        DdiSetPowerComponentFState(
            IN_CONST_PVOID       MiniportDeviceContext,
            IN UINT              ComponentIndex,
            IN UINT              FState);

    static NTSTATUS
        DdiPowerRuntimeControlRequest(
            IN_CONST_PVOID       MiniportDeviceContext,
            IN LPCGUID           PowerControlCode,
            IN OPTIONAL PVOID    InBuffer,
            IN SIZE_T            InBufferSize,
            OUT OPTIONAL PVOID   OutBuffer,
            IN SIZE_T            OutBufferSize,
            OUT OPTIONAL PSIZE_T BytesReturned);

    static NTSTATUS
        DdiNotifyAcpiEvent(
            IN_CONST_PVOID      MiniportDeviceContext,
            IN_DXGK_EVENT_TYPE  EventType,
            IN_ULONG            Event,
            IN_PVOID            Argument,
            OUT_PULONG          AcpiFlags);

    static void
        DdiResetDevice(
            IN_CONST_PVOID  MiniportDeviceContext);


public: // PAGED

    static DXGKDDI_OPENALLOCATIONINFO DdiOpenAllocation;
    static DXGKDDI_QUERYDEPENDENTENGINEGROUP DdiQueryDependentEngineGroup;

};

//
// DDIs that don't have to be registered in render only mode.
//
class RosKmdDisplayDdi
{
public:  // NONPAGED

    static DXGKDDI_SETVIDPNSOURCEADDRESS DdiSetVidPnSourceAddress;

private: // NONPAGED
public: // PAGED

    static DXGKDDI_SETPALETTE DdiSetPalette;
    static DXGKDDI_SETPOINTERPOSITION DdiSetPointerPosition;
    static DXGKDDI_SETPOINTERSHAPE DdiSetPointerShape;

    static DXGKDDI_ISSUPPORTEDVIDPN DdiIsSupportedVidPn;
    static DXGKDDI_RECOMMENDFUNCTIONALVIDPN DdiRecommendFunctionalVidPn;
    static DXGKDDI_ENUMVIDPNCOFUNCMODALITY DdiEnumVidPnCofuncModality;
    static DXGKDDI_SETVIDPNSOURCEVISIBILITY DdiSetVidPnSourceVisibility;
    static DXGKDDI_COMMITVIDPN DdiCommitVidPn;
    static DXGKDDI_UPDATEACTIVEVIDPNPRESENTPATH DdiUpdateActiveVidPnPresentPath;

    static DXGKDDI_RECOMMENDMONITORMODES DdiRecommendMonitorModes;
    static DXGKDDI_GETSCANLINE DdiGetScanLine;
    static DXGKDDI_QUERYVIDPNHWCAPABILITY DdiQueryVidPnHWCapability;
    static DXGKDDI_STOP_DEVICE_AND_RELEASE_POST_DISPLAY_OWNERSHIP
        DdiStopDeviceAndReleasePostDisplayOwnership;

private: // PAGED

};

