////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Heap implementation
//
// Copyright (C) Microsoft Corporation
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "CosUmd12.h"

HRESULT
CosUmd12Heap::Standup()
{
    HRESULT hr = S_OK;

    CosAllocationExchange * pCosAllocationExchange = dynamic_cast<CosAllocationExchange *>(this);

    memset(pCosAllocationExchange, 0, sizeof(*pCosAllocationExchange));

    pCosAllocationExchange->m_magic = kMagic;

    pCosAllocationExchange->m_resourceDimension = D3D10DDIRESOURCE_BUFFER;
    pCosAllocationExchange->m_hwHeightPixels = 1;
    pCosAllocationExchange->m_hwWidthPixels = pCosAllocationExchange->m_hwSizeBytes = (UINT)m_desc.ByteSize;

    if (m_desc.CPUPageProperty != D3D12DDI_CPU_PAGE_PROPERTY_NOT_AVAILABLE)
    {
        pCosAllocationExchange->m_cpuVisible = 1;
    }

    auto allocate = D3D12DDICB_ALLOCATE_0022{};
    auto allocationInfo = D3D12DDI_ALLOCATION_INFO_0022{};
    {
        // allocationInfo.hAllocation - out: Private driver data for allocation
        allocationInfo.pSystemMem = nullptr;
        allocationInfo.pPrivateDriverData = pCosAllocationExchange;
        allocationInfo.PrivateDriverDataSize = sizeof(*pCosAllocationExchange);
        allocationInfo.VidPnSourceId = 0;
    }

    auto rosAllocationGroupExchange = CosAllocationGroupExchange{};

    allocate.pPrivateDriverData = &rosAllocationGroupExchange;
    allocate.PrivateDriverDataSize = sizeof(rosAllocationGroupExchange);
    allocate.hResource = m_hRTHeap.handle;
    // allocate.hKMResource - out: kernel resource handle
    allocate.NumAllocations = 1;
    allocate.pAllocationInfo = &allocationInfo;

    hr = m_pDevice->m_pUMCallbacks->pfnAllocateCb(m_pDevice->m_hRTDevice, &allocate);
    if (FAILED(hr))
    {
        return hr;
    }

    m_hKMAllocation = allocate.pAllocationInfo[0].hAllocation;

    do
    {
#if COS_GPUVA_SUPPORT

        D3DDDI_MAPGPUVIRTUALADDRESS mapGpuVa = {};

        //
        // Driver can optionally use Base/Min/Max address to position a resource's GPU VA
        //

        mapGpuVa.hPagingQueue = m_pDevice->m_pagingQueueDesc.hPagingQueue;
        mapGpuVa.hAllocation = m_hKMAllocation;

        hr = m_pDevice->m_pKMCallbacks->pfnMapGpuVirtualAddressCb(m_pDevice->m_hRTDevice.handle, &mapGpuVa);
        if ((hr == E_PENDING) || (hr == S_OK))
        {
            m_gpuVa = mapGpuVa.VirtualAddress;
            m_pagingFenceValue = mapGpuVa.PagingFenceValue;

            hr = S_OK;
        }

        if (FAILED(hr))
        {
            break;
        }

#endif

        //
        // Driver need to make internally created resources resident
        //
        if (nullptr == m_hRTHeap.handle)
        {
            D3DDDI_MAKERESIDENT makeResident = {};

            makeResident.hPagingQueue = m_pDevice->m_pagingQueueDesc.hPagingQueue;
            makeResident.NumAllocations = 1;
            makeResident.AllocationList = &m_hKMAllocation;
            makeResident.PriorityList = nullptr;
            makeResident.Flags.MustSucceed = 1;

            hr = m_pDevice->m_pKMCallbacks->pfnMakeResidentCb(m_pDevice->m_hRTDevice.handle, &makeResident);
            if ((hr == E_PENDING) || (hr == S_OK))
            {
                m_pagingFenceValue = makeResident.PagingFenceValue;

                hr = S_OK;
            }
        }
    } while (false);

    if (FAILED(hr))
    {
        D3D12DDICB_DEALLOCATE_0022 deallocate = {};

        deallocate.HandleList = &m_hKMAllocation;
        deallocate.NumAllocations = 1;

        m_pDevice->m_pUMCallbacks->pfnDeallocateCb(m_pDevice->m_hRTDevice, &deallocate);

        return hr;
    }

    m_pDevice->TrackPagingFence(m_pagingFenceValue);

#if !COS_GPUVA_SUPPORT

    m_uniqueAddress = m_pDevice->AllocateUniqueAddress(m_hwSizeBytes);

#endif

    return hr;
}

void
CosUmd12Heap::Teardown()
{
    if (m_hKMAllocation)
    {
        auto deallocate = D3D12DDICB_DEALLOCATE_0022{};

        deallocate.NumAllocations = 1;
        deallocate.HandleList = &m_hKMAllocation;

        m_pDevice->m_pUMCallbacks->pfnDeallocateCb(m_pDevice->m_hRTDevice, &deallocate);
    }
}

HRESULT
CosUmd12Heap::Map(void** pHeapData)
{
    HRESULT hr;

    D3DDDICB_LOCK2 lock2 = {};

    lock2.hAllocation = m_hKMAllocation;

    hr = m_pDevice->m_pKMCallbacks->pfnLock2Cb(m_pDevice->m_hRTDevice.handle, &lock2);

    m_pCpuAddress = (BYTE *)lock2.pData;
    *pHeapData = lock2.pData;

    return hr;
}

void
CosUmd12Heap::Unmap()
{
    HRESULT hr;

    D3DDDICB_UNLOCK2 unlock2 = {};

    unlock2.hAllocation = m_hKMAllocation;

    hr = m_pDevice->m_pKMCallbacks->pfnUnlock2Cb(m_pDevice->m_hRTDevice.handle, &unlock2);

    ASSERT(S_OK == hr);
}
