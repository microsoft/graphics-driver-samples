////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Device implementation
//
// Copyright (C) Microsoft Corporation
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "RosUmdDevice.h"
#include "RosUmdResource.h"
#include "RosUmdDebug.h"
#include "RosUmdLogging.h"
#include "RosUmdDeviceDdi.h"
#include "RosUmdRenderTargetView.h"
#include "RosUmdBlendState.h"
#include "RosUmdSampler.h"

#include "RosContext.h"

#include <exception>
#include <typeinfo>
#include <new>

//==================================================================================================================================
//
// RosUmdDevice
//
//==================================================================================================================================

RosUmdDevice::RosUmdDevice(
    RosUmdAdapter* pAdapter,
    const D3D10DDIARG_CREATEDEVICE* pArgs) :
    m_hContext(NULL),
    m_pAdapter(pAdapter),
    m_Interface(pArgs->Interface),
    m_hRTDevice(pArgs->hRTDevice),
    m_hRTCoreLayer(pArgs->hRTCoreLayer)
{
    // Location of function table for runtime callbacks. Can not change these function pointers, as they are runtime-owned;
    // but the pointer should be saved. Do not cache function pointers, as the runtime may change the table entries at will.
    m_pMSKTCallbacks = pArgs->pKTCallbacks;

    // Currently only support 11.1
    assert(m_Interface == D3D11_1_DDI_INTERFACE_VERSION);
    m_pMSUMCallbacks = pArgs->p11UMCallbacks;

    // Can change these function pointers in this table whenever the UM Driver has context. That's why the UM Driver can
    // hold onto this pointer.

    // Immediately fill out the Device function table with default methods.
    m_PipelineLevel = D3D11DDI_EXTRACT_3DPIPELINELEVEL_FROM_FLAGS(pArgs->Flags);

    m_p11_1DeviceFuncs = pArgs->p11_1DeviceFuncs;
    *m_p11_1DeviceFuncs = RosUmdDeviceDdi::s_deviceFuncs11_1;

    assert(!IS_DXGI1_3_BASE_FUNCTIONS(pArgs->Interface, pArgs->Version));
    assert(IS_DXGI1_2_BASE_FUNCTIONS(pArgs->Interface, pArgs->Version));

    *(pArgs->DXGIBaseDDI.pDXGIDDIBaseFunctions3) = RosUmdDeviceDdi::s_dxgiDeviceFuncs3;

    //... and the DXGI callbacks
    m_pDXGICallbacks = pArgs->DXGIBaseDDI.pDXGIBaseCallbacks;

}

void RosUmdDevice::Standup()
{
    RosContextExchange rosContextExchange;

    D3DDDICB_CREATECONTEXT createContext;
    ZeroMemory( &createContext, sizeof(createContext) );

    createContext.NodeOrdinal = 0;
    createContext.EngineAffinity = 0;
    createContext.Flags.Value = 0;
    createContext.pPrivateDriverData = &rosContextExchange;
    createContext.PrivateDriverDataSize = sizeof(rosContextExchange);

    HRESULT hr = m_pMSKTCallbacks->pfnCreateContextCb( m_hRTDevice.handle, &createContext);

    if( FAILED( hr ) )
    {
        assert(hr == D3DDDIERR_DEVICEREMOVED || hr == E_OUTOFMEMORY);
        throw RosUmdException(hr);
    }

    m_hContext = createContext.hContext;

    m_commandBuffer.Standup(this, &createContext);

    //
    // Standup Graphics State
    //

    memset(m_vertexBuffers, 0, sizeof(m_vertexBuffers));
    memset(m_vertexStrides, 0, sizeof(m_vertexStrides));
    memset(m_vertexOffsets, 0, sizeof(m_vertexOffsets));
    m_numVertexBuffers = 0;

    m_topology = D3D10_DDI_PRIMITIVE_TOPOLOGY_UNDEFINED;

    memset(m_viewports, 0, sizeof(m_viewports));
    m_numViewports = 0;

    memset(m_renderTargetViews, 0, sizeof(m_renderTargetViews));

    m_blendState = nullptr;

    m_pixelShader = nullptr;
    memset(m_pixelSamplers, 0, sizeof(m_pixelSamplers));

    m_vertexShader = nullptr;
    memset(m_vertexSamplers, 0, sizeof(m_vertexSamplers));

    m_domainShader = nullptr;
    memset(m_domainSamplers, 0, sizeof(m_domainSamplers));

    m_geometryShader = nullptr;
    memset(m_geometrySamplers, 0, sizeof(m_geometrySamplers));

    m_hullShader = nullptr;
    memset(m_hullSamplers, 0, sizeof(m_hullSamplers));

    m_computeShader = nullptr;
    memset(m_computeSamplers, 0, sizeof(m_computeSamplers));

    m_elementLayout = nullptr;

    m_depthStencilState = nullptr;
    m_stencilRef = 0;

}

//----------------------------------------------------------------------------------------------------------------------------------
void RosUmdDevice::Teardown()
{
    if( m_hContext != NULL )
    {
        D3DDDICB_DESTROYCONTEXT destroyContext =
        {
            m_hContext,
        };

        DestroyContext( &destroyContext );
    }

}

//----------------------------------------------------------------------------------------------------------------------------------
RosUmdDevice::~RosUmdDevice()
{
    // do nothing
}

//
// Device resource handling
//

void RosUmdDevice::CreateResource(const D3D11DDIARG_CREATERESOURCE* pCreateResource, D3D10DDI_HRESOURCE hResource, D3D10DDI_HRTRESOURCE hRTResource)
{
    RosUmdResource* pResource = new (hResource.pDrvPrivate) RosUmdResource();

    pResource->Standup(this, pCreateResource, hRTResource);

    RosAllocationExchange rosAllocationExchange;

    pResource->GetAllocationExchange(&rosAllocationExchange);

    D3DDDI_ALLOCATIONINFO allocationInfo;

    allocationInfo.Flags.Value = 0;
    allocationInfo.hAllocation = NULL;
    allocationInfo.pPrivateDriverData = &rosAllocationExchange;
    allocationInfo.PrivateDriverDataSize = sizeof(rosAllocationExchange);
    allocationInfo.pSystemMem = NULL;
    allocationInfo.VidPnSourceId = 0;

    RosAllocationGroupExchange rosAllocationGroupExchange;

    rosAllocationGroupExchange.m_dummy = 0;

    D3DDDICB_ALLOCATE allocate;

    allocate.pPrivateDriverData = &rosAllocationGroupExchange;
    allocate.PrivateDriverDataSize = sizeof(rosAllocationGroupExchange);
    allocate.hResource = hRTResource.handle;
    allocate.hKMResource = NULL;
    allocate.NumAllocations = 1;
    allocate.pAllocationInfo = &allocationInfo;

    Allocate(&allocate);

    pResource->m_hKMResource = allocate.hKMResource;
    pResource->m_hKMAllocation = allocationInfo.hAllocation;

    if (pCreateResource->pInitialDataUP != NULL && pCreateResource->pInitialDataUP[0].pSysMem != NULL)
    {
        D3DDDICB_LOCK lock;
        memset(&lock, 0, sizeof(lock));

        lock.hAllocation = pResource->m_hKMAllocation;
        lock.Flags.WriteOnly = true;
        lock.Flags.LockEntire = true;

        Lock(&lock);

        memcpy(lock.pData, pCreateResource->pInitialDataUP[0].pSysMem, pResource->m_hwSizeBytes);

        D3DDDICB_UNLOCK unlock;
        memset(&unlock, 0, sizeof(unlock));

        unlock.NumAllocations = 1;
        unlock.phAllocations = &pResource->m_hKMAllocation;

        Unlock(&unlock);
    }
}

void RosUmdDevice::DestroyResource(
    RosUmdResource * pResource)
{
    pResource->Teardown();
    pResource->~RosUmdResource();
}

void RosUmdDevice::ResourceCopy(
    RosUmdResource * pDestinationResource,
    RosUmdResource * pSourceResource)
{
    if (pDestinationResource->m_usage == D3D10_DDI_USAGE_DEFAULT &&
        pSourceResource->m_usage == D3D10_DDI_USAGE_DEFAULT)
    {
        // We can use GPU to do copy
        m_commandBuffer.CopyResource(pDestinationResource, pSourceResource);
    }
    else
    {
        // Before accessing the resources on CPU, flush if there is pending 
        // GPU operation
        m_commandBuffer.FlushIfMatching(pDestinationResource->m_mostRecentFence);
        m_commandBuffer.FlushIfMatching(pSourceResource->m_mostRecentFence);

        D3DDDICB_LOCK destinationLock;
        memset(&destinationLock, 0, sizeof(destinationLock));

        destinationLock.hAllocation = pDestinationResource->m_hKMAllocation;
        destinationLock.Flags.WriteOnly = true;
        destinationLock.Flags.LockEntire = true;

        Lock(&destinationLock);

        D3DDDICB_LOCK sourceLock;
        memset(&sourceLock, 0, sizeof(sourceLock));

        sourceLock.hAllocation = pSourceResource->m_hKMAllocation;
        sourceLock.Flags.ReadOnly = true;
        sourceLock.Flags.LockEntire = true;

        Lock(&sourceLock);

        if (pDestinationResource->m_usage == D3D10_DDI_USAGE_STAGING &&
            pSourceResource->m_usage == D3D10_DDI_USAGE_STAGING)
        {
            assert(pSourceResource->m_hwSizeBytes == pDestinationResource->m_hwSizeBytes);
            memcpy(destinationLock.pData, sourceLock.pData, pDestinationResource->m_hwSizeBytes);
        }
        else if (pDestinationResource->m_usage == D3D10_DDI_USAGE_STAGING &&
            pSourceResource->m_usage == D3D10_DDI_USAGE_DEFAULT)
        {
            // TODO(bhouse) Implement CPU side tile to linear swizzle
            assert(pSourceResource->m_hwSizeBytes >= pDestinationResource->m_hwSizeBytes);
            memcpy(destinationLock.pData, sourceLock.pData, pDestinationResource->m_hwSizeBytes);
        }
        else
        {
            // TODO(bhouse) Implement CPU side liner to tile swizzle
            assert(pSourceResource->m_hwSizeBytes <= pDestinationResource->m_hwSizeBytes);
            memcpy(destinationLock.pData, sourceLock.pData, pSourceResource->m_hwSizeBytes);
        }

        D3DDDICB_UNLOCK unlock;
        memset(&unlock, 0, sizeof(unlock));

        D3DKMT_HANDLE hAllocations[2] = { pSourceResource->m_hKMAllocation , pDestinationResource->m_hKMAllocation };

        unlock.NumAllocations = 2;
        unlock.phAllocations = hAllocations;

        Unlock(&unlock);
    }
}

void RosUmdDevice::StagingResourceMap(
    RosUmdResource * pResource,
    UINT subResource,
    D3D10_DDI_MAP mapType,
    UINT mapFlags,
    D3D10DDI_MAPPED_SUBRESOURCE* pMappedSubRes)
{
    pResource->Map(this, subResource, mapType, mapFlags, pMappedSubRes);
}

void RosUmdDevice::StagingResourceUnmap(
    RosUmdResource * pResource,
    UINT subResource)
{
    pResource->Unmap(this, subResource);
}

//
// Check calls
//

void RosUmdDevice::CheckFormatSupport(
    DXGI_FORMAT inFormat,
    UINT* pOutFormatSupport)
{
    inFormat; // unused

    *pOutFormatSupport = 0;

    *pOutFormatSupport |= D3D10_DDI_FORMAT_SUPPORT_SHADER_SAMPLE;
    *pOutFormatSupport |= D3D10_DDI_FORMAT_SUPPORT_RENDERTARGET;
    *pOutFormatSupport |= D3D10_DDI_FORMAT_SUPPORT_BLENDABLE;
    *pOutFormatSupport |= D3D11_1DDI_FORMAT_SUPPORT_VERTEX_BUFFER;
    *pOutFormatSupport |= D3D11_1DDI_FORMAT_SUPPORT_UAV_WRITES;
    *pOutFormatSupport |= D3D11_1DDI_FORMAT_SUPPORT_SHADER_GATHER;
}

void RosUmdDevice::CheckCounterInfo(
    D3D10DDI_COUNTER_INFO* pOutCounterInfo)
{
    // We do not support any counters, use "no-support" values
    static const D3D10DDI_COUNTER_INFO Info =
    {
        D3D10DDI_QUERY(D3D10DDI_COUNTER_DEVICE_DEPENDENT_0 + 3),
        3,
        1,
    };

    *pOutCounterInfo = Info;
}

void RosUmdDevice::CheckMultisampleQualityLevels(
    DXGI_FORMAT inFormat,
    UINT inSampleCount,
    UINT* pOutNumQualityLevels)
{
    inFormat; // unused
    inSampleCount; // unused

    *pOutNumQualityLevels = 0;
}

//
// Kernel mode callbacks
//

void RosUmdDevice::Allocate(D3DDDICB_ALLOCATE * pAllocate)
{
    HRESULT hr = m_pMSKTCallbacks->pfnAllocateCb(m_hRTDevice.handle, pAllocate);

    if (hr != S_OK) throw RosUmdException(hr);
}

void RosUmdDevice::Lock(D3DDDICB_LOCK * pLock)
{
    HRESULT hr = m_pMSKTCallbacks->pfnLockCb(m_hRTDevice.handle, pLock);

    if (hr != S_OK) throw RosUmdException(hr);
}

void RosUmdDevice::Unlock(D3DDDICB_UNLOCK * pUnlock)
{
    HRESULT hr = m_pMSKTCallbacks->pfnUnlockCb(m_hRTDevice.handle, pUnlock);

    if (hr != S_OK) throw RosUmdException(hr);
}

void RosUmdDevice::Render(D3DDDICB_RENDER * pRender)
{
    HRESULT hr = m_pMSKTCallbacks->pfnRenderCb(m_hRTDevice.handle, pRender);

    if (hr != S_OK) throw RosUmdException(hr);
}

void RosUmdDevice::DestroyContext(D3DDDICB_DESTROYCONTEXT * pDestroyContext)
{
    HRESULT hr = m_pMSKTCallbacks->pfnDestroyContextCb(m_hRTDevice.handle, pDestroyContext);

    if (hr != S_OK) throw RosUmdException(hr);
}

//
// User mode call backs
//

void RosUmdDevice::SetError(HRESULT hr)
{
    assert(m_pMSUMCallbacks != NULL);

    m_pMSUMCallbacks->pfnSetErrorCb(m_hRTCoreLayer, hr);
}

//
// Excepton handling support
//

void RosUmdDevice::SetException(RosUmdException & e)
{
    SetError(e.m_hr);
}

void RosUmdDevice::SetException(std::exception & e)
{
    if (typeid(e) == typeid(RosUmdException))
    {
        RosUmdException & rosUmdException = dynamic_cast<RosUmdException &>(e);

        SetError(rosUmdException.m_hr);
    }
    else
    {
        SetError(E_FAIL);
    }
}

//
// Draw Support
//

void RosUmdDevice::Draw(UINT vertexCount, UINT startVertexLocation)
{
    // Do nothing
    vertexCount; // unused
    startVertexLocation; // unused

    __debugbreak();
}

void RosUmdDevice::ClearRenderTargetView(RosUmdRenderTargetView * pRenderTargetView, FLOAT clearColor[4])
{
    pRenderTargetView; // unused
    clearColor; // unused

    // do nothing
}


//
// Graphics State Management
//

void RosUmdDevice::SetVertexBuffers(UINT startBuffer, UINT numBuffers, const D3D10DDI_HRESOURCE* phBuffers, const UINT* pStrides, const UINT* pOffsets)
{
    assert(startBuffer + numBuffers <= kMaxVertexBuffers);

    for (UINT i = 0; i < numBuffers; i++)
    {
        m_vertexBuffers[i+startBuffer] = RosUmdResource::CastFrom(phBuffers[i]);
    }
    m_numVertexBuffers = numBuffers;

    memcpy(&m_vertexStrides[startBuffer], pStrides, numBuffers * sizeof(UINT));
    memcpy(&m_vertexOffsets[startBuffer], pOffsets, numBuffers * sizeof(UINT));

}

void RosUmdDevice::SetTopology(D3D10_DDI_PRIMITIVE_TOPOLOGY topology)
{
    m_topology = topology;
}

void RosUmdDevice::SetViewports(UINT numViewports, UINT clearViewports, const D3D10_DDI_VIEWPORT* pViewports)
{
    clearViewports; // unused
    memcpy(m_viewports, pViewports, numViewports * sizeof(D3D10_DDI_VIEWPORT));
    m_numViewports = numViewports;
}

void RosUmdDevice::SetRenderTargets(const D3D10DDI_HRENDERTARGETVIEW* phRenderTargetView, UINT numRTVs, UINT rtvNumbertoUnbind,
    D3D10DDI_HDEPTHSTENCILVIEW hDepthStencilView, const D3D11DDI_HUNORDEREDACCESSVIEW* phUnorderedAccessView,
    const UINT* pUAVInitialCounts, UINT UAVIndex, UINT numUAVs, UINT UAVFirsttoSet, UINT UAVNumberUpdated)
{
    rtvNumbertoUnbind; // unused

    assert(numUAVs == 0);
    UAVNumberUpdated; // unused
    UAVFirsttoSet; // unused
    UAVIndex; // unused
    pUAVInitialCounts; // unused
    phUnorderedAccessView; // unused

    assert(hDepthStencilView.pDrvPrivate == NULL);

    for (UINT i = 0; i < numRTVs; i++)
    {
        m_renderTargetViews[i] = RosUmdRenderTargetView::CastFrom(phRenderTargetView[i]);
    }
    m_numRenderTargetViews = numRTVs;

}

void RosUmdDevice::SetBlendState(RosUmdBlendState * pBlendState, const FLOAT pBlendFactor[4], UINT sampleMask)
{
    m_blendState = pBlendState;
    memcpy(m_blendFactor, pBlendFactor, sizeof(m_blendFactor));
    m_sampleMask = sampleMask;
}

void RosUmdDevice::SetPixelShader(RosUmdShader * pShader)
{
    m_pixelShader = pShader;
}

void RosUmdDevice::SetPixelSamplers(UINT Offset, UINT NumSamplers, const D3D10DDI_HSAMPLER* phSamplers)
{
    assert(Offset + NumSamplers <= kMaxSamplers);
    for (UINT i = 0; i < NumSamplers; i++)
    {
        m_pixelSamplers[i+Offset] = RosUmdSampler::CastFrom(phSamplers[i]);
    }
}

void RosUmdDevice::SetVertexShader(RosUmdShader * pShader)
{
    m_vertexShader = pShader;
}

void RosUmdDevice::SetVertexSamplers(UINT Offset, UINT NumSamplers, const D3D10DDI_HSAMPLER* phSamplers)
{
    assert(Offset + NumSamplers <= kMaxSamplers);
    for (UINT i = 0; i < NumSamplers; i++)
    {
        m_vertexSamplers[i+Offset] = RosUmdSampler::CastFrom(phSamplers[i]);
    }
}

void RosUmdDevice::SetDomainShader(RosUmdShader * pShader)
{
    m_domainShader = pShader;
}

void RosUmdDevice::SetDomainSamplers(UINT Offset, UINT NumSamplers, const D3D10DDI_HSAMPLER* phSamplers)
{
    assert(Offset + NumSamplers <= kMaxSamplers);
    for (UINT i = 0; i < NumSamplers; i++)
    {
        m_domainSamplers[i+ Offset] = RosUmdSampler::CastFrom(phSamplers[i]);
    }
}

void RosUmdDevice::SetGeometryShader(RosUmdShader * pShader)
{
    m_geometryShader = pShader;
}

void RosUmdDevice::SetGeometrySamplers(UINT Offset, UINT NumSamplers, const D3D10DDI_HSAMPLER* phSamplers)
{
    assert(Offset + NumSamplers <= kMaxSamplers);
    for (UINT i = 0; i < NumSamplers; i++)
    {
        m_geometrySamplers[i] = RosUmdSampler::CastFrom(phSamplers[i]);
    }
}

void RosUmdDevice::SetHullShader(RosUmdShader * pShader)
{
    m_hullShader = pShader;
}

void RosUmdDevice::SetHullSamplers(UINT Offset, UINT NumSamplers, const D3D10DDI_HSAMPLER* phSamplers)
{
    assert(Offset + NumSamplers <= kMaxSamplers);
    for (UINT i = 0; i < NumSamplers; i++)
    {
        m_hullSamplers[i] = RosUmdSampler::CastFrom(phSamplers[i]);
    }
}

void RosUmdDevice::SetComputeShader(RosUmdShader * pShader)
{
    m_computeShader = pShader;
}

void RosUmdDevice::SetComputeSamplers(UINT Offset, UINT NumSamplers, const D3D10DDI_HSAMPLER* phSamplers)
{
    assert(Offset + NumSamplers <= kMaxSamplers);
    for (UINT i = 0; i < NumSamplers; i++)
    {
        m_computeSamplers[i] = RosUmdSampler::CastFrom(phSamplers[i]);
    }
}

void RosUmdDevice::SetElementLayout(RosUmdElementLayout * pElementLayout)
{
    m_elementLayout = pElementLayout;
}

void RosUmdDevice::SetDepthStencilState(RosUmdDepthStencilState * pDepthStencilState, UINT stencilRef)
{
    m_depthStencilState = pDepthStencilState;
    m_stencilRef = stencilRef;
}

void RosUmdDevice::SetRasterizerState(RosUmdRasterizerState * pRasterizerState)
{
    m_rasterizerState = pRasterizerState;
}
