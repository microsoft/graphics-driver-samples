#include "RosKmdGlobal.h"
#include "RosKmdDevice.h"
#include "RosKmdAdapter.h"
#include "RosKmdContext.h"
#include "RosKmdDdi.h"

#include "ntverp.h"

bool RosKmdGlobal::s_bDoNotInstall = false;
size_t RosKmdGlobal::s_videoMemorySize = 0;
void * RosKmdGlobal::s_pVideoMemory = NULL;
PHYSICAL_ADDRESS RosKmdGlobal::s_videoMemoryPhysicalAddress;

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

    // Only break into the debugger on driver entry if debugger is present
    if (KdRefreshDebuggerNotPresent() == FALSE)
    {
        DbgBreakPoint();
    }

    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "DriverEntry\n");

    if (s_bDoNotInstall)
    {
        return STATUS_NO_MEMORY;
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

    MEMORY_CACHING_TYPE CacheType = MmNonCached;
    
#if USE_SIMPENROSE

    if (g_bUseSimPenrose)
    {
        CacheType = MmCached;
    }

#endif

    while (s_pVideoMemory == NULL)
    {
        //
        // The allocated contiguous memory is as NonCached
        // TODO[indyz]: Evaluate if cached memory and flush is a better option
        //

        s_pVideoMemory = MmAllocateContiguousMemorySpecifyCache(
                            s_videoMemorySize,
                            lowestAcceptableAddress,
                            highestAcceptableAddress,
                            boundaryAddressMultiple,
                            CacheType);
        if (s_pVideoMemory != NULL)
        {
            break;
        }

        s_videoMemorySize >>= 1;
    }

    s_videoMemoryPhysicalAddress = MmGetPhysicalAddress(s_pVideoMemory);

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

    DriverInitializationData.DxgkDdiOpenAllocation = RosKmDevice::DdiOpenAllocation;
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

    Status = DxgkInitialize(pDriverObject, pRegistryPath, &DriverInitializationData);

    if (!NT_SUCCESS(Status))
    {
        return Status;
    }

    return Status;
}
