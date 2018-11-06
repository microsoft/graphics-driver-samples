////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Command Queue implementation
//
// Copyright (C) Microsoft Corporation
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "CosUmd12.h"

#if !COS_GPUVA_SUPPORT

HRESULT 
CosUmd12CommandQueue::Standup()
{
    CosContextExchange cosContextExchange;

    ZeroMemory(&m_createContext, sizeof(m_createContext));

    m_createContext.NodeOrdinal = 0;
    m_createContext.EngineAffinity = 1;
    m_createContext.Flags.Value = 0;
    m_createContext.pPrivateDriverData = &cosContextExchange;
    m_createContext.PrivateDriverDataSize = sizeof(cosContextExchange);

    HRESULT hr;

    //
    // Calling pfnCreateContextCb in UM callbacks
    //

    hr = m_pDevice->m_pUMCallbacks->pfnCreateContextCb(m_hRTCommandQueue, &m_createContext);

    return hr;
}

void
CosUmd12CommandQueue::Teardown()
{
    if (NULL == m_createContext.hContext)
    {
        return;
    }

    HRESULT hr;
    D3DDDICB_DESTROYCONTEXT destroyContext;

    ZeroMemory(&destroyContext, sizeof(destroyContext));

    destroyContext.hContext = m_createContext.hContext;

    hr = m_pDevice->m_pUMCallbacks->pfnDestroyContextCb(m_hRTCommandQueue, &destroyContext);
    ASSERT(S_OK == hr);
}

void
CosUmd12CommandQueue::ExecuteCommandLists(
    UINT Count,
    const D3D12DDI_HCOMMANDLIST* pCommandLists)
{
    TRACE_FUNCTION();

    for (UINT i = 0; i < Count; i++)
    {
        CosUmd12CommandList * pCommandList = CosUmd12CommandList::CastFrom(pCommandLists[i]);

        pCommandList->Execute(this);
    }
}

HRESULT
CosUmd12CommandQueue::ExecuteCommandBuffer(
    BYTE *                      pCommandBuffer,
    UINT                        commandBufferLength,
    D3DDDI_ALLOCATIONLIST *     pAllocationList,
    UINT                        numAllocations,
    D3DDDI_PATCHLOCATIONLIST *  pPatchLocationList,
    UINT                        numPatchLocations)
{
    D3DDDICB_RENDER render;

    memset(&render, 0, sizeof(render));

    render.hContext = m_createContext.hContext;

    render.BroadcastContextCount = 0;

    render.CommandLength = commandBufferLength;
    render.NumAllocations = numAllocations;
    render.NumPatchLocations = numPatchLocations;

    //
    // Copy the command buffer, allocation list and patch location list to ones provided by kernel runtime
    //

    RtlCopyMemory(m_createContext.pCommandBuffer, pCommandBuffer, commandBufferLength);
    RtlCopyMemory(m_createContext.pAllocationList, pAllocationList, numAllocations*sizeof(D3DDDI_ALLOCATIONLIST));
    RtlCopyMemory(m_createContext.pPatchLocationList, pPatchLocationList, numPatchLocations*sizeof(D3DDDI_PATCHLOCATIONLIST));

    return m_pDevice->m_pKMCallbacks->pfnRenderCb(m_pDevice->m_hRTDevice.handle, &render);
}

#endif  // !COS_GPUVA_SUPPORT
