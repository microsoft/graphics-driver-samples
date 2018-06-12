////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Command Queue implementation
//
// Copyright (C) Microsoft Corporation
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "CosUmd12.h"

HRESULT 
CosUmd12CommandQueue::Standup()
{
    CosContextExchange cosContextExchange;

    ZeroMemory(&m_createContext, sizeof(m_createContext));

    m_createContext.NodeOrdinal = 0;
    m_createContext.EngineAffinity = 0;
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
