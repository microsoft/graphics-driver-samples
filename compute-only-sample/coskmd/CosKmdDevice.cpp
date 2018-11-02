#include "CosKmd.h"

#include "CosKmdLogging.h"
#include "CosKmdDevice.tmh"

#include "CosKmdAdapter.h"
#include "CosKmdDevice.h"
#include "CosKmdAllocation.h"
#include "CosKmdUtil.h"

// GPUVA_INIT_DDI_7
NTSTATUS __stdcall CosKmDevice::DdiCreateDevice(
    IN_CONST_HANDLE hAdapter,
    INOUT_PDXGKARG_CREATEDEVICE pCreateDevice)
{
    pCreateDevice->hDevice = new (NonPagedPoolNx, COS_ALLOC_TAG::DEVICE)
        CosKmDevice(hAdapter, pCreateDevice);
    if (!pCreateDevice->hDevice)
    {
        COS_LOG_LOW_MEMORY("Failed to allocate CosKmDevice.");
        return STATUS_NO_MEMORY;
    }

#if COS_GPUVA_SUPPORT

    CosKmDevice *   pCosKmDevice = (CosKmDevice *)pCreateDevice->hDevice;

    pCosKmDevice->m_pKmdProcess = (CosKmdProcess *)pCreateDevice->hKmdProcess;

#endif

    return STATUS_SUCCESS;
}

NTSTATUS __stdcall CosKmDevice::DdiDestroyDevice(
    IN_CONST_HANDLE     hDevice)
{
    CosKmDevice   *pCosKmDevice = (CosKmDevice *)hDevice;

    delete pCosKmDevice;

    return STATUS_SUCCESS;
}

CosKmDevice::CosKmDevice(IN_CONST_HANDLE hAdapter, INOUT_PDXGKARG_CREATEDEVICE pCreateDevice)
{
    m_hRTDevice = pCreateDevice->hDevice;
    m_pCosKmAdapter = (CosKmAdapter *)hAdapter;
    m_Flags = pCreateDevice->Flags;
    
    pCreateDevice->hDevice = this;
}

CosKmDevice::~CosKmDevice()
{
    // do nothing
}

NTSTATUS
__stdcall
CosKmDevice::DdiCloseAllocation(
    IN_CONST_HANDLE                     hDevice,
    IN_CONST_PDXGKARG_CLOSEALLOCATION   pCloseAllocation)
{
    DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_TRACE_LEVEL, "%s hDevice=%lx\n",
        __FUNCTION__, hDevice);

    CosKmDevice   *pCosKmDevice = (CosKmDevice *)hDevice;
    pCosKmDevice;

    NT_ASSERT(pCloseAllocation->NumAllocations == 1);

    CosKmdDeviceAllocation * pCosKmdDeviceAllocation = (CosKmdDeviceAllocation *)pCloseAllocation->pOpenHandleList[0];

    ExFreePoolWithTag(pCosKmdDeviceAllocation, COS_ALLOC_TAG::DEVICE);

    return STATUS_SUCCESS;
}

COS_PAGED_SEGMENT_BEGIN; //===================================================

_Use_decl_annotations_
NTSTATUS CosKmDevice::OpenAllocation (const DXGKARG_OPENALLOCATION* ArgsPtr)
{
    PAGED_CODE();
    COS_ASSERT_MAX_IRQL(PASSIVE_LEVEL);

    const DXGKRNL_INTERFACE& dxgkInterface = *m_pCosKmAdapter->GetDxgkInterface();

    for (UINT i = 0; i < ArgsPtr->NumAllocations; i++)
    {
        DXGK_OPENALLOCATIONINFO* openAllocInfoPtr = ArgsPtr->pOpenAllocation + i;

        CosKmdAllocation* rosKmdAllocationPtr;
        {
            DXGKARGCB_GETHANDLEDATA getHandleData;
            DXGKARG_RELEASE_HANDLE hReleaseHandle;

            getHandleData.hObject = openAllocInfoPtr->hAllocation;
            getHandleData.Type = DXGK_HANDLE_ALLOCATION;
            getHandleData.Flags.DeviceSpecific = 0;

            rosKmdAllocationPtr = static_cast<CosKmdAllocation*>(
                dxgkInterface.DxgkCbAcquireHandleData(&getHandleData, &hReleaseHandle));

            DXGKARGCB_RELEASEHANDLEDATA releaseHandleData;

            releaseHandleData.ReleaseHandle = hReleaseHandle;
            releaseHandleData.Type = DXGK_HANDLE_ALLOCATION;

            dxgkInterface.DxgkCbReleaseHandleData(releaseHandleData);
        }

        CosKmdDeviceAllocation* rosKmdDeviceAllocationPtr;
        {
            // TODO[jordanrh] this structure can probably be paged
            rosKmdDeviceAllocationPtr = new (NonPagedPoolNx, COS_ALLOC_TAG::DEVICE)
                    CosKmdDeviceAllocation();
            if (!rosKmdDeviceAllocationPtr)
            {
                COS_LOG_LOW_MEMORY(
                    "Failed to allocate memory for CosKmdDeviceAllocation structure. (sizeof(CosKmdDeviceAllocation)=%d)",
                    sizeof(CosKmdDeviceAllocation));
                return STATUS_NO_MEMORY;
            }

            rosKmdDeviceAllocationPtr->m_hKMAllocation = openAllocInfoPtr->hAllocation;
            rosKmdDeviceAllocationPtr->m_pCosKmdAllocation = rosKmdAllocationPtr;
        }

        // Return the per process allocation info
        openAllocInfoPtr->hDeviceSpecificAllocation = rosKmdDeviceAllocationPtr;
    }

    COS_LOG_TRACE(
        "Successfully opened allocation. (Flags.Create=%d, Flags.ReadOnly=%d)",
        ArgsPtr->Flags.Create,
        ArgsPtr->Flags.ReadOnly);

    return STATUS_SUCCESS;
}

COS_PAGED_SEGMENT_END; //=====================================================
