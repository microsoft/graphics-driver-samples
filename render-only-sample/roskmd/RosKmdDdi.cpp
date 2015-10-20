#include "RosKmd.h"

#include "RosKmdAdapter.h"
#include "RosKmdDevice.h"
#include "RosKmdAllocation.h"
#include "RosKmdDdi.h"
#include "RosKmdContext.h"
#include "RosKmdResource.h"

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

    RosKmAdapter  *pRosKmAdapter = RosKmAdapter::Cast(MiniportDeviceContext);

    pRosKmAdapter->DpcRoutine();
}

NTSTATUS
RosKmdDdi::DdiDispatchIoRequest(
    IN_CONST_PVOID              MiniportDeviceContext,
    IN_ULONG                    VidPnSourceId,
    IN_PVIDEO_REQUEST_PACKET    VideoRequestPacket)
{
    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s MiniportDeviceContext=%lx\n",
        __FUNCTION__, MiniportDeviceContext);

    MiniportDeviceContext;
    VidPnSourceId;
    VideoRequestPacket;

    NT_ASSERT(FALSE);
    return STATUS_SUCCESS;
}

BOOLEAN
RosKmdDdi::DdiInterruptRoutine(
    IN_CONST_PVOID  MiniportDeviceContext,
    IN_ULONG        MessageNumber)
{
    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s MiniportDeviceContext=%lx\n",
        __FUNCTION__, MiniportDeviceContext);

    MiniportDeviceContext;
    MessageNumber;

#if HW_GPU

    RosKmAdapter  *pRosKmAdapter = RosKmAdapter::Cast(MiniportDeviceContext);

    if (!m_bReadyToHandleInterrupt)
    {
        return FALSE;
    }

    // Acknowledge the interrupt

    // If the interrupt is for DMA buffer completion,
    // queue the DPC to wake up the worker thread
    KeInsertQueueDpc(&pRosKmAdapter->m_hwDmaBufCompletionDpc, NULL, NULL);

    return TRUE;

#else

    return FALSE;

#endif
}

NTSTATUS
__stdcall
RosKmdDdi::DdiBuildPagingBuffer(
    IN_CONST_HANDLE                 hAdapter,
    IN_PDXGKARG_BUILDPAGINGBUFFER   pArgs)
{
    RosKmAdapter  *pRosKmAdapter = RosKmAdapter::Cast(hAdapter);

    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s hAdapter=%lx\n", __FUNCTION__, hAdapter);

    return pRosKmAdapter->BuildPagingBuffer(pArgs);
}

