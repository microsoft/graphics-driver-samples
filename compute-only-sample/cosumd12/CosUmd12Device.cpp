////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Device implementation
//
// Copyright (C) Microsoft Corporation
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "CosUmd12.h"

#include "CosUmd12Device.h"

//==================================================================================================================================
//
// CosUmdDevice
//
//==================================================================================================================================

CosUmd12Device::CosUmd12Device(
    CosUmd12Adapter* pAdapter,
    const D3D12DDIARG_CREATEDEVICE_0003* pArgs) :
    m_pAdapter(pAdapter),
    m_Interface(pArgs->Interface),
    m_hRTDevice(pArgs->hRTDevice),
    m_pUMCallbacks(pArgs->p12UMCallbacks_0050),
    m_pKMCallbacks(pArgs->pKTCallbacks)
{
    assert(pArgs->hDrvDevice.pDrvPrivate == (void *) this);

    assert(m_Interface == D3D12DDI_INTERFACE_VERSION_R5);

#if COS_GPUVA_SUPPORT

    m_latestPagingFenceValue = 0L;

#endif
}

void CosUmd12Device::Standup()
{
    HRESULT hr;

    memset(&m_pagingQueueDesc, 0, sizeof(m_pagingQueueDesc));

    m_pagingQueueDesc.Priority = D3DDDI_PAGINGQUEUE_PRIORITY_NORMAL;

    hr = m_pKMCallbacks->pfnCreatePagingQueueCb(m_hRTDevice.handle, &m_pagingQueueDesc);
    if (FAILED(hr))
    {
        m_pUMCallbacks->pfnSetErrorCb(m_hRTDevice, D3DDDIERR_DEVICEREMOVED);
    }

    m_curUniqueAddress = 0x100000000;

}

//----------------------------------------------------------------------------------------------------------------------------------
void CosUmd12Device::Teardown()
{
#if COS_GPUVA_SUPPORT

    D3DDDI_DESTROYPAGINGQUEUE destroyPagingQueue = {};

    destroyPagingQueue.hPagingQueue = m_pagingQueueDesc.hPagingQueue;

    m_pKMCallbacks->pfnDestroyPagingQueueCb(m_hRTDevice.handle, &destroyPagingQueue);

#endif
}

//----------------------------------------------------------------------------------------------------------------------------------
CosUmd12Device::~CosUmd12Device()
{
    // do nothing
}

void CosUmd12Device::TrackPagingFence(
    UINT64  newPagingFenceValue)
{
    if (newPagingFenceValue <= m_latestPagingFenceValue)
    {
        return;
    }

    m_latestPagingFenceValue = newPagingFenceValue;
}

void CosUmd12Device::WaitForPagingOperation(
    HANDLE  hContext)
{
    if ((*((UINT *)m_pagingQueueDesc.FenceValueCPUVirtualAddress)) >= m_latestPagingFenceValue)
    {
        return;
    }

    HRESULT hr;

    D3DDDICB_WAITFORSYNCHRONIZATIONOBJECTFROMGPU    waitFromGpu = { 0 };

    waitFromGpu.hContext = hContext;
    waitFromGpu.ObjectCount = 1;
    waitFromGpu.ObjectHandleArray = &m_pagingQueueDesc.hSyncObject;
    waitFromGpu.MonitoredFenceValueArray = &m_latestPagingFenceValue;

    hr = m_pKMCallbacks->pfnWaitForSynchronizationObjectFromGpuCb(m_hRTDevice.handle, &waitFromGpu);
    ASSERT(S_OK == hr);
}

D3D12DDI_GPU_VIRTUAL_ADDRESS CosUmd12Device::AllocateUniqueAddress(UINT size)
{
    D3D12DDI_GPU_VIRTUAL_ADDRESS curUniqueAddress = m_curUniqueAddress;

    size = (size + PAGE_SIZE - 1) & (~(PAGE_SIZE - 1));

    m_curUniqueAddress += size;

    return curUniqueAddress;
}
