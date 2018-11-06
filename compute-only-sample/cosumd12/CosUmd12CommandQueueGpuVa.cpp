////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Command Queue implementation
//
// Copyright (C) Microsoft Corporation
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "CosUmd12.h"

#if COS_GPUVA_SUPPORT

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

    hr = m_pDevice->m_pUMCallbacks->pfnCreateContextVirtualCb(m_hRTCommandQueue, &m_createContext);

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
    D3DGPU_VIRTUAL_ADDRESS  commandBufferGpuVa,
    UINT                    commandBufferLength,
    BYTE *                  pCommamndBuffer)
{
    m_pDevice->WaitForPagingOperation(m_createContext.hContext);

    D3DDDICB_SUBMITCOMMAND submitCommand;

    memset(&submitCommand, 0, sizeof(submitCommand));

    submitCommand.Commands = commandBufferGpuVa;
    submitCommand.CommandLength = commandBufferLength;

    submitCommand.BroadcastContextCount = 1;
    submitCommand.BroadcastContext[0] = m_createContext.hContext;

    //
    // Passing the UM command buffer pointer to KMD for debugging purpose only
    //

    submitCommand.pPrivateDriverData = &pCommamndBuffer;
    submitCommand.PrivateDriverDataSize = sizeof(pCommamndBuffer);

    return m_pDevice->m_pKMCallbacks->pfnSubmitCommandCb(m_pDevice->m_hRTDevice.handle, &submitCommand);
}

#endif  // COS_GPUVA_SUPPORT

