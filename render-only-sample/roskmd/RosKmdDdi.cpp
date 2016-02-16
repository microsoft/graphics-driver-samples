#include "precomp.h"

#include "RosKmdLogging.h"
#include "RosKmdDdi.tmh"

#include "RosKmd.h"
#include "RosKmdAdapter.h"
#include "RosKmdDevice.h"
#include "RosKmdAllocation.h"
#include "RosKmdDdi.h"
#include "RosKmdContext.h"
#include "RosKmdResource.h"
#include "RosKmdUtil.h"



// TODO(bhouse) RosKmdPnpDispatch appears to be unused
NTSTATUS
RosKmdPnpDispatch(
    __in struct _DEVICE_OBJECT *DeviceObject,
    __inout struct _IRP *pIrp)
{
    DeviceObject;
    pIrp;

    return STATUS_NOT_SUPPORTED;
}


NTSTATUS __stdcall
RosKmdDdi::DdiAddAdapter(
    IN_CONST_PDEVICE_OBJECT     PhysicalDeviceObject,
    OUT_PPVOID                  MiniportDeviceContext)
{
    return RosKmAdapter::AddAdapter(PhysicalDeviceObject, MiniportDeviceContext);
}

NTSTATUS __stdcall
RosKmdDdi::DdiStartAdapter(
    IN_CONST_PVOID          MiniportDeviceContext,
    IN_PDXGK_START_INFO     DxgkStartInfo,
    IN_PDXGKRNL_INTERFACE   DxgkInterface,
    OUT_PULONG              NumberOfVideoPresentSources,
    OUT_PULONG              NumberOfChildren)
{
    RosKmAdapter  *pRosKmAdapter = RosKmAdapter::Cast(MiniportDeviceContext);

    return pRosKmAdapter->Start(DxgkStartInfo, DxgkInterface, NumberOfVideoPresentSources, NumberOfChildren);
}


NTSTATUS __stdcall
RosKmdDdi::DdiStopAdapter(
    IN_CONST_PVOID  MiniportDeviceContext)
{
    RosKmAdapter  *pRosKmAdapter = RosKmAdapter::Cast(MiniportDeviceContext);

    return pRosKmAdapter->Stop();
}


NTSTATUS __stdcall
RosKmdDdi::DdiRemoveAdapter(
    IN_CONST_PVOID  MiniportDeviceContext)
{
    RosKmAdapter  *pRosKmAdapter = RosKmAdapter::Cast(MiniportDeviceContext);

    delete pRosKmAdapter;

    return STATUS_SUCCESS;
}

void
RosKmdDdi::DdiDpcRoutine(
    IN_CONST_PVOID  MiniportDeviceContext)
{
    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s MiniportDeviceContext=%lx\n",
        __FUNCTION__, MiniportDeviceContext);

    RosKmAdapter  *pRosKmdAdapter = RosKmAdapter::Cast(MiniportDeviceContext);

    pRosKmdAdapter->DpcRoutine();
}

NTSTATUS
RosKmdDdi::DdiDispatchIoRequest(
    IN_CONST_PVOID              MiniportDeviceContext,
    IN_ULONG                    VidPnSourceId,
    IN_PVIDEO_REQUEST_PACKET    VideoRequestPacket)
{
    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s MiniportDeviceContext=%lx\n",
        __FUNCTION__, MiniportDeviceContext);

    RosKmAdapter  *pRosKmdAdapter = RosKmAdapter::Cast(MiniportDeviceContext);
    return pRosKmdAdapter->DispatchIoRequest(VidPnSourceId, VideoRequestPacket);
}

BOOLEAN
RosKmdDdi::DdiInterruptRoutine(
    IN_CONST_PVOID  MiniportDeviceContext,
    IN_ULONG        MessageNumber)
{
    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s MiniportDeviceContext=%lx\n",
        __FUNCTION__, MiniportDeviceContext);

    RosKmAdapter  *pRosKmdAdapter = RosKmAdapter::Cast(MiniportDeviceContext);

    return pRosKmdAdapter->InterruptRoutine(MessageNumber);
}

NTSTATUS
__stdcall
RosKmdDdi::DdiBuildPagingBuffer(
    IN_CONST_HANDLE                 hAdapter,
    IN_PDXGKARG_BUILDPAGINGBUFFER   pArgs)
{
    RosKmAdapter  *pRosKmdAdapter = RosKmAdapter::Cast(hAdapter);

    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s hAdapter=%lx\n", __FUNCTION__, hAdapter);

    return pRosKmdAdapter->BuildPagingBuffer(pArgs);
}

NTSTATUS __stdcall RosKmdDdi::DdiSubmitCommand(
    IN_CONST_HANDLE                     hAdapter,
    IN_CONST_PDXGKARG_SUBMITCOMMAND     pSubmitCommand)
{
    RosKmAdapter  *pRosKmdAdapter = RosKmAdapter::Cast(hAdapter);

    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s hAdapter=%lx\n", __FUNCTION__, hAdapter);

    return pRosKmdAdapter->SubmitCommand(pSubmitCommand);
}

NTSTATUS __stdcall RosKmdDdi::DdiPatch(
    IN_CONST_HANDLE             hAdapter,
    IN_CONST_PDXGKARG_PATCH     pPatch)
{
    RosKmAdapter  *pRosKmdAdapter = RosKmAdapter::Cast(hAdapter);

    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s hAdapter=%lx\n", __FUNCTION__, hAdapter);

    return pRosKmdAdapter->Patch(pPatch);
}

NTSTATUS __stdcall RosKmdDdi::DdiCreateAllocation(
    IN_CONST_HANDLE                     hAdapter,
    INOUT_PDXGKARG_CREATEALLOCATION     pCreateAllocation)
{
    RosKmAdapter  *pRosKmdAdapter = RosKmAdapter::Cast(hAdapter);

    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s hAdapter=%lx\n", __FUNCTION__, hAdapter);

    return pRosKmdAdapter->CreateAllocation(pCreateAllocation);
}


NTSTATUS __stdcall RosKmdDdi::DdiDestroyAllocation(
    IN_CONST_HANDLE                         hAdapter,
    IN_CONST_PDXGKARG_DESTROYALLOCATION     pDestroyAllocation)
{
    RosKmAdapter  *pRosKmdAdapter = RosKmAdapter::Cast(hAdapter);

    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s hAdapter=%lx\n", __FUNCTION__, hAdapter);

    return pRosKmdAdapter->DestroyAllocation(pDestroyAllocation);
}


NTSTATUS __stdcall RosKmdDdi::DdiQueryAdapterInfo(
    IN_CONST_HANDLE                         hAdapter,
    IN_CONST_PDXGKARG_QUERYADAPTERINFO      pQueryAdapterInfo)
{
    RosKmAdapter  *pRosKmdAdapter = RosKmAdapter::Cast(hAdapter);

    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s hAdapter=%lx\n", __FUNCTION__, hAdapter);

    return pRosKmdAdapter->QueryAdapterInfo(pQueryAdapterInfo);
}


NTSTATUS __stdcall RosKmdDdi::DdiDescribeAllocation(
    IN_CONST_HANDLE                         hAdapter,
    INOUT_PDXGKARG_DESCRIBEALLOCATION       pDescribeAllocation)
{
    RosKmAdapter  *pRosKmdAdapter = RosKmAdapter::Cast(hAdapter);

    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s hAdapter=%lx\n", __FUNCTION__, hAdapter);

    return pRosKmdAdapter->DescribeAllocation(pDescribeAllocation);
}


NTSTATUS __stdcall RosKmdDdi::DdiGetNodeMetadata(
    IN_CONST_HANDLE                 hAdapter,
    UINT                            NodeOrdinal,
    OUT_PDXGKARG_GETNODEMETADATA    pGetNodeMetadata)
{
    RosKmAdapter  *pRosKmdAdapter = RosKmAdapter::Cast(hAdapter);

    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s hAdapter=%lx\n", __FUNCTION__, hAdapter);

    return pRosKmdAdapter->GetNodeMetadata(NodeOrdinal, pGetNodeMetadata);
}


NTSTATUS __stdcall RosKmdDdi::DdiGetStandardAllocationDriverData(
    IN_CONST_HANDLE                                 hAdapter,
    INOUT_PDXGKARG_GETSTANDARDALLOCATIONDRIVERDATA  pGetStandardAllocationDriverData)
{
    RosKmAdapter  *pRosKmdAdapter = RosKmAdapter::Cast(hAdapter);

    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s hAdapter=%lx\n", __FUNCTION__, hAdapter);

    return pRosKmdAdapter->GetStandardAllocationDriverData(pGetStandardAllocationDriverData);
}


NTSTATUS
__stdcall
RosKmdDdi::DdiSubmitCommandVirtual(
    IN_CONST_HANDLE                         hAdapter,
    IN_CONST_PDXGKARG_SUBMITCOMMANDVIRTUAL  pSubmitCommandVirtual)
{
    RosKmAdapter  *pRosKmdAdapter = RosKmAdapter::Cast(hAdapter);

    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s hAdapter=%lx\n", __FUNCTION__, hAdapter);

    return pRosKmdAdapter->SubmitCommandVirtual(pSubmitCommandVirtual);
}


NTSTATUS
__stdcall
RosKmdDdi::DdiPreemptCommand(
    IN_CONST_HANDLE                     hAdapter,
    IN_CONST_PDXGKARG_PREEMPTCOMMAND    pPreemptCommand)
{
    RosKmAdapter  *pRosKmdAdapter = RosKmAdapter::Cast(hAdapter);

    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s hAdapter=%lx\n", __FUNCTION__, hAdapter);

    return pRosKmdAdapter->PreemptCommand(pPreemptCommand);
}


NTSTATUS
__stdcall CALLBACK
RosKmdDdi::DdiRestartFromTimeout(
    IN_CONST_HANDLE     hAdapter)
{
    RosKmAdapter  *pRosKmdAdapter = RosKmAdapter::Cast(hAdapter);

    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s hAdapter=%lx\n", __FUNCTION__, hAdapter);

    return pRosKmdAdapter->RestartFromTimeout();
}


NTSTATUS
__stdcall
RosKmdDdi::DdiCancelCommand(
    IN_CONST_HANDLE                 hAdapter,
    IN_CONST_PDXGKARG_CANCELCOMMAND pCancelCommand)
{
    RosKmAdapter  *pRosKmdAdapter = RosKmAdapter::Cast(hAdapter);

    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s hAdapter=%lx\n", __FUNCTION__, hAdapter);

    return pRosKmdAdapter->CancelCommand(pCancelCommand);
}


NTSTATUS
__stdcall
RosKmdDdi::DdiQueryCurrentFence(
    IN_CONST_HANDLE                    hAdapter,
    INOUT_PDXGKARG_QUERYCURRENTFENCE   pCurrentFence)
{
    RosKmAdapter  *pRosKmdAdapter = RosKmAdapter::Cast(hAdapter);

    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s hAdapter=%lx\n", __FUNCTION__, hAdapter);

    return pRosKmdAdapter->QueryCurrentFence(pCurrentFence);
}


NTSTATUS
__stdcall
RosKmdDdi::DdiResetEngine(
    IN_CONST_HANDLE             hAdapter,
    INOUT_PDXGKARG_RESETENGINE  pResetEngine)
{
    RosKmAdapter  *pRosKmdAdapter = RosKmAdapter::Cast(hAdapter);

    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s hAdapter=%lx\n", __FUNCTION__, hAdapter);

    return pRosKmdAdapter->ResetEngine(pResetEngine);
}


NTSTATUS
__stdcall
RosKmdDdi::DdiQueryEngineStatus(
    IN_CONST_HANDLE                     hAdapter,
    INOUT_PDXGKARG_QUERYENGINESTATUS    pQueryEngineStatus)
{
    RosKmAdapter  *pRosKmdAdapter = RosKmAdapter::Cast(hAdapter);

    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s hAdapter=%lx\n", __FUNCTION__, hAdapter);

    return pRosKmdAdapter->QueryEngineStatus(pQueryEngineStatus);
}




NTSTATUS
__stdcall
RosKmdDdi::DdiControlInterrupt(
    IN_CONST_HANDLE                 hAdapter,
    IN_CONST_DXGK_INTERRUPT_TYPE    InterruptType,
    IN_BOOLEAN                      EnableInterrupt)
{
    RosKmAdapter  *pRosKmdAdapter = RosKmAdapter::Cast(hAdapter);

    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s hAdapter=%lx\n", __FUNCTION__, hAdapter);

    return pRosKmdAdapter->ControlInterrupt(InterruptType, EnableInterrupt);
}


NTSTATUS
__stdcall
RosKmdDdi::DdiCollectDbgInfo(
    IN_CONST_HANDLE                         hAdapter,
    IN_CONST_PDXGKARG_COLLECTDBGINFO        pCollectDbgInfo)
{
    RosKmAdapter  *pRosKmdAdapter = RosKmAdapter::Cast(hAdapter);

    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s hAdapter=%lx\n", __FUNCTION__, hAdapter);

    return pRosKmdAdapter->CollectDbgInfo(pCollectDbgInfo);
}


NTSTATUS
__stdcall
RosKmdDdi::DdiPresent(
    IN_CONST_HANDLE         hContext,
    INOUT_PDXGKARG_PRESENT  pPresent)
{
    RosKmContext  *pRosKmdContext = RosKmContext::Cast(hContext);

    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s hContext=%lx\n", __FUNCTION__, hContext);

    return pRosKmdContext->Present(pPresent);
}


NTSTATUS
__stdcall
RosKmdDdi::DdiCreateProcess(
    IN_PVOID  pMiniportDeviceContext,
    IN DXGKARG_CREATEPROCESS* pArgs)
{
    RosKmAdapter  *pRosKmdAdapter = RosKmAdapter::Cast(pMiniportDeviceContext);

    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s pMiniportDeviceContext=%lx\n", __FUNCTION__, pMiniportDeviceContext);

    return pRosKmdAdapter->CreateProcess(pArgs);
}


NTSTATUS
__stdcall
RosKmdDdi::DdiDestroyProcess(
    IN_PVOID pMiniportDeviceContext,
    IN HANDLE KmdProcessHandle)
{
    RosKmAdapter  *pRosKmdAdapter = RosKmAdapter::Cast(pMiniportDeviceContext);

    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s pMiniportDeviceContext=%lx\n", __FUNCTION__, pMiniportDeviceContext);

    return pRosKmdAdapter->DestroyProcess(KmdProcessHandle);
}


void
__stdcall
RosKmdDdi::DdiSetStablePowerState(
    IN_CONST_HANDLE                        hAdapter,
    IN_CONST_PDXGKARG_SETSTABLEPOWERSTATE  pArgs)
{
    RosKmAdapter  *pRosKmdAdapter = RosKmAdapter::Cast(hAdapter);

    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s hAdapter=%lx\n", __FUNCTION__, hAdapter);

    return pRosKmdAdapter->SetStablePowerState(pArgs);
}


NTSTATUS
__stdcall
RosKmdDdi::DdiCalibrateGpuClock(
    IN_CONST_HANDLE                             hAdapter,
    IN UINT32                                   NodeOrdinal,
    IN UINT32                                   EngineOrdinal,
    OUT_PDXGKARG_CALIBRATEGPUCLOCK              pClockCalibration
    )
{
    RosKmAdapter  *pRosKmdAdapter = RosKmAdapter::Cast(hAdapter);

    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s hAdapter=%lx\n", __FUNCTION__, hAdapter);

    return pRosKmdAdapter->CalibrateGpuClock(NodeOrdinal, EngineOrdinal, pClockCalibration);
}


NTSTATUS
__stdcall
RosKmdDdi::DdiRenderKm(
    IN_CONST_HANDLE         hContext,
    INOUT_PDXGKARG_RENDER   pRender)
{
    RosKmContext  *pRosKmContext = RosKmContext::Cast(hContext);

    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s hContext=%lx\n", __FUNCTION__, hContext);

    return pRosKmContext->RenderKm(pRender);
}

NTSTATUS
__stdcall
RosKmdDdi::DdiEscape(
    IN_CONST_HANDLE                 hAdapter,
    IN_CONST_PDXGKARG_ESCAPE        pEscape)
{
    RosKmAdapter  *pRosKmdAdapter = RosKmAdapter::Cast(hAdapter);

    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s hAdapter=%lx\n", __FUNCTION__, hAdapter);

    return pRosKmdAdapter->Escape(pEscape);
}

NTSTATUS
__stdcall
RosKmdDdi::DdiResetFromTimeout(
    IN_CONST_HANDLE     hAdapter)
{
    RosKmAdapter  *pRosKmdAdapter = RosKmAdapter::Cast(hAdapter);

    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s hAdapter=%lx\n", __FUNCTION__, hAdapter);

    return pRosKmdAdapter->ResetFromTimeout();
}


NTSTATUS
RosKmdDdi::DdiQueryInterface(
    IN_CONST_PVOID          MiniportDeviceContext,
    IN_PQUERY_INTERFACE     QueryInterface)
{
    RosKmAdapter  *pRosKmdAdapter = RosKmAdapter::Cast(MiniportDeviceContext);

    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s MiniportDeviceContext=%lx\n", __FUNCTION__, MiniportDeviceContext);

    return pRosKmdAdapter->QueryInterface(QueryInterface);
}


NTSTATUS
RosKmdDdi::DdiQueryChildRelations(
    IN_CONST_PVOID                  MiniportDeviceContext,
    INOUT_PDXGK_CHILD_DESCRIPTOR    ChildRelations,
    IN_ULONG                        ChildRelationsSize)
{
    RosKmAdapter  *pRosKmdAdapter = RosKmAdapter::Cast(MiniportDeviceContext);

    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s MiniportDeviceContext=%lx\n", __FUNCTION__, MiniportDeviceContext);

    return pRosKmdAdapter->QueryChildRelations(ChildRelations, ChildRelationsSize);
}


NTSTATUS
RosKmdDdi::DdiQueryChildStatus(
    IN_CONST_PVOID          MiniportDeviceContext,
    IN_PDXGK_CHILD_STATUS   ChildStatus,
    IN_BOOLEAN              NonDestructiveOnly)
{
    RosKmAdapter  *pRosKmdAdapter = RosKmAdapter::Cast(MiniportDeviceContext);

    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s MiniportDeviceContext=%lx\n", __FUNCTION__, MiniportDeviceContext);

    return pRosKmdAdapter->QueryChildStatus(ChildStatus, NonDestructiveOnly);
}

NTSTATUS
RosKmdDdi::DdiQueryDeviceDescriptor(
    IN_CONST_PVOID                  MiniportDeviceContext,
    IN_ULONG                        ChildUid,
    INOUT_PDXGK_DEVICE_DESCRIPTOR   pDeviceDescriptor)
{
    RosKmAdapter  *pRosKmdAdapter = RosKmAdapter::Cast(MiniportDeviceContext);

    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s MiniportDeviceContext=%lx\n", __FUNCTION__, MiniportDeviceContext);

    return pRosKmdAdapter->QueryDeviceDescriptor(ChildUid, pDeviceDescriptor);
}


NTSTATUS
RosKmdDdi::DdiSetPowerState(
    IN_CONST_PVOID          MiniportDeviceContext,
    IN_ULONG                DeviceUid,
    IN_DEVICE_POWER_STATE   DevicePowerState,
    IN_POWER_ACTION         ActionType)
{
    RosKmAdapter  *pRosKmdAdapter = RosKmAdapter::Cast(MiniportDeviceContext);

    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s MiniportDeviceContext=%lx\n", __FUNCTION__, MiniportDeviceContext);

    return pRosKmdAdapter->SetPowerState(DeviceUid, DevicePowerState, ActionType);
}

NTSTATUS
RosKmdDdi::DdiSetPowerComponentFState(
    IN_CONST_PVOID       MiniportDeviceContext,
    IN UINT              ComponentIndex,
    IN UINT              FState)
{
    RosKmAdapter  *pRosKmdAdapter = RosKmAdapter::Cast(MiniportDeviceContext);

    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s MiniportDeviceContext=%lx\n", __FUNCTION__, MiniportDeviceContext);

    return pRosKmdAdapter->SetPowerComponentFState(ComponentIndex, FState);
}


NTSTATUS
RosKmdDdi::DdiPowerRuntimeControlRequest(
    IN_CONST_PVOID       MiniportDeviceContext,
    IN LPCGUID           PowerControlCode,
    IN OPTIONAL PVOID    InBuffer,
    IN SIZE_T            InBufferSize,
    OUT OPTIONAL PVOID   OutBuffer,
    IN SIZE_T            OutBufferSize,
    OUT OPTIONAL PSIZE_T BytesReturned)
{
    RosKmAdapter  *pRosKmdAdapter = RosKmAdapter::Cast(MiniportDeviceContext);

    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s MiniportDeviceContext=%lx\n", __FUNCTION__, MiniportDeviceContext);

    return pRosKmdAdapter->PowerRuntimeControlRequest(PowerControlCode, InBuffer, InBufferSize, OutBuffer, OutBufferSize, BytesReturned);
}


NTSTATUS
RosKmdDdi::DdiNotifyAcpiEvent(
    IN_CONST_PVOID      MiniportDeviceContext,
    IN_DXGK_EVENT_TYPE  EventType,
    IN_ULONG            Event,
    IN_PVOID            Argument,
    OUT_PULONG          AcpiFlags)
{
    RosKmAdapter  *pRosKmdAdapter = RosKmAdapter::Cast(MiniportDeviceContext);

    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s MiniportDeviceContext=%lx\n", __FUNCTION__, MiniportDeviceContext);

    return pRosKmdAdapter->NotifyAcpiEvent(EventType, Event, Argument, AcpiFlags);
}


void
RosKmdDdi::DdiResetDevice(
    IN_CONST_PVOID  MiniportDeviceContext)
{
    RosKmAdapter  *pRosKmdAdapter = RosKmAdapter::Cast(MiniportDeviceContext);

    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s MiniportDeviceContext=%lx\n", __FUNCTION__, MiniportDeviceContext);

    return pRosKmdAdapter->ResetDevice();
}

ROS_NONPAGED_SEGMENT_BEGIN; //================================================

_Use_decl_annotations_
NTSTATUS RosKmdDisplayDdi::DdiSetVidPnSourceAddress (
    HANDLE const hAdapter,
    const DXGKARG_SETVIDPNSOURCEADDRESS* SetVidPnSourceAddressPtr
    )
{
    return RosKmAdapter::Cast(hAdapter)->SetVidPnSourceAddress(
            SetVidPnSourceAddressPtr);
}

ROS_NONPAGED_SEGMENT_END; //==================================================
ROS_PAGED_SEGMENT_BEGIN; //===================================================
// TODO[jordanh] put PASSIVE_LEVEL DDIs in the paged section

//
// RosKmdDdi
//

_Use_decl_annotations_
NTSTATUS RosKmdDdi::DdiOpenAllocation (
    HANDLE const hDevice,
    const DXGKARG_OPENALLOCATION* ArgsPtr
    )
{
    PAGED_CODE();
    ROS_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    return RosKmDevice::Cast(hDevice)->OpenAllocation(ArgsPtr);
}

_Use_decl_annotations_
NTSTATUS RosKmdDdi::DdiQueryDependentEngineGroup (
    HANDLE const hAdapter,
    DXGKARG_QUERYDEPENDENTENGINEGROUP* ArgsPtr
    )
{
    PAGED_CODE();
    ROS_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    return RosKmAdapter::Cast(hAdapter)->QueryDependentEngineGroup(ArgsPtr);
}

//
// RosKmdDisplayDdi
//

_Use_decl_annotations_
NTSTATUS RosKmdDisplayDdi::DdiSetPalette (
    HANDLE const AdapterPtr,
    const DXGKARG_SETPALETTE* SetPalettePtr
    )
{
    PAGED_CODE();
    ROS_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    return RosKmAdapter::Cast(AdapterPtr)->SetPalette(SetPalettePtr);
}

_Use_decl_annotations_
NTSTATUS RosKmdDisplayDdi::DdiSetPointerPosition (
    HANDLE const AdapterPtr,
    const DXGKARG_SETPOINTERPOSITION* SetPointerPositionPtr
    )
{
    PAGED_CODE();
    ROS_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    return RosKmAdapter::Cast(AdapterPtr)->SetPointerPosition(
        SetPointerPositionPtr);
}

_Use_decl_annotations_
NTSTATUS RosKmdDisplayDdi::DdiSetPointerShape (
    HANDLE const AdapterPtr,
    const DXGKARG_SETPOINTERSHAPE* SetPointerShapePtr
    )
{
    PAGED_CODE();
    ROS_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    return RosKmAdapter::Cast(AdapterPtr)->SetPointerShape(SetPointerShapePtr);
}

_Use_decl_annotations_
NTSTATUS RosKmdDisplayDdi::DdiIsSupportedVidPn (
    VOID* const MiniportDeviceContextPtr,
    DXGKARG_ISSUPPORTEDVIDPN* IsSupportedVidPnPtr
    )
{
    PAGED_CODE();
    ROS_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    return RosKmAdapter::Cast(MiniportDeviceContextPtr)->IsSupportedVidPn(
            IsSupportedVidPnPtr);
}

_Use_decl_annotations_
NTSTATUS RosKmdDisplayDdi::DdiRecommendFunctionalVidPn (
    VOID* const MiniportDeviceContextPtr,
    const DXGKARG_RECOMMENDFUNCTIONALVIDPN* const RecommendFunctionalVidPnPtr
    )
{
    PAGED_CODE();
    ROS_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    return RosKmAdapter::Cast(MiniportDeviceContextPtr)->RecommendFunctionalVidPn(
            RecommendFunctionalVidPnPtr);
}

_Use_decl_annotations_
NTSTATUS RosKmdDisplayDdi::DdiEnumVidPnCofuncModality (
    VOID* const MiniportDeviceContextPtr,
    const DXGKARG_ENUMVIDPNCOFUNCMODALITY* const EnumCofuncModalityPtr
    )
{
    PAGED_CODE();
    ROS_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    return RosKmAdapter::Cast(MiniportDeviceContextPtr)->EnumVidPnCofuncModality(
            EnumCofuncModalityPtr);
}

_Use_decl_annotations_
NTSTATUS RosKmdDisplayDdi::DdiSetVidPnSourceVisibility (
    VOID* const MiniportDeviceContextPtr,
    const DXGKARG_SETVIDPNSOURCEVISIBILITY* SetVidPnSourceVisibilityPtr
    )
{
    PAGED_CODE();
    ROS_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    return RosKmAdapter::Cast(MiniportDeviceContextPtr)->SetVidPnSourceVisibility(
            SetVidPnSourceVisibilityPtr);
}

_Use_decl_annotations_
NTSTATUS RosKmdDisplayDdi::DdiCommitVidPn (
    VOID* const MiniportDeviceContextPtr,
    const DXGKARG_COMMITVIDPN* const CommitVidPnPtr
    )
{
    PAGED_CODE();
    ROS_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    return RosKmAdapter::Cast(MiniportDeviceContextPtr)->CommitVidPn(
            CommitVidPnPtr);
}

_Use_decl_annotations_
NTSTATUS RosKmdDisplayDdi::DdiUpdateActiveVidPnPresentPath (
    VOID* const MiniportDeviceContextPtr,
    const DXGKARG_UPDATEACTIVEVIDPNPRESENTPATH* const UpdateActiveVidPnPresentPathPtr
    )
{
    PAGED_CODE();
    ROS_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    return RosKmAdapter::Cast(MiniportDeviceContextPtr)->UpdateActiveVidPnPresentPath(
            UpdateActiveVidPnPresentPathPtr);
}

_Use_decl_annotations_
NTSTATUS RosKmdDisplayDdi::DdiRecommendMonitorModes (
    VOID* const MiniportDeviceContextPtr,
    const DXGKARG_RECOMMENDMONITORMODES* const RecommendMonitorModesPtr
    )
{
    PAGED_CODE();
    ROS_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    return RosKmAdapter::Cast(MiniportDeviceContextPtr)->RecommendMonitorModes(
            RecommendMonitorModesPtr);
}

_Use_decl_annotations_
NTSTATUS RosKmdDisplayDdi::DdiGetScanLine (
    HANDLE const AdapterPtr,
    DXGKARG_GETSCANLINE*  GetScanLinePtr
    )
{
    PAGED_CODE();
    ROS_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    return RosKmAdapter::Cast(AdapterPtr)->GetScanLine(GetScanLinePtr);
}

_Use_decl_annotations_
NTSTATUS RosKmdDisplayDdi::DdiQueryVidPnHWCapability (
    VOID* const MiniportDeviceContextPtr,
    DXGKARG_QUERYVIDPNHWCAPABILITY* VidPnHWCapsPtr
    )
{
    PAGED_CODE();
    ROS_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    return RosKmAdapter::Cast(MiniportDeviceContextPtr)->QueryVidPnHWCapability(
            VidPnHWCapsPtr);
}

_Use_decl_annotations_
NTSTATUS RosKmdDisplayDdi::DdiStopDeviceAndReleasePostDisplayOwnership (
    VOID* const MiniportDeviceContextPtr,
    D3DDDI_VIDEO_PRESENT_TARGET_ID TargetId,
    DXGK_DISPLAY_INFORMATION* DisplayInfoPtr
    )
{
    PAGED_CODE();
    ROS_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    return RosKmAdapter::Cast(MiniportDeviceContextPtr)->StopDeviceAndReleasePostDisplayOwnership(
            TargetId,
            DisplayInfoPtr);
}

ROS_PAGED_SEGMENT_END; //=====================================================
