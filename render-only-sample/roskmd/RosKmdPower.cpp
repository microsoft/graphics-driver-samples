#include "precomp.h"

#include "RosKmdLogging.h"
#include "RosKmdPower.tmh"

#include "RosKmd.h"
#include "RosKmdAdapter.h"
#include "RosKmdAllocation.h"
#include "RosKmdContext.h"
#include "RosKmdResource.h"
#include "RosKmdGlobal.h"
#include "RosKmdUtil.h"
#include "RosGpuCommand.h"
#include "RosKmdAcpi.h"

NTSTATUS
RosKmAdapter::SetPowerState(
    IN_ULONG                DeviceUid,
    IN_DEVICE_POWER_STATE   DevicePowerState,
    IN_POWER_ACTION         ActionType)
{
    if (DeviceUid == DISPLAY_ADAPTER_HW_ID)
    {
        m_AdapterPowerDState = DevicePowerState;
    }

    if (RosKmdGlobal::IsRenderOnly())
    {
        ROS_LOG_WARNING("SetPowerState() is not implemented by render side.");
        return STATUS_SUCCESS;
    }

    return m_display.SetPowerState(DeviceUid, DevicePowerState, ActionType);
}

NTSTATUS
RosKmAdapter::PowerRuntimeControlRequest(
    IN LPCGUID           PowerControlCode,
    IN OPTIONAL PVOID    InBuffer,
    IN SIZE_T            InBufferSize,
    OUT OPTIONAL PVOID   OutBuffer,
    IN SIZE_T            OutBufferSize,
    OUT OPTIONAL PSIZE_T BytesReturned)
{
    if (IsEqualGUID(*PowerControlCode, GUID_DXGKDDI_POWER_MANAGEMENT_PREPARE_TO_START))
    {
    }
    else if (IsEqualGUID(*PowerControlCode, GUID_DXGKDDI_POWER_MANAGEMENT_STARTED))
    {
        m_PowerManagementStarted = TRUE;
    }
    else if (IsEqualGUID(*PowerControlCode, GUID_DXGKDDI_POWER_MANAGEMENT_STOPPED))
    {
        m_PowerManagementStarted = FALSE;
    }

    InBuffer;
    InBufferSize;
    OutBuffer;
    OutBufferSize;
    BytesReturned;

    return STATUS_SUCCESS;
}

NTSTATUS
RosKmAdapter::InitializePowerComponentInfo()
{
    NTSTATUS Status;

    RosKmAcpiReader acpiReader(this, DISPLAY_ADAPTER_HW_ID);
    Status = acpiReader.Read('DCMP'); // Invoke Method(PMCD).
    if (NT_SUCCESS(Status) &&
        (acpiReader.GetOutputArgumentCount() == 3)) // must return 3 output arguments
    {
        RosKmAcpiArgumentParser acpiParser(&acpiReader, NULL);

        // Validate Version.
        ULONG Version;
        Status = acpiParser.GetValue<ULONG>(&Version);
        if (!NT_SUCCESS(Status) || (Version != 1)) // currently Version must be 1.
        {
            return STATUS_ACPI_INVALID_DATA;
        }

        // Validate number of power compoment
        ULONG numPowerCompoment;
        Status = acpiParser.GetValue<ULONG>(&numPowerCompoment);
        if (!NT_SUCCESS(Status) || (numPowerCompoment != C_ROSD_GPU_ENGINE_COUNT)) // currently only GPU node
        {
            return STATUS_ACPI_INVALID_DATA;
        }

        UNALIGNED ACPI_METHOD_ARGUMENT* pPowerComponentPackage;
        pPowerComponentPackage = acpiParser.GetPackage();
        NT_ASSERT(pPowerComponentPackage);

        RosKmAcpiArgumentParser acpiPowerComponentParser(&acpiReader, pPowerComponentPackage);
        for (ULONG i = 0; i < numPowerCompoment; i++)
        {
            UNALIGNED ACPI_METHOD_ARGUMENT* pComponentPackage;
            pComponentPackage = acpiPowerComponentParser.GetPackage();
            NT_ASSERT(pComponentPackage);

            RosKmAcpiArgumentParser acpiComponentParser(&acpiReader, pComponentPackage);

            ULONG componentIndex;
            Status = acpiComponentParser.GetValue<ULONG>(&componentIndex);
            NT_ASSERT(componentIndex == 0);

            Status = acpiComponentParser.GetValue<DXGK_POWER_COMPONENT_TYPE>(&m_PowerComponents[componentIndex].ComponentMapping.ComponentType);
            NT_ASSERT(NT_SUCCESS(Status));

            Status = acpiComponentParser.GetValue<UINT>(&m_PowerComponents[componentIndex].ComponentMapping.EngineDesc.NodeIndex);
            NT_ASSERT(NT_SUCCESS(Status));

            GUID* pComponentGuid = NULL;
            ULONG ComponentGuidLength = 0;
            Status = acpiComponentParser.GetBuffer((BYTE**)&pComponentGuid, &ComponentGuidLength);
            NT_ASSERT(NT_SUCCESS(Status));
            NT_ASSERT(pComponentGuid != NULL);
            NT_ASSERT(ComponentGuidLength == sizeof(GUID));
            RtlCopyMemory(&m_PowerComponents[componentIndex].ComponentGuid, pComponentGuid, sizeof(GUID));

            char *pComponentName = NULL;
            ULONG ComponentNameLength = 0;
            Status = acpiComponentParser.GetAnsiString(&pComponentName, &ComponentNameLength);
            NT_ASSERT(NT_SUCCESS(Status));
            NT_ASSERT(pComponentName);
            RtlCopyMemory(
                &m_PowerComponents[componentIndex].ComponentName[0],
                pComponentName,
                min(ComponentNameLength, sizeof(m_PowerComponents[componentIndex].ComponentName)));

            Status = acpiComponentParser.GetValue<ULONG>(&m_PowerComponents[componentIndex].StateCount);
            NT_ASSERT(NT_SUCCESS(Status));
            NT_ASSERT(m_PowerComponents[componentIndex].StateCount);

            UNALIGNED ACPI_METHOD_ARGUMENT* pPowerStatePackage;
            pPowerStatePackage = acpiComponentParser.GetPackage();
            NT_ASSERT(pPowerStatePackage);

            RosKmAcpiArgumentParser acpiPowerStateParser(&acpiReader, pPowerStatePackage);
            for (ULONG j = 0; j < m_PowerComponents[componentIndex].StateCount; j++)
            {
                UNALIGNED ACPI_METHOD_ARGUMENT* pStatePackage;
                pStatePackage = acpiPowerStateParser.GetPackage();
                NT_ASSERT(pStatePackage);

                RosKmAcpiArgumentParser acpiStateParser(&acpiReader, pStatePackage);

                Status = acpiStateParser.GetValue<ULONGLONG>(&m_PowerComponents[componentIndex].States[j].TransitionLatency);
                NT_ASSERT(NT_SUCCESS(Status));

                Status = acpiStateParser.GetValue<ULONGLONG>(&m_PowerComponents[componentIndex].States[j].ResidencyRequirement);
                NT_ASSERT(NT_SUCCESS(Status));

                Status = acpiStateParser.GetValue<ULONG>(&m_PowerComponents[componentIndex].States[j].NominalPower);
                NT_ASSERT(NT_SUCCESS(Status));
            }

            //
            // No dependent provider.
            //
            m_PowerComponents[componentIndex].ProviderCount = 0;
            RtlZeroMemory(&m_PowerComponents[componentIndex].Providers, sizeof(m_PowerComponents[componentIndex].Providers));

            //
            // Driver makes callback to complete transition.
            //
            m_PowerComponents[componentIndex].Flags.Value = 0;
            m_PowerComponents[componentIndex].Flags.DriverCompletesFStateTransition = 1;
        }

        //
        // If everything work out good, set number of power components.
        //
        m_NumPowerComponents = numPowerCompoment;

        return STATUS_SUCCESS;
    }

    return  STATUS_ACPI_INVALID_DATA;
}

NTSTATUS
RosKmAdapter::GetNumPowerComponents()
{
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

    RtlCopyMemory(pPowerComponent, &m_PowerComponents[ComponentIndex], sizeof(DXGK_POWER_RUNTIME_COMPONENT));

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
