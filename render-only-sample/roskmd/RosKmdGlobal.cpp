#include "precomp.h"

#include "RosKmdLogging.h"
#include "RosKmdGlobal.tmh"

#include "RosKmdGlobal.h"
#include "RosKmdDevice.h"
#include "RosKmdAdapter.h"
#include "RosKmdContext.h"
#include "RosKmdDdi.h"
#include "RosKmdUtil.h"

#include <ntverp.h>

DRIVER_OBJECT* RosKmdGlobal::s_pDriverObject;
bool RosKmdGlobal::s_bDoNotInstall = false;
size_t RosKmdGlobal::s_videoMemorySize = 0;
void * RosKmdGlobal::s_pVideoMemory = NULL;
PHYSICAL_ADDRESS RosKmdGlobal::s_videoMemoryPhysicalAddress;
bool RosKmdGlobal::s_bRenderOnly;

#if USE_SIMPENROSE

extern bool g_bUseSimPenrose;

#endif

void
RosKmdGlobal::DdiUnload(
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

    NT_ASSERT(s_pDriverObject);
    WPP_CLEANUP(s_pDriverObject);
    s_pDriverObject = nullptr;
}

void
RosKmdGlobal::DdiControlEtwLogging(
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
    return RosKmdGlobal::DriverEntry(pDriverObject, pRegistryPath);
}

NTSTATUS RosKmdGlobal::DriverEntry(__in IN DRIVER_OBJECT* pDriverObject, __in IN UNICODE_STRING* pRegistryPath)
{
    NTSTATUS    Status;
    DRIVER_INITIALIZATION_DATA DriverInitializationData;

    NT_ASSERT(!RosKmdGlobal::s_pDriverObject);
    RosKmdGlobal::s_pDriverObject = pDriverObject;

    //
    // Initialize logging
    //
    {
        WPP_INIT_TRACING(pDriverObject, pRegistryPath);
        RECORDER_CONFIGURE_PARAMS recorderConfigureParams;
        RECORDER_CONFIGURE_PARAMS_INIT(&recorderConfigureParams);
        WppRecorderConfigure(&recorderConfigureParams);
        WPP_RECORDER_LEVEL_FILTER(ROS_TRACING_VIDPN) = FALSE;
        WPP_RECORDER_LEVEL_FILTER(ROS_TRACING_PRESENT) = FALSE;
#if DBG
        WPP_RECORDER_LEVEL_FILTER(ROS_TRACING_DEFAULT) = TRUE;
#endif // DBG
    }

    ROS_LOG_INFORMATION(
        "Initializing roskmd. (pDriverObject=0x%p, pRegistryPath=%wZ)",
        pDriverObject,
        pRegistryPath);

    if (s_bDoNotInstall)
    {
        ROS_LOG_INFORMATION("s_bDoNotInstall is set; aborting driver initialization.");
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
                            MmCached);
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
            ROS_LOG_ERROR(
                "Failed to open driver registry key. (Status=%!STATUS!, pRegistryPath=%wZ, pDriverObject=0x%p)",
                Status,
                pRegistryPath,
                pDriverObject);
            return Status;
        }
        auto closeRegKey = ROS_FINALLY::Do([&]
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
                    ROS_LOG_INFORMATION("Configuring driver as render-only.");
                    s_bRenderOnly = true;
                }
            }
            else
            {
                ROS_LOG_WARNING(
                    "RenderOnly registry value was found, but is not a REG_DWORD. (valueInfo.Type=%d, valueInfo.DataLength=%d)",
                    valueInfo.Type,
                    valueInfo.DataLength);
            }
        }
        else if (Status != STATUS_OBJECT_NAME_NOT_FOUND)
        {
            ROS_LOG_ASSERTION(
                "Unexpected error occurred querying RenderOnly registry key. (Status=%!STATUS!)",
                Status);

            // an unexpected type in the registry could cause us to get here,
            // so don't stop the show.
        }
    } // RenderOnly

    //
    // Fill in the DriverInitializationData structure and call DlInitialize()
    //

    memset(&DriverInitializationData, 0, sizeof(DriverInitializationData));

    DriverInitializationData.Version = DXGKDDI_INTERFACE_VERSION;

    DriverInitializationData.DxgkDdiAddDevice = RosKmdDdi::DdiAddAdapter;
    DriverInitializationData.DxgkDdiStartDevice = RosKmdDdi::DdiStartAdapter;
    DriverInitializationData.DxgkDdiStopDevice = RosKmdDdi::DdiStopAdapter;
    DriverInitializationData.DxgkDdiRemoveDevice = RosKmdDdi::DdiRemoveAdapter;

    DriverInitializationData.DxgkDdiDispatchIoRequest = RosKmdDdi::DdiDispatchIoRequest;
    DriverInitializationData.DxgkDdiInterruptRoutine = RosKmdDdi::DdiInterruptRoutine;
    DriverInitializationData.DxgkDdiDpcRoutine = RosKmdDdi::DdiDpcRoutine;

    DriverInitializationData.DxgkDdiQueryChildRelations = RosKmdDdi::DdiQueryChildRelations;
    DriverInitializationData.DxgkDdiQueryChildStatus = RosKmdDdi::DdiQueryChildStatus;
    DriverInitializationData.DxgkDdiQueryDeviceDescriptor = RosKmdDdi::DdiQueryDeviceDescriptor;
    DriverInitializationData.DxgkDdiSetPowerState = RosKmdDdi::DdiSetPowerState;
    DriverInitializationData.DxgkDdiNotifyAcpiEvent = RosKmdDdi::DdiNotifyAcpiEvent;
    DriverInitializationData.DxgkDdiResetDevice = RosKmdDdi::DdiResetDevice;
    DriverInitializationData.DxgkDdiUnload = RosKmdGlobal::DdiUnload;
    DriverInitializationData.DxgkDdiQueryInterface = RosKmdDdi::DdiQueryInterface;
    DriverInitializationData.DxgkDdiControlEtwLogging = RosKmdGlobal::DdiControlEtwLogging;

    DriverInitializationData.DxgkDdiQueryAdapterInfo = RosKmdDdi::DdiQueryAdapterInfo;

    DriverInitializationData.DxgkDdiCreateDevice = RosKmDevice::DdiCreateDevice;

    DriverInitializationData.DxgkDdiCreateAllocation = RosKmdDdi::DdiCreateAllocation;
    DriverInitializationData.DxgkDdiDestroyAllocation = RosKmdDdi::DdiDestroyAllocation;

    DriverInitializationData.DxgkDdiDescribeAllocation = RosKmdDdi::DdiDescribeAllocation;
    DriverInitializationData.DxgkDdiGetStandardAllocationDriverData = RosKmdDdi::DdiGetStandardAllocationDriverData;

    // DriverInitializationData.DxgkDdiAcquireSwizzlingRange   = RosKmdAcquireSwizzlingRange;
    // DriverInitializationData.DxgkDdiReleaseSwizzlingRange   = RosKmdReleaseSwizzlingRange;

    DriverInitializationData.DxgkDdiOpenAllocation = RosKmdDdi::DdiOpenAllocation;
    DriverInitializationData.DxgkDdiCloseAllocation = RosKmDevice::DdiCloseAllocation;

    DriverInitializationData.DxgkDdiPatch = RosKmdDdi::DdiPatch;
    DriverInitializationData.DxgkDdiSubmitCommand = RosKmdDdi::DdiSubmitCommand;
    DriverInitializationData.DxgkDdiBuildPagingBuffer = RosKmdDdi::DdiBuildPagingBuffer;
    DriverInitializationData.DxgkDdiPreemptCommand = RosKmdDdi::DdiPreemptCommand;

    DriverInitializationData.DxgkDdiDestroyDevice = RosKmDevice::DdiDestroyDevice;

    DriverInitializationData.DxgkDdiRender = RosKmContext::DdiRender;
    DriverInitializationData.DxgkDdiPresent = RosKmdDdi::DdiPresent;
    DriverInitializationData.DxgkDdiResetFromTimeout = RosKmdDdi::DdiResetFromTimeout;
    DriverInitializationData.DxgkDdiRestartFromTimeout = RosKmdDdi::DdiRestartFromTimeout;
    DriverInitializationData.DxgkDdiEscape = RosKmdDdi::DdiEscape;
    DriverInitializationData.DxgkDdiCollectDbgInfo = RosKmdDdi::DdiCollectDbgInfo;
    DriverInitializationData.DxgkDdiQueryCurrentFence = RosKmdDdi::DdiQueryCurrentFence;
    DriverInitializationData.DxgkDdiControlInterrupt = RosKmdDdi::DdiControlInterrupt;

    DriverInitializationData.DxgkDdiCreateContext = RosKmContext::DdiCreateContext;
    DriverInitializationData.DxgkDdiDestroyContext = RosKmContext::DdiDestroyContext;

    DriverInitializationData.DxgkDdiRenderKm = RosKmdDdi::DdiRenderKm;

    //
    // Fill in DDI routines for resetting individual engine
    //
    DriverInitializationData.DxgkDdiQueryDependentEngineGroup = RosKmdDdi::DdiQueryDependentEngineGroup;
    DriverInitializationData.DxgkDdiQueryEngineStatus = RosKmdDdi::DdiQueryEngineStatus;
    DriverInitializationData.DxgkDdiResetEngine = RosKmdDdi::DdiResetEngine;

    //
    // Fill in DDI for canceling DMA buffers
    //
    DriverInitializationData.DxgkDdiCancelCommand = RosKmdDdi::DdiCancelCommand;

    //
    // Fill in DDI for component power management
    //
    DriverInitializationData.DxgkDdiSetPowerComponentFState = RosKmdDdi::DdiSetPowerComponentFState;
    DriverInitializationData.DxgkDdiPowerRuntimeControlRequest = RosKmdDdi::DdiPowerRuntimeControlRequest;

    DriverInitializationData.DxgkDdiGetNodeMetadata = RosKmdDdi::DdiGetNodeMetadata;

    DriverInitializationData.DxgkDdiSubmitCommandVirtual = RosKmdDdi::DdiSubmitCommandVirtual;

    DriverInitializationData.DxgkDdiCreateProcess = RosKmdDdi::DdiCreateProcess;
    DriverInitializationData.DxgkDdiDestroyProcess = RosKmdDdi::DdiDestroyProcess;

    DriverInitializationData.DxgkDdiCalibrateGpuClock = RosKmdDdi::DdiCalibrateGpuClock;
    DriverInitializationData.DxgkDdiSetStablePowerState = RosKmdDdi::DdiSetStablePowerState;


    //
    // Register display subsystem DDIS.
    // Refer to adapterdisplay.cxx:ADAPTER_DISPLAY::CreateDisplayCore() for
    // required DDIs.
    //
    if (!IsRenderOnly())
    {
        DriverInitializationData.DxgkDdiSetPalette = RosKmdDisplayDdi::DdiSetPalette;
        DriverInitializationData.DxgkDdiSetPointerPosition = RosKmdDisplayDdi::DdiSetPointerPosition;
        DriverInitializationData.DxgkDdiSetPointerShape = RosKmdDisplayDdi::DdiSetPointerShape;
    
        DriverInitializationData.DxgkDdiIsSupportedVidPn = RosKmdDisplayDdi::DdiIsSupportedVidPn;
        DriverInitializationData.DxgkDdiRecommendFunctionalVidPn = RosKmdDisplayDdi::DdiRecommendFunctionalVidPn;
        DriverInitializationData.DxgkDdiEnumVidPnCofuncModality = RosKmdDisplayDdi::DdiEnumVidPnCofuncModality;
        DriverInitializationData.DxgkDdiSetVidPnSourceAddress = RosKmdDisplayDdi::DdiSetVidPnSourceAddress;
        DriverInitializationData.DxgkDdiSetVidPnSourceVisibility = RosKmdDisplayDdi::DdiSetVidPnSourceVisibility;
        DriverInitializationData.DxgkDdiCommitVidPn = RosKmdDisplayDdi::DdiCommitVidPn;
        DriverInitializationData.DxgkDdiUpdateActiveVidPnPresentPath = RosKmdDisplayDdi::DdiUpdateActiveVidPnPresentPath;

        DriverInitializationData.DxgkDdiRecommendMonitorModes = RosKmdDisplayDdi::DdiRecommendMonitorModes;
        DriverInitializationData.DxgkDdiGetScanLine = RosKmdDisplayDdi::DdiGetScanLine;
        DriverInitializationData.DxgkDdiQueryVidPnHWCapability = RosKmdDisplayDdi::DdiQueryVidPnHWCapability;
        DriverInitializationData.DxgkDdiStopDeviceAndReleasePostDisplayOwnership = RosKmdDisplayDdi::DdiStopDeviceAndReleasePostDisplayOwnership;
    }

    Status = DxgkInitialize(pDriverObject, pRegistryPath, &DriverInitializationData);

    if (!NT_SUCCESS(Status))
    {
        return Status;
    }

    return Status;
}
