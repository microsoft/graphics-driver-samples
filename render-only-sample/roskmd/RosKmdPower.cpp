#include <ntifs.h>
#include "RosKmd.h"
#include "RosKmdAdapter.h"
#include "RosKmdAllocation.h"
#include "RosKmdContext.h"
#include "RosKmdResource.h"
#include "RosKmdGlobal.h"
#include "RosKmdUtil.h"
#include "RosGpuCommand.h"

NTSTATUS
RosKmAdapter::DdiSetPowerState(
    IN_CONST_PVOID          MiniportDeviceContext,
    IN_ULONG                DeviceUid,
    IN_DEVICE_POWER_STATE   DevicePowerState,
    IN_POWER_ACTION         ActionType)
{
    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s MiniportDeviceContext=%lx\n",
        __FUNCTION__, MiniportDeviceContext);

    MiniportDeviceContext;
    DeviceUid;
    DevicePowerState;
    ActionType;

    return STATUS_SUCCESS;
}

NTSTATUS
RosKmAdapter::DdiSetPowerComponentFState(
    IN_CONST_PVOID       MiniportDeviceContext,
    IN UINT              ComponentIndex,
    IN UINT              FState)
{
    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s MiniportDeviceContext=%lx\n",
        __FUNCTION__, MiniportDeviceContext);

    ComponentIndex;
    FState;

    return STATUS_SUCCESS;
}

NTSTATUS
RosKmAdapter::DdiPowerRuntimeControlRequest(
    IN_CONST_PVOID       MiniportDeviceContext,
    IN LPCGUID           PowerControlCode,
    IN OPTIONAL PVOID    InBuffer,
    IN SIZE_T            InBufferSize,
    OUT OPTIONAL PVOID   OutBuffer,
    IN SIZE_T            OutBufferSize,
    OUT OPTIONAL PSIZE_T BytesReturned)
{
    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s MiniportDeviceContext=%lx\n",
        __FUNCTION__, MiniportDeviceContext);

    PowerControlCode;
    InBuffer;
    InBufferSize;
    OutBuffer;
    OutBufferSize;
    BytesReturned;

    return STATUS_SUCCESS;
}


