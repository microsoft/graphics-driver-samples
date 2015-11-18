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
#include "RosUmdDepthStencilView.h"
#include "RosUmdBlendState.h"
#include "RosUmdSampler.h"
#include "RosUmdShader.h"
#include "RosUmdRasterizerState.h"
#include "RosUmdDepthStencilState.h"
#include "RosUmdElementLayout.h"
#include "RosContext.h"
#include "RosUmdUtil.h"

#include <exception>
#include <typeinfo>
#include <new>

#if VC4

#include "Vc4Hw.h"
#include "Vc4Ddi.h"

#define NV_SHADER 1

#endif

#include "math.h"

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

    m_flags.m_value = 0;
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

    //
    // Create internal dummy buffer
    //

    CreateInternalBuffer(&m_dummyBuffer, PAGE_SIZE);
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

void RosUmdDevice::CreatePixelShader(
    const UINT* pCode,
    D3D10DDI_HSHADER hShader,
    D3D10DDI_HRTSHADER hRTShader,
    const D3D11_1DDIARG_STAGE_IO_SIGNATURES* pSignatures)
{
    RosUmdPipelineShader* pPipelineShader = new (hShader.pDrvPrivate) RosUmdPipelineShader(this, D3D10_SB_PIXEL_SHADER);

    pPipelineShader->Standup(pCode, hRTShader, pSignatures);
}

void RosUmdDevice::CreateVertexShader(
    const UINT* pCode,
    D3D10DDI_HSHADER hShader,
    D3D10DDI_HRTSHADER hRTShader,
    const D3D11_1DDIARG_STAGE_IO_SIGNATURES* pSignatures)
{
    RosUmdPipelineShader* pPipelineShader = new (hShader.pDrvPrivate) RosUmdPipelineShader(this, D3D10_SB_VERTEX_SHADER);

    pPipelineShader->Standup(pCode, hRTShader, pSignatures);
}

void RosUmdDevice::CreateGeometryShader(
    const UINT* pCode,
    D3D10DDI_HSHADER hShader,
    D3D10DDI_HRTSHADER hRTShader,
    const D3D11_1DDIARG_STAGE_IO_SIGNATURES* pSignatures)
{
    RosUmdPipelineShader* pPipelineShader = new (hShader.pDrvPrivate) RosUmdPipelineShader(this, D3D10_SB_GEOMETRY_SHADER);

    pPipelineShader->Standup(pCode, hRTShader, pSignatures);
}

void RosUmdDevice::CreateComputeShader(
    const UINT* pCode,
    D3D10DDI_HSHADER hShader,
    D3D10DDI_HRTSHADER hRTShader)
{
    RosUmdPipelineShader* pPipelineShader = new (hShader.pDrvPrivate) RosUmdPipelineShader(this, D3D11_SB_COMPUTE_SHADER);

    pPipelineShader->Standup(pCode, hRTShader, NULL);
}

void RosUmdDevice::CreateTessellationShader(
    const UINT * pCode,
    D3D10DDI_HSHADER hShader,
    D3D10DDI_HRTSHADER hRTShader,
    const D3D11_1DDIARG_TESSELLATION_IO_SIGNATURES* pSignatures,
    D3D10_SB_TOKENIZED_PROGRAM_TYPE ProgramType)
{
    RosUmdTesselationShader* pTessellationShader = new (hShader.pDrvPrivate) RosUmdTesselationShader(this, ProgramType);

    pTessellationShader->Standup(pCode, hRTShader, pSignatures);
}

void RosUmdDevice::DestroyShader(
    D3D10DDI_HSHADER hShader)
{
    RosUmdShader * pShader = RosUmdShader::CastFrom(hShader);

    pShader->Teardown();
    pShader->~RosUmdShader();
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

    // Update device flag to indicate comamnd buffer has Draw call
    m_flags.m_hasDrawCall = true;
}

void RosUmdDevice::DrawIndexed(UINT indexCount, UINT startIndexLocation, INT baseVertexLocation)
{
    //
    // TODO[indyz]: Need to guarantee that Draw command goes with Start Tile Binning command
    //

    //
    // Refresh render state
    //

    RefreshPipelineState(baseVertexLocation);

    BYTE *  pCommandBuffer;
    UINT    curCommandOffset;
    D3DDDI_PATCHLOCATIONLIST *  pPatchLocation;
    UINT    allocListIndex, indexSize;

    m_commandBuffer.ReserveCommandBufferSpace(
        false,                                  // HW command
        sizeof(VC4IndexedPrimitiveList),
        &pCommandBuffer,
        1,
        1,
        &curCommandOffset,
        &pPatchLocation);

#if VC4

    VC4IndexedPrimitiveList *   pVC4IndexedPrimitiveList = (VC4IndexedPrimitiveList *)pCommandBuffer;

    *pVC4IndexedPrimitiveList = vc4IndexedPrimitiveList;

    pVC4IndexedPrimitiveList->PrimitiveMode = ConvertD3D11Topology(m_topology);

    assert(m_indexFormat == DXGI_FORMAT_R16_UINT);
    pVC4IndexedPrimitiveList->IndexType = 1;    // 16 bit index
    indexSize = 2;                              // 2 bytes

    pVC4IndexedPrimitiveList->Length = indexCount;

#if DBG
    pVC4IndexedPrimitiveList->AddressOfIndicesList = 0xDEADBEEF;
#endif

    allocListIndex = m_commandBuffer.UseResource(m_indexBuffer, false);

    m_commandBuffer.SetPatchLocation(
        pPatchLocation,
        allocListIndex,
        curCommandOffset + offsetof(VC4IndexedPrimitiveList, AddressOfIndicesList),
        0,
        startIndexLocation*indexSize + m_indexOffset);

    pVC4IndexedPrimitiveList->MaximumIndex = 0xffff;    // Maximal USHORT 

#endif

    m_commandBuffer.CommitCommandBufferSpace(sizeof(VC4IndexedPrimitiveList), 1);

    // Update device flag to indicate comamnd buffer has Draw call
    m_flags.m_hasDrawCall = true;
}

void RosUmdDevice::ClearRenderTargetView(RosUmdRenderTargetView * pRenderTargetView, FLOAT clearColor[4])
{
    // TODO[indyz]: Use pRenderTargetView to decide if 
    //              VC4ClearColors::ClearColor16 should be used
    pRenderTargetView; // unused

#if VC4

    //
    // KMD issumes Clear Colors command before Draw call
    //

    if (m_flags.m_hasDrawCall)
    {
        m_commandBuffer.Flush(0);
    }

    // Set clear color into command buffer header for KMD to generate Rendering Control List
    m_commandBuffer.UpdateClearColor(ConvertFloatColor(clearColor));

#endif
}

void RosUmdDevice::ClearDepthStencilView(RosUmdDepthStencilView * pDepthStencilView, UINT clearFlags, FLOAT depthValue, UINT8 stencilValue)
{
    pDepthStencilView;  // unused

    // TODO[indyz]: Clear depth and stencil separately
    clearFlags;         // unused;

    //
    // KMD issumes Clear Colors command before Draw call
    //

    if (m_flags.m_hasDrawCall)
    {
        m_commandBuffer.Flush(0);
    }

    // Set clear depth and stencil into command buffer header for KMD to generate Rendering Control List
    m_commandBuffer.UpdateClearDepthStencil(depthValue, stencilValue);
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

void RosUmdDevice::SetIndexBuffer(const D3D10DDI_HRESOURCE indexBuffer, DXGI_FORMAT indexFormat, UINT offset)
{
    m_indexBuffer = RosUmdResource::CastFrom(indexBuffer);
    m_indexFormat = indexFormat;
    m_indexOffset = offset;
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

#if VC4

    //
    // Flush is necessary for tile based render 
    // if there is Draw command for the previous render target
    //

    if (m_flags.m_hasDrawCall)
    {
        m_commandBuffer.Flush(0);
    }

#endif

    for (UINT i = 0; i < numRTVs; i++)
    {
        m_renderTargetViews[i] = RosUmdRenderTargetView::CastFrom(phRenderTargetView[i]);
    }
    m_numRenderTargetViews = numRTVs;

    m_depthStencilView = RosUmdDepthStencilView::CastFrom(hDepthStencilView);
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

void RosUmdDevice::RefreshPipelineState(UINT vertexOffset)
{
    RosUmdResource * pRenderTarget = (RosUmdResource *)m_renderTargetViews[0]->m_create.hDrvResource.pDrvPrivate;

    // TODO[indyz] : Update RosHwFormat
    assert(pRenderTarget->m_hwFormat == RosHwFormat::X8888);
    // TODO[indyz] : Handle T and LT tiled formats
    assert(pRenderTarget->m_hwLayout == RosHwLayout::Linear);

    //
    // Update shaders
    //
    if (m_pixelShader)
    {
        m_pixelShader->Update();
    }
    if (m_vertexShader)
    {
        m_vertexShader->Update();
    }
    if (m_domainShader)
    {
        m_domainShader->Update();
    }
    if (m_geometryShader)
    {
        m_geometryShader->Update();
    }
    if (m_hullShader)
    {
        m_hullShader->Update();
    }
    if (m_computeShader)
    {
        m_computeShader->Update();
    }

#if VC4

    BYTE *  pCommandBuffer;
    UINT    curCommandOffset;
    D3DDDI_PATCHLOCATIONLIST *  pPatchLocation;
    D3DDDI_PATCHLOCATIONLIST *  pCurPatchLocation;
    UINT    dummyAllocIndex;
    UINT    allocListIndex;

    //
    // TODO[indyz] : Decide maximal value to cover all cases
    //

    UINT    maxStateComamnds = 128;
    UINT    maxAllocationsUsed = 10;
    UINT    maxPathLocations = 16;

    m_commandBuffer.ReserveCommandBufferSpace(
        false,                                  // HW command
        maxStateComamnds,
        &pCommandBuffer,
        maxAllocationsUsed,
        maxPathLocations,
        &curCommandOffset,
        &pPatchLocation);

    pCurPatchLocation = pPatchLocation;

    dummyAllocIndex = m_commandBuffer.UseResource(&m_dummyBuffer, true);

    VC4PrimitiveListFormat *    pVC4PrimitiveListFormat;

    if (false == m_flags.m_binningStarted)
    {
        //
        // Write Tile Binning Mode Config command
        //

        VC4TileBinningModeConfig *  pVC4TileBinningModeConfig = (VC4TileBinningModeConfig *)pCommandBuffer;

        *pVC4TileBinningModeConfig = vc4TileBinningModeConfig;

#if DBG
        pVC4TileBinningModeConfig->TileAllocationMemoryAddress = 0xDEADBEEF;
#endif
        pVC4TileBinningModeConfig->TileAllocationMemorySize = VC4_TILE_ALLOCATION_MEMORY_SIZE;

#if DBG
        pVC4TileBinningModeConfig->TileStateDataArrayBaseAddress = 0xDEADBEEF;
#endif

        pVC4TileBinningModeConfig->WidthInTiles = (BYTE)pRenderTarget->m_hwWidthTiles;
        pVC4TileBinningModeConfig->HeightInTiles = (BYTE)pRenderTarget->m_hwHeightTiles;

        pVC4TileBinningModeConfig->AutoInitialiseTileStateDataArray = 1;

        // Tile allocation memory and stata data array are provided by KMD

        m_commandBuffer.SetPatchLocation(
            pCurPatchLocation,
            dummyAllocIndex,
            curCommandOffset + offsetof(VC4TileBinningModeConfig, TileAllocationMemoryAddress),
            VC4_SLOT_TILE_ALLOCATION_MEMORY);

        m_commandBuffer.SetPatchLocation(
            pCurPatchLocation,
            dummyAllocIndex,
            curCommandOffset + offsetof(VC4TileBinningModeConfig, TileStateDataArrayBaseAddress),
            VC4_SLOT_TILE_STATE_DATA_ARRAY);

        allocListIndex = m_commandBuffer.UseResource(pRenderTarget, true);

        m_commandBuffer.SetPatchLocation(
            pCurPatchLocation,
            allocListIndex,
            curCommandOffset + offsetof(VC4TileBinningModeConfig, WidthInTiles),
            VC4_SLOT_RT_BINNING_CONFIG);

        //
        // Write Start Tile Binning command
        //

        VC4StartTileBinng * pVC4StartTileBinning;
        MoveToNextCommand(pVC4TileBinningModeConfig, pVC4StartTileBinning, curCommandOffset);

        *pVC4StartTileBinning = vc4StartTileBinng;

        //
        // Indicate binning command has been written
        //

        m_flags.m_binningStarted = true;

        MoveToNextCommand(pVC4StartTileBinning, pVC4PrimitiveListFormat, curCommandOffset);
    }
    else
    {
        pVC4PrimitiveListFormat = (VC4PrimitiveListFormat *)pCommandBuffer;
    }

    //
    // Write Primitive List Format command
    // TODO[indyz] : Need to understand how this command interacts with Draw
    //

    *pVC4PrimitiveListFormat = vc4PrimitiveListFormat;

    // TODO[indyz]: Use primitive topology to set up this command
    //
    pVC4PrimitiveListFormat->PrimitiveType  = 2;    // Hard-coded to triangle
    pVC4PrimitiveListFormat->DataType       = 3;    // Hard-coded to 16 bit X/Y

    //
    // Write Clip Window command
    //

    VC4ClipWindow * pVC4ClipWindow;
    MoveToNextCommand(pVC4PrimitiveListFormat, pVC4ClipWindow, curCommandOffset);

    *pVC4ClipWindow = vc4ClipWindow;

    pVC4ClipWindow->ClipWindowLeft   = (USHORT)round(m_viewports[0].TopLeftX);
    pVC4ClipWindow->ClipWindowBottom = (USHORT)round(m_viewports[0].TopLeftY);
    pVC4ClipWindow->ClipWindowWidth  = (USHORT)round(m_viewports[0].Width);
    pVC4ClipWindow->ClipWindowHeight = (USHORT)round(m_viewports[0].Height);

    //
    // Write Configuration Bits command to update render state
    //
    // TODO[indyz]: Set up more config from rasterizer state, etc
    //

    VC4ConfigBits *  pVC4ConfigBits;
    MoveToNextCommand(pVC4ClipWindow, pVC4ConfigBits, curCommandOffset);

    *pVC4ConfigBits = vc4ConfigBits;

#if 0

    switch (m_rasterizerState->m_desc.CullMode)
    {
    case D3D10_DDI_CULL_NONE:
        pVC4ConfigBits->EnableForwardFacingPrimitive = 1;
        pVC4ConfigBits->EnableReverseFacingPrimitive = 1;
        break;
    case D3D10_DDI_CULL_FRONT:
        pVC4ConfigBits->EnableReverseFacingPrimitive = 1;
        break;
    case D3D10_DDI_CULL_BACK:
        pVC4ConfigBits->EnableForwardFacingPrimitive = 1;
        break;
    }

#else

    pVC4ConfigBits->EnableForwardFacingPrimitive = 1;
    pVC4ConfigBits->EnableReverseFacingPrimitive = 1;

#endif

    if (m_depthStencilState->m_desc.DepthEnable)
    {
        pVC4ConfigBits->EarlyZEnable = 1;

        pVC4ConfigBits->DepthTestFunction = ConvertD3D11DepthComparisonFunc(
            m_depthStencilState->m_desc.DepthFunc);

        if (m_depthStencilState->m_desc.DepthWriteMask == D3D10_DDI_DEPTH_WRITE_MASK_ALL)
        {
            pVC4ConfigBits->EarlyZUpdatesEnable = 1;
            pVC4ConfigBits->ZUpdatesEnable = 1;
        }
    }

#if NV_SHADER

    //
    // Write Viewport Offset command
    //

    VC4ViewportOffset * pVC4ViewportOffset;
    MoveToNextCommand(pVC4ConfigBits, pVC4ViewportOffset, curCommandOffset);

    *pVC4ViewportOffset = vc4ViewportOffset;

#ifdef SSR_END_DMA

    UINT vc4NVShaderStateRecordOffset = PAGE_SIZE - sizeof(VC4NVShaderStateRecord);

    VC4NVShaderStateRecord  *pVC4NVShaderStateRecord = (VC4NVShaderStateRecord *)((((PBYTE)pVC4ViewportOffset) - curCommandOffset) + vc4NVShaderStateRecordOffset);

#else

    //
    // Write Branch command to skip over Shader State Record
    //

    VC4Branch * pVC4Branch;
    MoveToNextCommand(pVC4ViewportOffset, pVC4Branch, curCommandOffset);

    UINT vc4NVShaderStateRecordOffset = curCommandOffset + sizeof(VC4Branch);
    AlignValue(vc4NVShaderStateRecordOffset, 16);

    *pVC4Branch = vc4Branch;

    m_commandBuffer.SetPatchLocation(
        pCurPatchLocation,
        dummyAllocIndex,
        curCommandOffset + offsetof(VC4Branch, BranchAddress),
        VC4_SLOT_BRANCH,
        vc4NVShaderStateRecordOffset + sizeof(VC4NVShaderStateRecord));

    //
    // Write Shader State Record for the subsequent NV Shader State command
    //

    VC4NVShaderStateRecord  *pVC4NVShaderStateRecord = (VC4NVShaderStateRecord *)(((PBYTE)pVC4Branch) + (vc4NVShaderStateRecordOffset - curCommandOffset));

#endif

    *pVC4NVShaderStateRecord = vc4NVShaderStateRecord;

    pVC4NVShaderStateRecord->FragmentShaderIsSingleThreaded = 1;
    pVC4NVShaderStateRecord->ShadedVertexDataStride = (BYTE)m_vertexStrides[0];

    // TODO[indyz] : Decide the size of Uniforms by constant buffer bound
    //
    
    pVC4NVShaderStateRecord->FragmentShaderNumberOfUniforms = 0;

    // TODO[indyz] : Need to calculate it from Input Layout Elements, etc
    //
    pVC4NVShaderStateRecord->FragmentShaderNumberOfVaryings = 3;

#if DBG
    pVC4NVShaderStateRecord->FragmentShaderCodeAddress      = 0xDEADBEEF;
    pVC4NVShaderStateRecord->FragmentShaderUniformsAddress  = 0xDEADBEEF;
    pVC4NVShaderStateRecord->ShadedVertexDataAddress        = 0xDEADBEEF;
#endif

    allocListIndex = m_commandBuffer.UseResource(m_pixelShader->GetCodeResource(), false);

    m_commandBuffer.SetPatchLocation(
        pCurPatchLocation,
        allocListIndex,
        vc4NVShaderStateRecordOffset + offsetof(VC4NVShaderStateRecord, FragmentShaderCodeAddress));

    // TODO[indyz] : Set FragmentShaderUniformsAddress to constant buffer's address
    //
    // Set the uniforms address to dummy allocation when there is not constant buffer
    // VC4 probably has read-ahead capability, it seems to hang without an valid address
    //

    m_commandBuffer.SetPatchLocation(
        pCurPatchLocation,
        dummyAllocIndex,
        vc4NVShaderStateRecordOffset + offsetof(VC4NVShaderStateRecord, FragmentShaderUniformsAddress));

    allocListIndex = m_commandBuffer.UseResource(m_vertexBuffers[0], false);

    m_commandBuffer.SetPatchLocation(
        pCurPatchLocation,
        allocListIndex,
        vc4NVShaderStateRecordOffset + offsetof(VC4NVShaderStateRecord, ShadedVertexDataAddress),
        0,
        vertexOffset*m_vertexStrides[0]);

    //
    // TODO[indyz]: Support all types of fragment shader
    //
    // Write NV Shader State command
    //
    VC4NVShaderState *  pVC4NVShaderState;

#ifdef SSR_END_DMA

    MoveToNextCommand(pVC4ViewportOffset, pVC4NVShaderState, curCommandOffset);

#else

    // Update the current command offset
    curCommandOffset = vc4NVShaderStateRecordOffset;

    MoveToNextCommand(pVC4NVShaderStateRecord, pVC4NVShaderState, curCommandOffset);

#endif

    *pVC4NVShaderState = vc4NVShaderState;
#if DBG
    pVC4NVShaderState->ShaderRecordAddress = 0xDEADBEEF;
#endif

    // Dummy allocation is used in place of DMA buffer
    // Allocation Offset is NV Shader State Record's offset with the DMA buffer

    m_commandBuffer.SetPatchLocation(
        pCurPatchLocation,
        dummyAllocIndex,
        curCommandOffset + offsetof(VC4NVShaderState, ShaderRecordAddress),
        VC4_SLOT_NV_SHADER_STATE,
        vc4NVShaderStateRecordOffset);

    //
    // Commit the written state commands
    //

    UINT commandsWritten   = (UINT)(((PBYTE)(pVC4NVShaderState + 1)) - pCommandBuffer);
    UINT patchLocationUsed = (UINT)(pCurPatchLocation - pPatchLocation);

    assert(commandsWritten <= maxStateComamnds);
    assert(patchLocationUsed <= maxPathLocations);

    m_commandBuffer.CommitCommandBufferSpace(
        commandsWritten,
        patchLocationUsed);

#else

    //
    // Write Viewport Offset command
    //

    VC4ViewportOffset * pVC4ViewportOffset;
    MoveToNextCommand(pVC4ConfigBits, pVC4ViewportOffset, curCommandOffset);

    *pVC4ViewportOffset = vc4ViewportOffset;

    pVC4ViewportOffset->ViewportCenterX = (SHORT)(m_viewports[0].Width / 2.0f);
    pVC4ViewportOffset->ViewportCenterY = (SHORT)(m_viewports[0].Height / 2.0f);

    //
    // Write Clipper XY Scaling command
    //

    VC4ClipperXYScaling *   pVC4ClipperXYScaling;
    MoveToNextCommand(pVC4ViewportOffset, pVC4ClipperXYScaling, curCommandOffset);

    *pVC4ClipperXYScaling = vc4ClipperXYScaling;

    pVC4ClipperXYScaling->ViewportHalfWidth = m_viewports[0].Width / 2.0f * 16.0f;
    pVC4ClipperXYScaling->ViewportHalfHeight = -m_viewports[0].Height / 2.0f * 16.0f;

    //
    // Write Clipper Z Scale and Offset command
    //

    VC4ClipperZScaleAndOffset * pVC4ClipperZScaleAndOffset;
    MoveToNextCommand(pVC4ClipperXYScaling, pVC4ClipperZScaleAndOffset, curCommandOffset);

    *pVC4ClipperZScaleAndOffset = vc4ClipperZScaleAndOffset;

    pVC4ClipperZScaleAndOffset->ViewportZOffset = (m_viewports[0].MaxDepth - m_viewports[0].MinDepth) / 2.0f;
    pVC4ClipperZScaleAndOffset->ViewportZScale = pVC4ClipperZScaleAndOffset->ViewportZOffset;

#ifdef SSR_END_DMA

    UINT vc4GLShaderStateRecordOffset = PAGE_SIZE - sizeof(VC4GLShaderStateRecord);

    VC4GLShaderStateRecord  *pVC4GLShaderStateRecord = (VC4GLShaderStateRecord *)((((PBYTE)pVC4ViewportOffset) - curCommandOffset) + vc4GLShaderStateRecordOffset);

#else

    //
    // Write Branch command to skip over Shader State Record
    //

    VC4Branch * pVC4Branch;
    MoveToNextCommand(pVC4ClipperZScaleAndOffset, pVC4Branch, curCommandOffset);

    UINT vc4GLShaderStateRecordOffset = curCommandOffset + sizeof(VC4Branch);
    AlignValue(vc4GLShaderStateRecordOffset, 16);

    *pVC4Branch = vc4Branch;

    m_commandBuffer.SetPatchLocation(
        pCurPatchLocation,
        dummyAllocIndex,
        curCommandOffset + offsetof(VC4Branch, BranchAddress),
        VC4_SLOT_BRANCH,
        vc4GLShaderStateRecordOffset + sizeof(VC4GLShaderStateRecord) + m_elementLayout->m_numElements*sizeof(VC4VertexAttribute));

    //
    // Write Shader State Record for the subsequent NV Shader State command
    //

    VC4GLShaderStateRecord  *pVC4GLShaderStateRecord = (VC4GLShaderStateRecord *)(((PBYTE)pVC4Branch) + (vc4GLShaderStateRecordOffset - curCommandOffset));

#endif

    *pVC4GLShaderStateRecord = vc4GLShaderStateRecord;

#if 0

    pVC4GLShaderStateRecord->EnableClipping = 1;

#endif
    
    pVC4GLShaderStateRecord->FragmentShaderIsSingleThreaded = 1;

    //
    // Calculate Number Of Varyings from pixel shader signature
    //

    D3D11_1DDIARG_SIGNATURE_ENTRY * pInputEntry;
    UINT    numEntries = m_pixelShader->m_pCompiler->GetInputSignature(&pInputEntry);
    BYTE    numVaryings = 0;

    for (UINT i = 0; i < numEntries; i++,pInputEntry++)
    {
        if (pInputEntry->SystemValue != D3D10_SB_NAME_UNDEFINED)
        {
            continue;
        }

        for (UINT j = 0; j < 4; j++)
        {
            if (pInputEntry->Mask & (1 << j))
            {
                numVaryings++;
            }
        }
    }

    pVC4GLShaderStateRecord->FragmentShaderNumberOfVaryings = numVaryings;

#if DBG
    pVC4GLShaderStateRecord->FragmentShaderCodeAddress      = 0xDEADBEEF;
    pVC4GLShaderStateRecord->FragmentShaderUniformsAddress  = 0xDEADBEEF;
#endif

    allocListIndex = m_commandBuffer.UseResource(m_pixelShader->GetCodeResource(), false);

    m_commandBuffer.SetPatchLocation(
        pCurPatchLocation,
        allocListIndex,
        vc4GLShaderStateRecordOffset + offsetof(VC4GLShaderStateRecord, FragmentShaderCodeAddress));

    // TODO[indyz] : Set FragmentShaderUniformsAddress to constant buffer's address
    //
    // Set the uniforms address to dummy allocation when there is not constant buffer
    // VC4 probably has read-ahead capability, it seems to hang without an valid address
    //

    m_commandBuffer.SetPatchLocation(
        pCurPatchLocation,
        dummyAllocIndex,
        vc4GLShaderStateRecordOffset + offsetof(VC4GLShaderStateRecord, FragmentShaderUniformsAddress));

#if DBG
    pVC4GLShaderStateRecord->VertexShaderCodeAddress        = 0xDEADBEEF;
    pVC4GLShaderStateRecord->VertexShaderUniformsAddress    = 0xDEADBEEF;
#endif

    allocListIndex = m_commandBuffer.UseResource(m_vertexShader->GetCodeResource(), false);

    m_commandBuffer.SetPatchLocation(
        pCurPatchLocation,
        allocListIndex,
        vc4GLShaderStateRecordOffset + offsetof(VC4GLShaderStateRecord, VertexShaderCodeAddress));

    // TODO[indyz] : Set VertexShaderUniformsAddress to constant buffer's address
    //

    m_commandBuffer.SetPatchLocation(
        pCurPatchLocation,
        dummyAllocIndex,
        vc4GLShaderStateRecordOffset + offsetof(VC4GLShaderStateRecord, VertexShaderUniformsAddress));
    
#if DBG
    pVC4GLShaderStateRecord->CoordinateShaderCodeAddress        = 0xDEADBEEF;
    pVC4GLShaderStateRecord->CoordinateShaderUniformsAddress    = 0xDEADBEEF;
#endif

    allocListIndex = m_commandBuffer.UseResource(m_vertexShader->GetCodeResource(), false);

    m_commandBuffer.SetPatchLocation(
        pCurPatchLocation,
        allocListIndex,
        vc4GLShaderStateRecordOffset + offsetof(VC4GLShaderStateRecord, CoordinateShaderCodeAddress),
        0,
        m_vertexShader->m_vc4CoordinateShaderOffset);

    // TODO[indyz] : Set CoordinateShaderUniformsAddress to constant buffer's address
    //

    m_commandBuffer.SetPatchLocation(
        pCurPatchLocation,
        dummyAllocIndex,
        vc4GLShaderStateRecordOffset + offsetof(VC4GLShaderStateRecord, CoordinateShaderUniformsAddress));

    curCommandOffset = vc4GLShaderStateRecordOffset;

    VC4VertexAttribute *    pVC4VertexAttribute;
    MoveToNextCommand(pVC4GLShaderStateRecord, pVC4VertexAttribute, curCommandOffset);

    D3D10DDIARG_INPUT_ELEMENT_DESC *    pElementDesc = m_elementLayout->m_pElementDesc;
    BYTE    vpmOffset = 0;
    BYTE    elementBytes;

    for (UINT i = 0; i < m_elementLayout->m_numElements; i++)
    {
#if DBG
        pVC4VertexAttribute->VertexBaseMemoryAddress = 0xDEADBEEF;
#endif

        elementBytes = (BYTE)CPixel::BytesPerPixel(pElementDesc[i].Format);

        pVC4VertexAttribute->NumberOfBytesMinusOne = elementBytes - 1;
        pVC4VertexAttribute->VertexShaderVPMOffset = vpmOffset;
        pVC4VertexAttribute->CoordinateShaderVPMOffset = vpmOffset;

        allocListIndex = m_commandBuffer.UseResource(m_vertexBuffers[pElementDesc[i].InputSlot], false);

        m_commandBuffer.SetPatchLocation(
            pCurPatchLocation,
            allocListIndex,
            curCommandOffset + offsetof(VC4VertexAttribute, VertexBaseMemoryAddress),
            0,
            vertexOffset*m_vertexStrides[pElementDesc[i].InputSlot] + pElementDesc[i].AlignedByteOffset);

        vpmOffset += elementBytes;
        MoveToNextCommand(pVC4VertexAttribute, pVC4VertexAttribute, curCommandOffset);
    }

    //
    // Set the Total Attributes Size and Attribute Array Select Bits
    //
    pVC4GLShaderStateRecord->VertexShaderAttributeArraySelectBits = (1 << m_elementLayout->m_numElements) - 1;
    pVC4GLShaderStateRecord->VertexShaderTotalAttributesSize = vpmOffset;

    pVC4GLShaderStateRecord->CoordinateShaderAttributeArraySelectBits = (1 << m_elementLayout->m_numElements) - 1;
    pVC4GLShaderStateRecord->CoordinateShaderTotalAttributesSize = vpmOffset;

    //
    // Write GL Shader State command
    //
    VC4GLShaderState *  pVC4GLShaderState;

#ifdef SSR_END_DMA

    MoveToNextCommand(pVC4ViewportOffset, pVC4GLShaderState, curCommandOffset);

#else

    pVC4GLShaderState = (VC4GLShaderState *)pVC4VertexAttribute;

#endif

    *pVC4GLShaderState = vc4GLShaderState;

    pVC4GLShaderState->NumberOfAttributeArrays = m_elementLayout->m_numElements;

    //
    // TODO[indyz]: Need to understand when Extended Shader Record is used
    //

    pVC4GLShaderState->ExtendedShaderRecord = 0;

#if DBG
    pVC4GLShaderState->ShaderRecordAddress = 0xDEADBEE;
#endif

    // Dummy allocation is used in place of DMA buffer
    //
    // Allocation Offset is GL Shader State Record's offset within the DMA buffer
    //
    // NumberOfAttributeArrays and ExtendedShaderRecord are in the allocation offset

    m_commandBuffer.SetPatchLocation(
        pCurPatchLocation,
        dummyAllocIndex,
        curCommandOffset + offsetof(VC4GLShaderState, UInt1),
        VC4_SLOT_GL_SHADER_STATE,
        vc4GLShaderStateRecordOffset + pVC4GLShaderState->NumberOfAttributeArrays + pVC4GLShaderState->ExtendedShaderRecord);

    //
    // Commit the written state commands
    //

    UINT commandsWritten   = (UINT)(((PBYTE)(pVC4GLShaderState + 1)) - pCommandBuffer);
    UINT patchLocationUsed = (UINT)(pCurPatchLocation - pPatchLocation);

    assert(commandsWritten <= maxStateComamnds);
    assert(patchLocationUsed <= maxPathLocations);

    m_commandBuffer.CommitCommandBufferSpace(
        commandsWritten,
        patchLocationUsed);

#endif

#endif

}

void RosUmdDevice::WriteEpilog()
{
    PBYTE   pCommandBuffer;

    m_commandBuffer.ReserveCommandBufferSpace(
        false,
        3,
        &pCommandBuffer);

    //
    // Write Flush All State, NOP and Halt commands
    //
    pCommandBuffer[0] = VC4_CMD_FLUSH_ALL_STATE;
    pCommandBuffer[1] = VC4_CMD_NOP;
    pCommandBuffer[2] = VC4_CMD_HALT;

    m_commandBuffer.CommitCommandBufferSpace(3);

    //
    // Clear up state flag
    //
    m_flags.m_binningStarted = false;
    m_flags.m_hasDrawCall    = false;
}

void RosUmdDevice::CreateInternalBuffer(RosUmdResource * pRes, UINT size)
{
    D3D10DDI_MIPINFO            mipInfo = { size, 1, 1, size, 1, 1 };
    D3D11DDIARG_CREATERESOURCE  createResource = { 0 };

    createResource.pMipInfoList = &mipInfo;
    createResource.ResourceDimension = D3D10DDIRESOURCE_BUFFER;

    createResource.Usage = 0;
    createResource.Format = DXGI_FORMAT_UNKNOWN;
    createResource.ArraySize = 1;

    CreateResource(
        &createResource,
        MAKE_D3D10DDI_HRESOURCE(pRes),
        MAKE_D3D10DDI_HRTRESOURCE(NULL));
}

