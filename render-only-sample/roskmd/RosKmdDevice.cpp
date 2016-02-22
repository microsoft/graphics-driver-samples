#include "precomp.h"

#include "RosKmdLogging.h"
#include "RosKmdDevice.tmh"

#include "RosKmdAdapter.h"
#include "RosKmdDevice.h"
#include "RosKmdAllocation.h"
#include "RosKmdUtil.h"

NTSTATUS __stdcall RosKmDevice::DdiCreateDevice(
    IN_CONST_HANDLE hAdapter,
    INOUT_PDXGKARG_CREATEDEVICE pCreateDevice)
{
    pCreateDevice->hDevice = new (NonPagedPoolNx, ROS_ALLOC_TAG::DEVICE)
        RosKmDevice(hAdapter, pCreateDevice);
    if (!pCreateDevice->hDevice)
    {
        ROS_LOG_LOW_MEMORY("Failed to allocate RosKmDevice.");
        return STATUS_NO_MEMORY;
    }

    return STATUS_SUCCESS;
}

NTSTATUS __stdcall RosKmDevice::DdiDestroyDevice(
    IN_CONST_HANDLE     hDevice)
{
    RosKmDevice   *pRosKmDevice = (RosKmDevice *)hDevice;

    delete pRosKmDevice;

    return STATUS_SUCCESS;
}

RosKmDevice::RosKmDevice(IN_CONST_HANDLE hAdapter, INOUT_PDXGKARG_CREATEDEVICE pCreateDevice)
{
    m_hRTDevice = pCreateDevice->hDevice;
    m_pRosKmAdapter = (RosKmAdapter *)hAdapter;
    m_Flags = pCreateDevice->Flags;
    
    pCreateDevice->hDevice = this;
}

RosKmDevice::~RosKmDevice()
{
    // do nothing
}

NTSTATUS
__stdcall
RosKmDevice::DdiCloseAllocation(
    IN_CONST_HANDLE                     hDevice,
    IN_CONST_PDXGKARG_CLOSEALLOCATION   pCloseAllocation)
{
    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s hDevice=%lx\n",
        __FUNCTION__, hDevice);

    RosKmDevice   *pRosKmDevice = (RosKmDevice *)hDevice;
    pRosKmDevice;

    NT_ASSERT(pCloseAllocation->NumAllocations == 1);

    RosKmdDeviceAllocation * pRosKmdDeviceAllocation = (RosKmdDeviceAllocation *)pCloseAllocation->pOpenHandleList[0];

    ExFreePoolWithTag(pRosKmdDeviceAllocation, ROS_ALLOC_TAG::DEVICE);

    return STATUS_SUCCESS;
}

ROS_PAGED_SEGMENT_BEGIN; //===================================================

_Use_decl_annotations_
NTSTATUS RosKmDevice::OpenAllocation (const DXGKARG_OPENALLOCATION* ArgsPtr)
{
    PAGED_CODE();
    ROS_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    const DXGKRNL_INTERFACE& dxgkInterface = *m_pRosKmAdapter->GetDxgkInterface();

    for (UINT i = 0; i < ArgsPtr->NumAllocations; i++)
    {
        DXGK_OPENALLOCATIONINFO* openAllocInfoPtr = ArgsPtr->pOpenAllocation + i;

        RosKmdAllocation* rosKmdAllocationPtr;
        {
            DXGKARGCB_GETHANDLEDATA getHandleData;
            getHandleData.hObject = openAllocInfoPtr->hAllocation;
            getHandleData.Type = DXGK_HANDLE_ALLOCATION;
            getHandleData.Flags.DeviceSpecific = 0;
            rosKmdAllocationPtr = static_cast<RosKmdAllocation*>(
                dxgkInterface.DxgkCbGetHandleData(&getHandleData));
        }

        RosKmdDeviceAllocation* rosKmdDeviceAllocationPtr;
        {
            // TODO[jordanrh] this structure can probably be paged
            rosKmdDeviceAllocationPtr = new (NonPagedPoolNx, ROS_ALLOC_TAG::DEVICE)
                    RosKmdDeviceAllocation();
            if (!rosKmdDeviceAllocationPtr)
            {
                ROS_LOG_LOW_MEMORY(
                    "Failed to allocate memory for RosKmdDeviceAllocation structure. (sizeof(RosKmdDeviceAllocation)=%d)",
                    sizeof(RosKmdDeviceAllocation));
                return STATUS_NO_MEMORY;
            }

            rosKmdDeviceAllocationPtr->m_hKMAllocation = openAllocInfoPtr->hAllocation;
            rosKmdDeviceAllocationPtr->m_pRosKmdAllocation = rosKmdAllocationPtr;
        }

        // Return the per process allocation info
        openAllocInfoPtr->hDeviceSpecificAllocation = rosKmdDeviceAllocationPtr;
    }

    ROS_LOG_TRACE(
        "Successfully opened allocation. (Flags.Create=%d, Flags.ReadOnly=%d)",
        ArgsPtr->Flags.Create,
        ArgsPtr->Flags.ReadOnly);

    return STATUS_SUCCESS;
}

ROS_PAGED_SEGMENT_END; //=====================================================