#include "precomp.h"
#include "RosKmdAdapter.h"
#include "RosKmdDevice.h"
#include "RosKmdAllocation.h"

void * RosKmDevice::operator new(size_t size)
{
    return ExAllocatePoolWithTag(NonPagedPoolNx, size, 'ROSD');
}

void RosKmDevice::operator delete(void * ptr)
{
    ExFreePool(ptr);
}

NTSTATUS __stdcall RosKmDevice::DdiCreateDevice(
    IN_CONST_HANDLE hAdapter,
    INOUT_PDXGKARG_CREATEDEVICE pCreateDevice)
{
    if (new RosKmDevice(hAdapter, pCreateDevice) == NULL)
    {
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
RosKmDevice::DdiOpenAllocation(
    IN_CONST_HANDLE                         hDevice,
    IN_CONST_PDXGKARG_OPENALLOCATION        pOpenAllocation)
{
    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s hDevice=%lx\n",
        __FUNCTION__, hDevice);

    RosKmDevice    *pRosKmDevice = (RosKmDevice *)hDevice;
    RosKmAdapter   *pRosKmAdapter = pRosKmDevice->m_pRosKmAdapter;

    NT_ASSERT(pOpenAllocation->PrivateDriverSize == sizeof(RosAllocationGroupExchange));
    RosAllocationGroupExchange * pRosAllocationGroupExchange = (RosAllocationGroupExchange *)pOpenAllocation->pPrivateDriverData;

    pRosAllocationGroupExchange;
    NT_ASSERT(pRosAllocationGroupExchange->m_dummy == 0);

    pOpenAllocation->Pitch;

    NT_ASSERT(pOpenAllocation->Flags.Create == true);
    NT_ASSERT(pOpenAllocation->Flags.ReadOnly == false);

    NT_ASSERT(pOpenAllocation->NumAllocations == 1);

    DXGK_OPENALLOCATIONINFO * pOpenAllocationInfo = pOpenAllocation->pOpenAllocation;

    NT_ASSERT(pOpenAllocationInfo->PrivateDriverDataSize == sizeof(RosAllocationExchange));
    RosAllocationExchange * pRosAllocationExchange = (RosAllocationExchange *)pOpenAllocationInfo->pPrivateDriverData;
    pRosAllocationExchange;

    RosKmdDeviceAllocation * pRosKmdDeviceAllocation = (RosKmdDeviceAllocation *)ExAllocatePoolWithTag(NonPagedPoolNx, sizeof(RosKmdDeviceAllocation), 'ROSD');
    if (!pRosKmdDeviceAllocation)
    {
        return STATUS_NO_MEMORY;
    }

    pRosKmdDeviceAllocation->m_hKMAllocation = pOpenAllocationInfo->hAllocation;

    DXGKARGCB_GETHANDLEDATA getHandleData;

    getHandleData.hObject = pRosKmdDeviceAllocation->m_hKMAllocation;
    getHandleData.Type = DXGK_HANDLE_ALLOCATION;
    getHandleData.Flags.DeviceSpecific = 0;

    pRosKmdDeviceAllocation->m_pRosKmdAllocation = (RosKmdAllocation *)pRosKmAdapter->GetDxgkInterface()->DxgkCbGetHandleData(&getHandleData);

    pOpenAllocationInfo->hDeviceSpecificAllocation = pRosKmdDeviceAllocation;

    return STATUS_SUCCESS;
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

    ExFreePoolWithTag(pRosKmdDeviceAllocation, 'ROSD');

    return STATUS_SUCCESS;
}
