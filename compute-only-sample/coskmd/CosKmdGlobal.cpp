#include "CosKmd.h"

#include "CosKmdLogging.h"
#include "CosKmdGlobal.tmh"

#include "CosKmdGlobal.h"
#include "CosKmdDevice.h"
#include "CosKmdAdapter.h"
#include "CosKmdContext.h"
#include "CosKmdDdi.h"
#include "CosKmdUtil.h"

#include <ntverp.h>

DRIVER_OBJECT* CosKmdGlobal::s_pDriverObject;
bool CosKmdGlobal::s_bDoNotInstall = false;
size_t CosKmdGlobal::s_videoMemorySize = 0;
void * CosKmdGlobal::s_pVideoMemory = NULL;
PHYSICAL_ADDRESS CosKmdGlobal::s_videoMemoryPhysicalAddress;
bool CosKmdGlobal::s_bRenderOnly;
size_t CosKmdGlobal::s_vpuMemorySize = 0;
void * CosKmdGlobal::s_pVpuMemory = NULL;

void
CosKmdGlobal::DdiUnload(
    void)
{
    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s\n",
        __FUNCTION__);

    if (s_pVideoMemory != NULL)
    {
        MmFreeContiguousMemory(s_pVideoMemory);
        s_pVideoMemory = NULL;
        s_videoMemorySize = 0;
    }

	if (s_pVpuMemory != NULL)
	{
		ExFreePoolWithTag(s_pVpuMemory, 'cosd');
		s_pVpuMemory = NULL;
		s_vpuMemorySize = 0;
	}

    NT_ASSERT(s_pDriverObject);
    WPP_CLEANUP(s_pDriverObject);
    s_pDriverObject = nullptr;
}

void
CosKmdGlobal::DdiControlEtwLogging(
    IN_BOOLEAN  Enable,
    IN_ULONG    Flags,
    IN_UCHAR    Level)
{

    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s Enable=%d Flags=%lx Level=%lx\n",
        __FUNCTION__, Enable, (ULONG)Flags, (ULONG)Level);

    //
    // Enable/Disable ETW logging
    //

}

extern "C" __control_entrypoint(DeviceDriver)
    NTSTATUS DriverEntry(__in IN DRIVER_OBJECT* pDriverObject, __in IN UNICODE_STRING* pRegistryPath)
{
    return CosKmdGlobal::DriverEntry(pDriverObject, pRegistryPath);
}

NTSTATUS CosKmdGlobal::DriverEntry(__in IN DRIVER_OBJECT* pDriverObject, __in IN UNICODE_STRING* pRegistryPath)
{
    NTSTATUS    Status;
    DRIVER_INITIALIZATION_DATA DriverInitializationData;

    NT_ASSERT(!CosKmdGlobal::s_pDriverObject);
    CosKmdGlobal::s_pDriverObject = pDriverObject;

    //
    // Initialize logging
    //
    {
        WPP_INIT_TRACING(pDriverObject, pRegistryPath);
        RECORDER_CONFIGURE_PARAMS recorderConfigureParams;
        RECORDER_CONFIGURE_PARAMS_INIT(&recorderConfigureParams);
        WppRecorderConfigure(&recorderConfigureParams);
        WPP_RECORDER_LEVEL_FILTER(COS_TRACING_VIDPN) = FALSE;
        WPP_RECORDER_LEVEL_FILTER(COS_TRACING_PRESENT) = FALSE;
#if DBG
        WPP_RECORDER_LEVEL_FILTER(COS_TRACING_DEFAULT) = TRUE;
#endif // DBG
    }

    COS_LOG_INFORMATION(
        "Initializing roskmd. (pDriverObject=0x%p, pRegistryPath=%wZ)",
        pDriverObject,
        pRegistryPath);

    if (s_bDoNotInstall)
    {
        COS_LOG_INFORMATION("s_bDoNotInstall is set; aborting driver initialization.");
        return STATUS_UNSUCCESSFUL;
    }

    //
    // Allocate physically contiguous memory to represent cpu visible video memory
    //

    PHYSICAL_ADDRESS lowestAcceptableAddress;
    lowestAcceptableAddress.QuadPart = 0;

    PHYSICAL_ADDRESS highestAcceptableAddress;
    highestAcceptableAddress.QuadPart = -1;

    PHYSICAL_ADDRESS boundaryAddressMultiple;
    boundaryAddressMultiple.QuadPart = 0;

    s_videoMemorySize = kMaxVideoMemorySize;

    while (s_pVideoMemory == NULL)
    {
        //
        // The allocated contiguous memory is mapped as cached
        //
        // TODO[indyz]: Evaluate if flushing CPU cache for GPU access is the best option
        //
        // Use this option because GenerateRenderControlList has data alignment issue
        //

        s_pVideoMemory = MmAllocateContiguousMemorySpecifyCache(
                            s_videoMemorySize,
                            lowestAcceptableAddress,
                            highestAcceptableAddress,
                            boundaryAddressMultiple,
                            MmWriteCombined);               // TODO: Investigate how to support Cached allocation
        if (s_pVideoMemory != NULL)
        {
            break;
        }

        s_videoMemorySize >>= 1;
    }

    s_videoMemoryPhysicalAddress = MmGetPhysicalAddress(s_pVideoMemory);

    //
    // Query the driver registry key to see whether we're render only
    //
    {
        OBJECT_ATTRIBUTES attributes;
        InitializeObjectAttributes(
            &attributes,
            pRegistryPath,
            OBJ_KERNEL_HANDLE,
            nullptr,                // RootDirectory
            nullptr);               // SecurityDescriptor

        HANDLE keyHandle;
        Status = ZwOpenKey(&keyHandle, GENERIC_READ, &attributes);
        if (!NT_SUCCESS(Status))
        {
            COS_LOG_ERROR(
                "Failed to open driver registry key. (Status=%!STATUS!, pRegistryPath=%wZ, pDriverObject=0x%p)",
                Status,
                pRegistryPath,
                pDriverObject);
            return Status;
        }
        auto closeRegKey = COS_FINALLY::Do([&]
        {
            PAGED_CODE();
            NTSTATUS tempStatus = ZwClose(keyHandle);
            UNREFERENCED_PARAMETER(tempStatus);
            NT_ASSERT(NT_SUCCESS(tempStatus));
        });

        DECLARE_CONST_UNICODE_STRING(renderOnlyValueName, L"RenderOnly");

        #pragma warning(disable:4201)   // nameless struct/union
        union {
            KEY_VALUE_PARTIAL_INFORMATION PartialInfo;
            struct {
                ULONG TitleIndex;
                ULONG Type;
                ULONG DataLength;
                ULONG Data;
            } DUMMYSTRUCTNAME;
        } valueInfo;
        #pragma warning(default:4201) // nameless struct/union

        ULONG resultLength;
        Status = ZwQueryValueKey(
                keyHandle,
                const_cast<UNICODE_STRING*>(&renderOnlyValueName),
                KeyValuePartialInformation,
                &valueInfo.PartialInfo,
                sizeof(valueInfo),
                &resultLength);

        if (NT_SUCCESS(Status))
        {
            if (valueInfo.Type == REG_DWORD)
            {
                NT_ASSERT(valueInfo.DataLength == sizeof(valueInfo.Data));
                if (valueInfo.Data != 0)
                {
                    COS_LOG_INFORMATION("Configuring driver as render-only.");
                    s_bRenderOnly = true;
                }
            }
            else
            {
                COS_LOG_WARNING(
                    "RenderOnly registry value was found, but is not a REG_DWORD. (valueInfo.Type=%d, valueInfo.DataLength=%d)",
                    valueInfo.Type,
                    valueInfo.DataLength);
            }
        }
        else if (Status != STATUS_OBJECT_NAME_NOT_FOUND)
        {
            COS_LOG_ASSERTION(
                "Unexpected error occurred querying RenderOnly registry key. (Status=%!STATUS!)",
                Status);

            // an unexpected type in the registry could cause us to get here,
            // so don't stop the show.
        }
    } // RenderOnly

	//
	// Allocate vpu memory to be used to execute a shader
	//

	s_vpuMemorySize = 1024 * 1024;
	s_pVpuMemory = ExAllocatePoolWithTag(NonPagedPoolExecute, s_vpuMemorySize, 'cosd');
	if (s_pVpuMemory == NULL)
		return E_OUTOFMEMORY;

    //
    // Fill in the DriverInitializationData structure and call DlInitialize()
    //

    memset(&DriverInitializationData, 0, sizeof(DriverInitializationData));

    DriverInitializationData.Version = DXGKDDI_INTERFACE_VERSION;

    DriverInitializationData.DxgkDdiAddDevice = CosKmdDdi::DdiAddAdapter;
    DriverInitializationData.DxgkDdiStartDevice = CosKmdDdi::DdiStartAdapter;
    DriverInitializationData.DxgkDdiStopDevice = CosKmdDdi::DdiStopAdapter;
    DriverInitializationData.DxgkDdiRemoveDevice = CosKmdDdi::DdiRemoveAdapter;

    DriverInitializationData.DxgkDdiDispatchIoRequest = CosKmdDdi::DdiDispatchIoRequest;
    DriverInitializationData.DxgkDdiInterruptRoutine = CosKmdDdi::DdiInterruptRoutine;
    DriverInitializationData.DxgkDdiDpcRoutine = CosKmdDdi::DdiDpcRoutine;

    DriverInitializationData.DxgkDdiQueryChildRelations = CosKmdDdi::DdiQueryChildRelations;
    DriverInitializationData.DxgkDdiQueryChildStatus = CosKmdDdi::DdiQueryChildStatus;
    DriverInitializationData.DxgkDdiQueryDeviceDescriptor = CosKmdDdi::DdiQueryDeviceDescriptor;
    DriverInitializationData.DxgkDdiSetPowerState = CosKmdDdi::DdiSetPowerState;
    DriverInitializationData.DxgkDdiNotifyAcpiEvent = CosKmdDdi::DdiNotifyAcpiEvent;
    DriverInitializationData.DxgkDdiResetDevice = CosKmdDdi::DdiResetDevice;
    DriverInitializationData.DxgkDdiUnload = CosKmdGlobal::DdiUnload;
    DriverInitializationData.DxgkDdiQueryInterface = CosKmdDdi::DdiQueryInterface;
    DriverInitializationData.DxgkDdiControlEtwLogging = CosKmdGlobal::DdiControlEtwLogging;

    DriverInitializationData.DxgkDdiQueryAdapterInfo = CosKmdDdi::DdiQueryAdapterInfo;

    DriverInitializationData.DxgkDdiCreateDevice = CosKmDevice::DdiCreateDevice;

    DriverInitializationData.DxgkDdiCreateAllocation = CosKmdDdi::DdiCreateAllocation;
    DriverInitializationData.DxgkDdiDestroyAllocation = CosKmdDdi::DdiDestroyAllocation;

    DriverInitializationData.DxgkDdiDescribeAllocation = CosKmdDdi::DdiDescribeAllocation;
    DriverInitializationData.DxgkDdiGetStandardAllocationDriverData = CosKmdDdi::DdiGetStandardAllocationDriverData;

    // DriverInitializationData.DxgkDdiAcquireSwizzlingRange   = CosKmdAcquireSwizzlingRange;
    // DriverInitializationData.DxgkDdiReleaseSwizzlingRange   = CosKmdReleaseSwizzlingRange;

    DriverInitializationData.DxgkDdiOpenAllocation = CosKmdDdi::DdiOpenAllocation;
    DriverInitializationData.DxgkDdiCloseAllocation = CosKmDevice::DdiCloseAllocation;

#if COS_PHYSICAL_SUPPORT
    DriverInitializationData.DxgkDdiPatch = CosKmdDdi::DdiPatch;
    DriverInitializationData.DxgkDdiRender = CosKmContext::DdiRender;
    DriverInitializationData.DxgkDdiSubmitCommand = CosKmdDdi::DdiSubmitCommand;
#endif

    DriverInitializationData.DxgkDdiBuildPagingBuffer = CosKmdDdi::DdiBuildPagingBuffer;
    DriverInitializationData.DxgkDdiPreemptCommand = CosKmdDdi::DdiPreemptCommand;

    DriverInitializationData.DxgkDdiDestroyDevice = CosKmDevice::DdiDestroyDevice;

    DriverInitializationData.DxgkDdiResetFromTimeout = CosKmdDdi::DdiResetFromTimeout;
    DriverInitializationData.DxgkDdiRestartFromTimeout = CosKmdDdi::DdiRestartFromTimeout;
    DriverInitializationData.DxgkDdiEscape = CosKmdDdi::DdiEscape;
    DriverInitializationData.DxgkDdiCollectDbgInfo = CosKmdDdi::DdiCollectDbgInfo;

    DriverInitializationData.DxgkDdiCreateContext = CosKmContext::DdiCreateContext;
    DriverInitializationData.DxgkDdiDestroyContext = CosKmContext::DdiDestroyContext;

    //
    // Fill in DDI routines for resetting individual engine
    //
    DriverInitializationData.DxgkDdiQueryDependentEngineGroup = CosKmdDdi::DdiQueryDependentEngineGroup;
    DriverInitializationData.DxgkDdiQueryEngineStatus = CosKmdDdi::DdiQueryEngineStatus;
    DriverInitializationData.DxgkDdiResetEngine = CosKmdDdi::DdiResetEngine;

    //
    // Fill in DDI for canceling DMA buffers
    //
    DriverInitializationData.DxgkDdiCancelCommand = CosKmdDdi::DdiCancelCommand;

    //
    // Fill in DDI for component power management
    //
    DriverInitializationData.DxgkDdiSetPowerComponentFState = CosKmdDdi::DdiSetPowerComponentFState;
    DriverInitializationData.DxgkDdiPowerRuntimeControlRequest = CosKmdDdi::DdiPowerRuntimeControlRequest;

    DriverInitializationData.DxgkDdiGetNodeMetadata = CosKmdDdi::DdiGetNodeMetadata;

#if COS_GPUVA_SUPPORT
    DriverInitializationData.DxgkDdiSubmitCommandVirtual = CosKmdDdi::DdiSubmitCommandVirtual;
    DriverInitializationData.DxgkDdiSetRootPageTable = CosKmdDdi::DdiSetRootPageTable;
    DriverInitializationData.DxgkDdiGetRootPageTableSize = CosKmdDdi::DdiGetRootPageTableSize;

    DriverInitializationData.DxgkDdiCreateProcess = CosKmdDdi::DdiCreateProcess;
    DriverInitializationData.DxgkDdiDestroyProcess = CosKmdDdi::DdiDestroyProcess;
#endif

    DriverInitializationData.DxgkDdiCalibrateGpuClock = CosKmdDdi::DdiCalibrateGpuClock;
    DriverInitializationData.DxgkDdiFormatHistoryBuffer = CosKmContext::DdiFormatHistoryBuffer;

    DriverInitializationData.DxgkDdiSetStablePowerState = CosKmdDdi::DdiSetStablePowerState;

    DriverInitializationData.DxgkDdiSetVirtualMachineData = CosKmdDdi::DdiSetVirtualMachineData;

    Status = DxgkInitialize(pDriverObject, pRegistryPath, &DriverInitializationData);

    if (!NT_SUCCESS(Status))
    {
        return Status;
    }

    return Status;
}
