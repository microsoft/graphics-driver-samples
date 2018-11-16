#include "CosKmd.h"

#include "CosKmdLogging.h"
#include "CosKmdDdi.tmh"

#include "CosKmdAdapter.h"
#include "CosKmdDevice.h"
#include "CosKmdAllocation.h"
#include "CosKmdDdi.h"
#include "CosKmdContext.h"
#include "CosKmdResource.h"
#include "CosKmdUtil.h"

NTSTATUS __stdcall
CosKmdDdi::DdiAddAdapter(
    IN_CONST_PDEVICE_OBJECT     PhysicalDeviceObject,
    OUT_PPVOID                  MiniportDeviceContext)
{
    return CosKmAdapter::AddAdapter(PhysicalDeviceObject, MiniportDeviceContext);
}

NTSTATUS __stdcall
CosKmdDdi::DdiStartAdapter(
    IN_CONST_PVOID          MiniportDeviceContext,
    IN_PDXGK_START_INFO     DxgkStartInfo,
    IN_PDXGKRNL_INTERFACE   DxgkInterface,
    OUT_PULONG              NumberOfVideoPresentSources,
    OUT_PULONG              NumberOfChildren)
{
    CosKmAdapter  *pCosKmAdapter = CosKmAdapter::Cast(MiniportDeviceContext);

    return pCosKmAdapter->Start(DxgkStartInfo, DxgkInterface, NumberOfVideoPresentSources, NumberOfChildren);
}


NTSTATUS __stdcall
CosKmdDdi::DdiStopAdapter(
    IN_CONST_PVOID  MiniportDeviceContext)
{
    CosKmAdapter  *pCosKmAdapter = CosKmAdapter::Cast(MiniportDeviceContext);

    return pCosKmAdapter->Stop();
}


NTSTATUS __stdcall
CosKmdDdi::DdiRemoveAdapter(
    IN_CONST_PVOID  MiniportDeviceContext)
{
    CosKmAdapter  *pCosKmAdapter = CosKmAdapter::Cast(MiniportDeviceContext);

    delete pCosKmAdapter;

    return STATUS_SUCCESS;
}

void
CosKmdDdi::DdiDpcRoutine(
    IN_CONST_PVOID  MiniportDeviceContext)
{
    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s MiniportDeviceContext=%lx\n",
        __FUNCTION__, MiniportDeviceContext);

    CosKmAdapter  *pCosKmdAdapter = CosKmAdapter::Cast(MiniportDeviceContext);

    pCosKmdAdapter->DpcRoutine();
}

NTSTATUS
CosKmdDdi::DdiDispatchIoRequest(
    IN_CONST_PVOID              MiniportDeviceContext,
    IN_ULONG                    VidPnSourceId,
    IN_PVIDEO_REQUEST_PACKET    VideoRequestPacket)
{
    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s MiniportDeviceContext=%lx\n",
        __FUNCTION__, MiniportDeviceContext);

    CosKmAdapter  *pCosKmdAdapter = CosKmAdapter::Cast(MiniportDeviceContext);
    return pCosKmdAdapter->DispatchIoRequest(VidPnSourceId, VideoRequestPacket);
}

BOOLEAN
CosKmdDdi::DdiInterruptRoutine(
    IN_CONST_PVOID  MiniportDeviceContext,
    IN_ULONG        MessageNumber)
{
    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s MiniportDeviceContext=%lx\n",
        __FUNCTION__, MiniportDeviceContext);

    CosKmAdapter  *pCosKmdAdapter = CosKmAdapter::Cast(MiniportDeviceContext);

    return pCosKmdAdapter->InterruptRoutine(MessageNumber);
}

NTSTATUS
__stdcall
CosKmdDdi::DdiBuildPagingBuffer(
    IN_CONST_HANDLE                 hAdapter,
    IN_PDXGKARG_BUILDPAGINGBUFFER   pArgs)
{
    CosKmAdapter  *pCosKmdAdapter = CosKmAdapter::Cast(hAdapter);

    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s hAdapter=%lx\n", __FUNCTION__, hAdapter);

    return pCosKmdAdapter->BuildPagingBuffer(pArgs);
}

NTSTATUS __stdcall CosKmdDdi::DdiSubmitCommand(
    IN_CONST_HANDLE                     hAdapter,
    IN_CONST_PDXGKARG_SUBMITCOMMAND     pSubmitCommand)
{
    CosKmAdapter  *pCosKmdAdapter = CosKmAdapter::Cast(hAdapter);

    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s hAdapter=%lx\n", __FUNCTION__, hAdapter);

    return pCosKmdAdapter->SubmitCommand(pSubmitCommand);
}

#if COS_PHYSICAL_SUPPORT
NTSTATUS __stdcall CosKmdDdi::DdiPatch(
    IN_CONST_HANDLE             hAdapter,
    IN_CONST_PDXGKARG_PATCH     pPatch)
{
    CosKmAdapter  *pCosKmdAdapter = CosKmAdapter::Cast(hAdapter);

    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s hAdapter=%lx\n", __FUNCTION__, hAdapter);

    return pCosKmdAdapter->Patch(pPatch);
}
#endif

NTSTATUS __stdcall CosKmdDdi::DdiCreateAllocation(
    IN_CONST_HANDLE                     hAdapter,
    INOUT_PDXGKARG_CREATEALLOCATION     pCreateAllocation)
{
    CosKmAdapter  *pCosKmdAdapter = CosKmAdapter::Cast(hAdapter);

    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s hAdapter=%lx\n", __FUNCTION__, hAdapter);

    return pCosKmdAdapter->CreateAllocation(pCreateAllocation);
}


NTSTATUS __stdcall CosKmdDdi::DdiDestroyAllocation(
    IN_CONST_HANDLE                         hAdapter,
    IN_CONST_PDXGKARG_DESTROYALLOCATION     pDestroyAllocation)
{
    CosKmAdapter  *pCosKmdAdapter = CosKmAdapter::Cast(hAdapter);

    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s hAdapter=%lx\n", __FUNCTION__, hAdapter);

    return pCosKmdAdapter->DestroyAllocation(pDestroyAllocation);
}


NTSTATUS __stdcall CosKmdDdi::DdiQueryAdapterInfo(
    IN_CONST_HANDLE                         hAdapter,
    IN_CONST_PDXGKARG_QUERYADAPTERINFO      pQueryAdapterInfo)
{
    CosKmAdapter  *pCosKmdAdapter = CosKmAdapter::Cast(hAdapter);

    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s hAdapter=%lx\n", __FUNCTION__, hAdapter);

    return pCosKmdAdapter->QueryAdapterInfo(pQueryAdapterInfo);
}


NTSTATUS __stdcall CosKmdDdi::DdiDescribeAllocation(
    IN_CONST_HANDLE                         hAdapter,
    INOUT_PDXGKARG_DESCRIBEALLOCATION       pDescribeAllocation)
{
    CosKmAdapter  *pCosKmdAdapter = CosKmAdapter::Cast(hAdapter);

    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s hAdapter=%lx\n", __FUNCTION__, hAdapter);

    return pCosKmdAdapter->DescribeAllocation(pDescribeAllocation);
}


NTSTATUS __stdcall CosKmdDdi::DdiGetNodeMetadata(
    IN_CONST_HANDLE                 hAdapter,
    UINT                            NodeOrdinal,
    OUT_PDXGKARG_GETNODEMETADATA    pGetNodeMetadata)
{
    CosKmAdapter  *pCosKmdAdapter = CosKmAdapter::Cast(hAdapter);

    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s hAdapter=%lx\n", __FUNCTION__, hAdapter);

    return pCosKmdAdapter->GetNodeMetadata(NodeOrdinal, pGetNodeMetadata);
}


NTSTATUS __stdcall CosKmdDdi::DdiGetStandardAllocationDriverData(
    IN_CONST_HANDLE                                 hAdapter,
    INOUT_PDXGKARG_GETSTANDARDALLOCATIONDRIVERDATA  pGetStandardAllocationDriverData)
{
    CosKmAdapter  *pCosKmdAdapter = CosKmAdapter::Cast(hAdapter);

    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s hAdapter=%lx\n", __FUNCTION__, hAdapter);

    return pCosKmdAdapter->GetStandardAllocationDriverData(pGetStandardAllocationDriverData);
}

#if COS_GPUVA_SUPPORT

NTSTATUS
__stdcall
CosKmdDdi::DdiSubmitCommandVirtual(
    IN_CONST_HANDLE                         hAdapter,
    IN_CONST_PDXGKARG_SUBMITCOMMANDVIRTUAL  pSubmitCommandVirtual)
{
    CosKmAdapter  *pCosKmdAdapter = CosKmAdapter::Cast(hAdapter);

    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s hAdapter=%lx\n", __FUNCTION__, hAdapter);

    return pCosKmdAdapter->SubmitCommandVirtual(pSubmitCommandVirtual);
}

SIZE_T
__stdcall
CosKmdDdi::DdiGetRootPageTableSize(
    IN_CONST_HANDLE                     /*hAdapter*/,
    INOUT_PDXGKARG_GETROOTPAGETABLESIZE /*pArgs*/)
{
    return COS_PAGE_TABLE_SIZE;
}

VOID
__stdcall
CosKmdDdi::DdiSetRootPageTable(
    IN_CONST_HANDLE                     /*hAdapter*/,
    IN_CONST_PDXGKARG_SETROOTPAGETABLE  pSetPageTable)
{
    CosKmContext  *pCosKmContext = CosKmContext::Cast(pSetPageTable->hContext);

    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s hContext=%lx\n", __FUNCTION__, pSetPageTable->hContext);

    return pCosKmContext->SetRootPageTable(pSetPageTable);
}

#endif

NTSTATUS
__stdcall
CosKmdDdi::DdiPreemptCommand(
    IN_CONST_HANDLE                     hAdapter,
    IN_CONST_PDXGKARG_PREEMPTCOMMAND    pPreemptCommand)
{
    CosKmAdapter  *pCosKmdAdapter = CosKmAdapter::Cast(hAdapter);

    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s hAdapter=%lx\n", __FUNCTION__, hAdapter);

    return pCosKmdAdapter->PreemptCommand(pPreemptCommand);
}


NTSTATUS
__stdcall
CosKmdDdi::DdiCancelCommand(
    IN_CONST_HANDLE                 hAdapter,
    IN_CONST_PDXGKARG_CANCELCOMMAND pCancelCommand)
{
    CosKmAdapter  *pCosKmdAdapter = CosKmAdapter::Cast(hAdapter);

    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s hAdapter=%lx\n", __FUNCTION__, hAdapter);

    return pCosKmdAdapter->CancelCommand(pCancelCommand);
}


NTSTATUS
__stdcall
CosKmdDdi::DdiResetFromTimeout(
    IN_CONST_HANDLE     hAdapter)
{
    CosKmAdapter  *pCosKmdAdapter = CosKmAdapter::Cast(hAdapter);

    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s hAdapter=%lx\n", __FUNCTION__, hAdapter);

    return pCosKmdAdapter->ResetFromTimeout();
}


NTSTATUS
__stdcall CALLBACK
CosKmdDdi::DdiRestartFromTimeout(
    IN_CONST_HANDLE     hAdapter)
{
    CosKmAdapter  *pCosKmdAdapter = CosKmAdapter::Cast(hAdapter);

    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s hAdapter=%lx\n", __FUNCTION__, hAdapter);

    return pCosKmdAdapter->RestartFromTimeout();
}


NTSTATUS
__stdcall
CosKmdDdi::DdiCollectDbgInfo(
    IN_CONST_HANDLE                         hAdapter,
    IN_CONST_PDXGKARG_COLLECTDBGINFO        pCollectDbgInfo)
{
    CosKmAdapter  *pCosKmdAdapter = CosKmAdapter::Cast(hAdapter);

    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s hAdapter=%lx\n", __FUNCTION__, hAdapter);

    return pCosKmdAdapter->CollectDbgInfo(pCollectDbgInfo);
}


_Use_decl_annotations_
NTSTATUS CosKmdDdi::DdiQueryDependentEngineGroup(
    HANDLE const hAdapter,
    DXGKARG_QUERYDEPENDENTENGINEGROUP* ArgsPtr
)
{
    PAGED_CODE();
    COS_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    return CosKmAdapter::Cast(hAdapter)->QueryDependentEngineGroup(ArgsPtr);
}


NTSTATUS
__stdcall
CosKmdDdi::DdiQueryEngineStatus(
    IN_CONST_HANDLE                     hAdapter,
    INOUT_PDXGKARG_QUERYENGINESTATUS    pQueryEngineStatus)
{
    CosKmAdapter  *pCosKmdAdapter = CosKmAdapter::Cast(hAdapter);

    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s hAdapter=%lx\n", __FUNCTION__, hAdapter);

    return pCosKmdAdapter->QueryEngineStatus(pQueryEngineStatus);
}


NTSTATUS
__stdcall
CosKmdDdi::DdiResetEngine(
    IN_CONST_HANDLE             hAdapter,
    INOUT_PDXGKARG_RESETENGINE  pResetEngine)
{
    CosKmAdapter  *pCosKmdAdapter = CosKmAdapter::Cast(hAdapter);

    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s hAdapter=%lx\n", __FUNCTION__, hAdapter);

    return pCosKmdAdapter->ResetEngine(pResetEngine);
}


#if COS_GPUVA_SUPPORT
NTSTATUS
__stdcall
CosKmdDdi::DdiCreateProcess(
    IN_PVOID  pMiniportDeviceContext,
    IN DXGKARG_CREATEPROCESS* pArgs)
{
    CosKmAdapter  *pCosKmdAdapter = CosKmAdapter::Cast(pMiniportDeviceContext);

    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s pMiniportDeviceContext=%lx\n", __FUNCTION__, pMiniportDeviceContext);

    return pCosKmdAdapter->CreateProcess(pArgs);
}


NTSTATUS
__stdcall
CosKmdDdi::DdiDestroyProcess(
    IN_PVOID pMiniportDeviceContext,
    IN HANDLE KmdProcessHandle)
{
    CosKmAdapter  *pCosKmdAdapter = CosKmAdapter::Cast(pMiniportDeviceContext);

    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s pMiniportDeviceContext=%lx\n", __FUNCTION__, pMiniportDeviceContext);

    return pCosKmdAdapter->DestroyProcess(KmdProcessHandle);
}
#endif

void
__stdcall
CosKmdDdi::DdiSetStablePowerState(
    IN_CONST_HANDLE                        hAdapter,
    IN_CONST_PDXGKARG_SETSTABLEPOWERSTATE  pArgs)
{
    CosKmAdapter  *pCosKmdAdapter = CosKmAdapter::Cast(hAdapter);

    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s hAdapter=%lx\n", __FUNCTION__, hAdapter);

    return pCosKmdAdapter->SetStablePowerState(pArgs);
}


NTSTATUS
__stdcall
CosKmdDdi::DdiCalibrateGpuClock(
    IN_CONST_HANDLE                             hAdapter,
    IN UINT32                                   NodeOrdinal,
    IN UINT32                                   EngineOrdinal,
    OUT_PDXGKARG_CALIBRATEGPUCLOCK              pClockCalibration
    )
{
    CosKmAdapter  *pCosKmdAdapter = CosKmAdapter::Cast(hAdapter);

    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s hAdapter=%lx\n", __FUNCTION__, hAdapter);

    return pCosKmdAdapter->CalibrateGpuClock(NodeOrdinal, EngineOrdinal, pClockCalibration);
}

NTSTATUS
__stdcall
CosKmdDdi::DdiEscape(
    IN_CONST_HANDLE                 hAdapter,
    IN_CONST_PDXGKARG_ESCAPE        pEscape)
{
    CosKmAdapter  *pCosKmdAdapter = CosKmAdapter::Cast(hAdapter);

    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s hAdapter=%lx\n", __FUNCTION__, hAdapter);

    return pCosKmdAdapter->Escape(pEscape);
}


NTSTATUS
CosKmdDdi::DdiQueryInterface(
    IN_CONST_PVOID          MiniportDeviceContext,
    IN_PQUERY_INTERFACE     QueryInterface)
{
    CosKmAdapter  *pCosKmdAdapter = CosKmAdapter::Cast(MiniportDeviceContext);

    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s MiniportDeviceContext=%lx\n", __FUNCTION__, MiniportDeviceContext);

    return pCosKmdAdapter->QueryInterface(QueryInterface);
}


NTSTATUS
CosKmdDdi::DdiQueryChildRelations(
    IN_CONST_PVOID                  MiniportDeviceContext,
    INOUT_PDXGK_CHILD_DESCRIPTOR    ChildRelations,
    IN_ULONG                        ChildRelationsSize)
{
    CosKmAdapter  *pCosKmdAdapter = CosKmAdapter::Cast(MiniportDeviceContext);

    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s MiniportDeviceContext=%lx\n", __FUNCTION__, MiniportDeviceContext);

    return pCosKmdAdapter->QueryChildRelations(ChildRelations, ChildRelationsSize);
}


NTSTATUS
CosKmdDdi::DdiQueryChildStatus(
    IN_CONST_PVOID          MiniportDeviceContext,
    IN_PDXGK_CHILD_STATUS   ChildStatus,
    IN_BOOLEAN              NonDestructiveOnly)
{
    CosKmAdapter  *pCosKmdAdapter = CosKmAdapter::Cast(MiniportDeviceContext);

    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s MiniportDeviceContext=%lx\n", __FUNCTION__, MiniportDeviceContext);

    return pCosKmdAdapter->QueryChildStatus(ChildStatus, NonDestructiveOnly);
}

NTSTATUS
CosKmdDdi::DdiQueryDeviceDescriptor(
    IN_CONST_PVOID                  MiniportDeviceContext,
    IN_ULONG                        ChildUid,
    INOUT_PDXGK_DEVICE_DESCRIPTOR   pDeviceDescriptor)
{
    CosKmAdapter  *pCosKmdAdapter = CosKmAdapter::Cast(MiniportDeviceContext);

    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s MiniportDeviceContext=%lx\n", __FUNCTION__, MiniportDeviceContext);

    return pCosKmdAdapter->QueryDeviceDescriptor(ChildUid, pDeviceDescriptor);
}


NTSTATUS
CosKmdDdi::DdiSetPowerState(
    IN_CONST_PVOID          MiniportDeviceContext,
    IN_ULONG                DeviceUid,
    IN_DEVICE_POWER_STATE   DevicePowerState,
    IN_POWER_ACTION         ActionType)
{
    CosKmAdapter  *pCosKmdAdapter = CosKmAdapter::Cast(MiniportDeviceContext);

    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s MiniportDeviceContext=%lx\n", __FUNCTION__, MiniportDeviceContext);

    return pCosKmdAdapter->SetPowerState(DeviceUid, DevicePowerState, ActionType);
}

NTSTATUS
CosKmdDdi::DdiSetPowerComponentFState(
    IN_CONST_PVOID       MiniportDeviceContext,
    IN UINT              ComponentIndex,
    IN UINT              FState)
{
    CosKmAdapter  *pCosKmdAdapter = CosKmAdapter::Cast(MiniportDeviceContext);

    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s MiniportDeviceContext=%lx\n", __FUNCTION__, MiniportDeviceContext);

    return pCosKmdAdapter->SetPowerComponentFState(ComponentIndex, FState);
}


NTSTATUS
CosKmdDdi::DdiPowerRuntimeControlRequest(
    IN_CONST_PVOID       MiniportDeviceContext,
    IN LPCGUID           PowerControlCode,
    IN OPTIONAL PVOID    InBuffer,
    IN SIZE_T            InBufferSize,
    OUT OPTIONAL PVOID   OutBuffer,
    IN SIZE_T            OutBufferSize,
    OUT OPTIONAL PSIZE_T BytesReturned)
{
    CosKmAdapter  *pCosKmdAdapter = CosKmAdapter::Cast(MiniportDeviceContext);

    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s MiniportDeviceContext=%lx\n", __FUNCTION__, MiniportDeviceContext);

    return pCosKmdAdapter->PowerRuntimeControlRequest(PowerControlCode, InBuffer, InBufferSize, OutBuffer, OutBufferSize, BytesReturned);
}


NTSTATUS
CosKmdDdi::DdiNotifyAcpiEvent(
    IN_CONST_PVOID      MiniportDeviceContext,
    IN_DXGK_EVENT_TYPE  EventType,
    IN_ULONG            Event,
    IN_PVOID            Argument,
    OUT_PULONG          AcpiFlags)
{
    CosKmAdapter  *pCosKmdAdapter = CosKmAdapter::Cast(MiniportDeviceContext);

    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s MiniportDeviceContext=%lx\n", __FUNCTION__, MiniportDeviceContext);

    return pCosKmdAdapter->NotifyAcpiEvent(EventType, Event, Argument, AcpiFlags);
}


void
CosKmdDdi::DdiResetDevice(
    IN_CONST_PVOID  MiniportDeviceContext)
{
    CosKmAdapter  *pCosKmdAdapter = CosKmAdapter::Cast(MiniportDeviceContext);

    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s MiniportDeviceContext=%lx\n", __FUNCTION__, MiniportDeviceContext);

    return pCosKmdAdapter->ResetDevice();
}

COS_NONPAGED_SEGMENT_BEGIN; //================================================

#if 0
_Use_decl_annotations_
NTSTATUS CosKmdDisplayDdi::DdiSetVidPnSourceAddress (
    HANDLE const hAdapter,
    const DXGKARG_SETVIDPNSOURCEADDRESS* SetVidPnSourceAddressPtr
    )
{
    return CosKmAdapter::Cast(hAdapter)->SetVidPnSourceAddress(
            SetVidPnSourceAddressPtr);
}
#endif

COS_NONPAGED_SEGMENT_END; //==================================================
COS_PAGED_SEGMENT_BEGIN; //===================================================
// TODO[jordanh] put PASSIVE_LEVEL DDIs in the paged section

//
// CosKmdDdi
//

_Use_decl_annotations_
NTSTATUS CosKmdDdi::DdiOpenAllocation (
    HANDLE const hDevice,
    const DXGKARG_OPENALLOCATION* ArgsPtr
    )
{
    PAGED_CODE();
    COS_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    return CosKmDevice::Cast(hDevice)->OpenAllocation(ArgsPtr);
}

NTSTATUS
CosKmdDdi::DdiSetVirtualMachineData(
    IN_CONST_HANDLE                         hAdapter,
    IN_CONST_PDXGKARG_SETVIRTUALMACHINEDATA Args)
{
    PAGED_CODE();
    COS_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    CosKmAdapter  *pCosKmdAdapter = CosKmAdapter::Cast(hAdapter);

    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s hAdapter=%lx\n", __FUNCTION__, hAdapter);

    return pCosKmdAdapter->SetVirtualMachineData(Args);
}

#if 0
//
// CosKmdDisplayDdi
//

_Use_decl_annotations_
NTSTATUS CosKmdDisplayDdi::DdiSetPalette (
    HANDLE const AdapterPtr,
    const DXGKARG_SETPALETTE* SetPalettePtr
    )
{
    PAGED_CODE();
    COS_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    return CosKmAdapter::Cast(AdapterPtr)->SetPalette(SetPalettePtr);
}

_Use_decl_annotations_
NTSTATUS CosKmdDisplayDdi::DdiSetPointerPosition (
    HANDLE const AdapterPtr,
    const DXGKARG_SETPOINTERPOSITION* SetPointerPositionPtr
    )
{
    PAGED_CODE();
    COS_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    return CosKmAdapter::Cast(AdapterPtr)->SetPointerPosition(
        SetPointerPositionPtr);
}

_Use_decl_annotations_
NTSTATUS CosKmdDisplayDdi::DdiSetPointerShape (
    HANDLE const AdapterPtr,
    const DXGKARG_SETPOINTERSHAPE* SetPointerShapePtr
    )
{
    PAGED_CODE();
    COS_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    return CosKmAdapter::Cast(AdapterPtr)->SetPointerShape(SetPointerShapePtr);
}

_Use_decl_annotations_
NTSTATUS CosKmdDisplayDdi::DdiIsSupportedVidPn (
    VOID* const MiniportDeviceContextPtr,
    DXGKARG_ISSUPPORTEDVIDPN* IsSupportedVidPnPtr
    )
{
    PAGED_CODE();
    COS_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    return CosKmAdapter::Cast(MiniportDeviceContextPtr)->IsSupportedVidPn(
            IsSupportedVidPnPtr);
}

_Use_decl_annotations_
NTSTATUS CosKmdDisplayDdi::DdiRecommendFunctionalVidPn (
    VOID* const MiniportDeviceContextPtr,
    const DXGKARG_RECOMMENDFUNCTIONALVIDPN* const RecommendFunctionalVidPnPtr
    )
{
    PAGED_CODE();
    COS_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    return CosKmAdapter::Cast(MiniportDeviceContextPtr)->RecommendFunctionalVidPn(
            RecommendFunctionalVidPnPtr);
}

_Use_decl_annotations_
NTSTATUS CosKmdDisplayDdi::DdiEnumVidPnCofuncModality (
    VOID* const MiniportDeviceContextPtr,
    const DXGKARG_ENUMVIDPNCOFUNCMODALITY* const EnumCofuncModalityPtr
    )
{
    PAGED_CODE();
    COS_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    return CosKmAdapter::Cast(MiniportDeviceContextPtr)->EnumVidPnCofuncModality(
            EnumCofuncModalityPtr);
}

_Use_decl_annotations_
NTSTATUS CosKmdDisplayDdi::DdiSetVidPnSourceVisibility (
    VOID* const MiniportDeviceContextPtr,
    const DXGKARG_SETVIDPNSOURCEVISIBILITY* SetVidPnSourceVisibilityPtr
    )
{
    PAGED_CODE();
    COS_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    return CosKmAdapter::Cast(MiniportDeviceContextPtr)->SetVidPnSourceVisibility(
            SetVidPnSourceVisibilityPtr);
}

_Use_decl_annotations_
NTSTATUS CosKmdDisplayDdi::DdiCommitVidPn (
    VOID* const MiniportDeviceContextPtr,
    const DXGKARG_COMMITVIDPN* const CommitVidPnPtr
    )
{
    PAGED_CODE();
    COS_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    return CosKmAdapter::Cast(MiniportDeviceContextPtr)->CommitVidPn(
            CommitVidPnPtr);
}

_Use_decl_annotations_
NTSTATUS CosKmdDisplayDdi::DdiUpdateActiveVidPnPresentPath (
    VOID* const MiniportDeviceContextPtr,
    const DXGKARG_UPDATEACTIVEVIDPNPRESENTPATH* const UpdateActiveVidPnPresentPathPtr
    )
{
    PAGED_CODE();
    COS_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    return CosKmAdapter::Cast(MiniportDeviceContextPtr)->UpdateActiveVidPnPresentPath(
            UpdateActiveVidPnPresentPathPtr);
}

_Use_decl_annotations_
NTSTATUS CosKmdDisplayDdi::DdiRecommendMonitorModes (
    VOID* const MiniportDeviceContextPtr,
    const DXGKARG_RECOMMENDMONITORMODES* const RecommendMonitorModesPtr
    )
{
    PAGED_CODE();
    COS_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    return CosKmAdapter::Cast(MiniportDeviceContextPtr)->RecommendMonitorModes(
            RecommendMonitorModesPtr);
}

_Use_decl_annotations_
NTSTATUS CosKmdDisplayDdi::DdiGetScanLine (
    HANDLE const AdapterPtr,
    DXGKARG_GETSCANLINE*  GetScanLinePtr
    )
{
    PAGED_CODE();
    COS_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    return CosKmAdapter::Cast(AdapterPtr)->GetScanLine(GetScanLinePtr);
}

_Use_decl_annotations_
NTSTATUS CosKmdDisplayDdi::DdiQueryVidPnHWCapability (
    VOID* const MiniportDeviceContextPtr,
    DXGKARG_QUERYVIDPNHWCAPABILITY* VidPnHWCapsPtr
    )
{
    PAGED_CODE();
    COS_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    return CosKmAdapter::Cast(MiniportDeviceContextPtr)->QueryVidPnHWCapability(
            VidPnHWCapsPtr);
}

_Use_decl_annotations_
NTSTATUS CosKmdDisplayDdi::DdiStopDeviceAndReleasePostDisplayOwnership (
    VOID* const MiniportDeviceContextPtr,
    D3DDDI_VIDEO_PRESENT_TARGET_ID TargetId,
    DXGK_DISPLAY_INFORMATION* DisplayInfoPtr
    )
{
    PAGED_CODE();
    COS_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    return CosKmAdapter::Cast(MiniportDeviceContextPtr)->StopDeviceAndReleasePostDisplayOwnership(
            TargetId,
            DisplayInfoPtr);
}
#endif

COS_PAGED_SEGMENT_END; //=====================================================
