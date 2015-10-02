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

    RosKmAdapter  *pRosKmAdapter = RosKmAdapter::Cast(MiniportDeviceContext);

    if (DeviceUid == DISPLAY_ADAPTER_HW_ID)
    {
        pRosKmAdapter->m_AdapterPowerDState = DevicePowerState;
    }

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

    RosKmAdapter  *pRosKmAdapter = RosKmAdapter::Cast(MiniportDeviceContext);

    return pRosKmAdapter->SetPowerComponentFState(ComponentIndex,FState);
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

    RosKmAdapter  *pRosKmAdapter = RosKmAdapter::Cast(MiniportDeviceContext);

    if (IsEqualGUID(*PowerControlCode, GUID_DXGKDDI_POWER_MANAGEMENT_PREPARE_TO_START))
    {
    }
    else if (IsEqualGUID(*PowerControlCode, GUID_DXGKDDI_POWER_MANAGEMENT_STARTED))
    {
        pRosKmAdapter->m_PowerManagementStarted = TRUE;
    }
    else if (IsEqualGUID(*PowerControlCode, GUID_DXGKDDI_POWER_MANAGEMENT_STOPPED))
    {
        pRosKmAdapter->m_PowerManagementStarted = FALSE;
    }

    InBuffer;
    InBufferSize;
    OutBuffer;
    OutBufferSize;
    BytesReturned;

    return STATUS_SUCCESS;
}

NTSTATUS
RosKmAdapter::SetPowerComponentFState(
    IN UINT ComponentIndex,
    IN UINT FState)
{
	//
	// Validate component index.
	//
	if (ComponentIndex >= m_NumNodes)
	{
		return STATUS_INVALID_PARAMETER;
	}

    //
	// Update Fstate.
    //
    m_EnginePowerFState[ComponentIndex] = FState;

    //
    // Notify power transition is completed. 
    // (Driver can perform this within this call, or scheduler workitem later if not possible to do now).
    //
    m_DxgkInterface.DxgkCbCompleteFStateTransition(
        m_DxgkInterface.DeviceHandle,
        ComponentIndex);
	
    return STATUS_SUCCESS;
}
