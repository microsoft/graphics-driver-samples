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
RosKmAdapter::GetNumPowerComponents()
{
    //
    // Only component index for 3D engine is supported.
    //
	m_NumPowerComponents = m_NumNodes;

    return m_NumPowerComponents;
}

NTSTATUS
RosKmAdapter::GetPowerComponentInfo(
    IN UINT ComponentIndex,
    OUT DXGK_POWER_RUNTIME_COMPONENT* pPowerComponent)
{
    if ((ComponentIndex >= m_NumPowerComponents) || 
         (pPowerComponent == NULL))
    {
        return STATUS_INVALID_PARAMETER;
    }

    RtlZeroMemory(pPowerComponent, sizeof(DXGK_POWER_RUNTIME_COMPONENT));

    pPowerComponent->StateCount = 3; // three F states; F0/1/2.

    // These are fake/temporary numbers, it has to be adjusted with real h/w numbers.
    // F0
    pPowerComponent->States[0].TransitionLatency = 0; // must be 0
    pPowerComponent->States[0].ResidencyRequirement = 0; // must be 0.
    pPowerComponent->States[0].NominalPower = 4;
    // F1
    pPowerComponent->States[1].TransitionLatency = 10000;
    pPowerComponent->States[1].ResidencyRequirement = 0;
    pPowerComponent->States[1].NominalPower = 2;
    // F2
    pPowerComponent->States[2].TransitionLatency = 40000;
    pPowerComponent->States[2].ResidencyRequirement = 0;
    pPowerComponent->States[2].NominalPower = 1;

    // Component Mapping to 3D engine(s).
    pPowerComponent->ComponentMapping.ComponentType = DXGK_POWER_COMPONENT_ENGINE;
    pPowerComponent->ComponentMapping.EngineDesc.NodeIndex = ComponentIndex; // currently nodeIndex == componentIndex since only 3D engines are exposed as power component.

    // Driver makes callback to complete transition.
    pPowerComponent->Flags.DriverCompletesFStateTransition = 1;

    // [hideyukn:TODO]
    // Component[i]->ComponentGuid is required to communicate with PEP.

    RtlStringCbPrintfA(reinterpret_cast<NTSTRSAFE_PSTR>(&pPowerComponent->ComponentName[0]),
                       sizeof(pPowerComponent->ComponentName),
                       "3D_Engine_%02X_Power", ComponentIndex);

    pPowerComponent->ProviderCount = 0; // no dependent provider

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
	if (ComponentIndex >= m_NumPowerComponents)
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
