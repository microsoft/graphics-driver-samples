////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Device implementation
//
// Copyright (C) Microsoft Corporation
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "precomp.h"

#include "RosUmdLogging.h"
#include "RosUmdDevice.tmh"

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
#include "RosUmdShaderResourceView.h"
#include "RosUmdRasterizerState.h"
#include "RosUmdDepthStencilState.h"
#include "RosUmdElementLayout.h"
#include "RosContext.h"
#include "RosUmdUtil.h"

#if VC4

#include "Vc4Hw.h"
#include "Vc4Ddi.h"

// #define NV_SHADER 1

#endif

static
BOOLEAN _IntersectRect(RECT* CONST pDst, RECT CONST* CONST pSrc1, RECT CONST* CONST pSrc2)
{
    pDst->left = max(pSrc1->left, pSrc2->left);
    pDst->right = min(pSrc1->right, pSrc2->right);

    //
    // Left must be less than right for a rect intersection.  Otherwise pDst
    // is either an empty rect (left == right), or an invalid rect (left > right).
    //
    if (pDst->left < pDst->right)
    {
        pDst->top = max(pSrc1->top, pSrc2->top);
        pDst->bottom = min(pSrc1->bottom, pSrc2->bottom);

        //
        // Top must be less than bottom for a rect intersection.  Otherwise pDst
        // is either an empty rect (top == bottom), or an invalid rect (top > bottom).
        //
        if (pDst->top < pDst->bottom)
        {
            return TRUE;
        }
    }

    return FALSE;
}

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
    m_hRTCoreLayer(pArgs->hRTCoreLayer),
    m_bPredicateValue(FALSE)
{
    // Location of function table for runtime callbacks. Can not change these function pointers, as they are runtime-owned;
    // but the pointer should be saved. Do not cache function pointers, as the runtime may change the table entries at will.
    m_pMSKTCallbacks = pArgs->pKTCallbacks;

    // Currently only support WDDM1.3 DDI
    assert(m_Interface == D3DWDDM1_3_DDI_INTERFACE_VERSION);
    m_pMSUMCallbacks = pArgs->p11UMCallbacks;

    // Can change these function pointers in this table whenever the UM Driver has context. That's why the UM Driver can
    // hold onto this pointer.

    // Immediately fill out the Device function table with default methods.
    m_PipelineLevel = D3D11DDI_EXTRACT_3DPIPELINELEVEL_FROM_FLAGS(pArgs->Flags);

    m_pWDDM1_3DeviceFuncs = pArgs->pWDDM1_3DeviceFuncs;
    *m_pWDDM1_3DeviceFuncs = RosUmdDeviceDdi::s_deviceFuncsWDDM1_3;

    assert(IS_DXGI1_3_BASE_FUNCTIONS(pArgs->Interface, pArgs->Version));

    *(pArgs->DXGIBaseDDI.pDXGIDDIBaseFunctions4) = RosUmdDeviceDdi::s_dxgiDeviceFuncs4;

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

    memset(m_psResourceViews, 0, sizeof(m_psResourceViews));

    memset(m_psConstantBuffer, 0, sizeof(m_psConstantBuffer));
    memset(m_ps1stConstant, 0, sizeof(m_ps1stConstant));
    memset(m_psNumberContants, 0, sizeof(m_psNumberContants));

    memset(m_vsConstantBuffer, 0, sizeof(m_vsConstantBuffer));
    memset(m_vs1stConstant, 0, sizeof(m_vs1stConstant));
    memset(m_vsNumberContants, 0, sizeof(m_vsNumberContants));

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
    
    //
    // Constant buffer is created in system memory
    //

    if (pCreateResource->BindFlags & D3D10_DDI_BIND_CONSTANT_BUFFER)
    {
        pResource->m_pSysMemCopy = new BYTE[pResource->m_hwSizeBytes];

        if (NULL == pResource->m_pSysMemCopy)
        {
            throw RosUmdException(E_OUTOFMEMORY);
        }

        if (pCreateResource->pInitialDataUP != NULL && pCreateResource->pInitialDataUP[0].pSysMem != NULL)
        {
            memcpy(pResource->m_pSysMemCopy, pCreateResource->pInitialDataUP[0].pSysMem, pResource->m_hwSizeBytes);
        }

        return;
    }

    // Call kernel mode allocation routine
    {
        RosAllocationExchange* rosAllocationExchange = pResource;

        auto allocate = D3DDDICB_ALLOCATE{};
        auto allocationInfo = D3DDDI_ALLOCATIONINFO{};
        {
            // allocationInfo.hAllocation - out: Private driver data for allocation
            allocationInfo.pSystemMem = nullptr;
            allocationInfo.pPrivateDriverData = rosAllocationExchange;
            allocationInfo.PrivateDriverDataSize = sizeof(*rosAllocationExchange);
            allocationInfo.VidPnSourceId = 0;
            allocationInfo.Flags.Primary = pCreateResource->pPrimaryDesc != nullptr;
        }

        auto rosAllocationGroupExchange = RosAllocationGroupExchange{};

        allocate.pPrivateDriverData = &rosAllocationGroupExchange;
        allocate.PrivateDriverDataSize = sizeof(rosAllocationGroupExchange);
        allocate.hResource = hRTResource.handle;
        // allocate.hKMResource - out: kernel resource handle
        allocate.NumAllocations = 1;
        allocate.pAllocationInfo = &allocationInfo;

        HRESULT hr = m_pMSKTCallbacks->pfnAllocateCb(m_hRTDevice.handle, &allocate);
        if (FAILED(hr))
            throw RosUmdException(hr);

        pResource->m_hKMResource = allocate.hKMResource;
        pResource->m_hKMAllocation = allocationInfo.hAllocation;
    }
    
     ROS_LOG_TRACE(
        "Creating resource. "
        "(m_hwWidth/HeightPixels = %u,%u  "
        "m_hwPitchBytes = %u, "
        "m_hwSizeBytes = %u, "
        "m_isPrimary = %d, "
        "m_hRTResource = 0x%p, "
        "m_hKMResource = 0x%x, "
        "m_hKMAllocation = 0x%x)",
        pResource->m_hwWidthPixels,
        pResource->m_hwHeightPixels,
        pResource->m_hwPitchBytes,
        pResource->m_hwSizeBytes,
        pResource->m_isPrimary,
        pResource->m_hRTResource.handle,
        pResource->m_hKMResource,
        pResource->m_hKMAllocation);

    if (pCreateResource->pInitialDataUP != NULL && pCreateResource->pInitialDataUP[0].pSysMem != NULL)
    {
        D3DDDICB_LOCK lock;
        memset(&lock, 0, sizeof(lock));

        lock.hAllocation = pResource->m_hKMAllocation;
        lock.Flags.WriteOnly = true;
        lock.Flags.LockEntire = true;

        Lock(&lock);

        if (pResource->m_resourceDimension == D3D10DDIRESOURCE_BUFFER)
        {
            memcpy(lock.pData, pCreateResource->pInitialDataUP[0].pSysMem, pResource->m_mip0Info.PhysicalWidth);
        }
        else if (pResource->m_resourceDimension == D3D10DDIRESOURCE_TEXTURE2D)
        {

            const BYTE * pSrc = (BYTE *)pCreateResource->pInitialDataUP[0].pSysMem;
            BYTE * pDst = (BYTE *)lock.pData;
            UINT  rowStride = pCreateResource->pInitialDataUP[0].SysMemPitch;
            pResource->ConvertInitialTextureFormatToInternal(pSrc, pDst, rowStride);
        }
        else
        {
            assert(false);
        }

        D3DDDICB_UNLOCK unlock;
        memset(&unlock, 0, sizeof(unlock));

        unlock.NumAllocations = 1;
        unlock.phAllocations = &pResource->m_hKMAllocation;

        Unlock(&unlock);
    }
}

//
// Need to instantiate a new RosUmdResource object in the memory
// pointed to by hResource.pDrvPrivate and initialize it according to
// the existing RosAllocationExchange object pointed to by the pPrivateDriverData
// member of D3DDDI_OPENALLOCATIONINFO.
//
void RosUmdDevice::OpenResource(
    const D3D10DDIARG_OPENRESOURCE* Args,
    D3D10DDI_HRESOURCE hResource,
    D3D10DDI_HRTRESOURCE hRTResource)
{
    assert(Args->PrivateDriverDataSize == sizeof(RosAllocationGroupExchange));

    if (Args->NumAllocations != 1)
        throw RosUmdException(E_INVALIDARG);

    D3DDDI_OPENALLOCATIONINFO* openAllocationInfoPtr = &Args->pOpenAllocationInfo[0];
    assert(
        openAllocationInfoPtr->PrivateDriverDataSize ==
        sizeof(RosAllocationExchange));

    auto rosAllocationExchangePtr = static_cast<const RosAllocationExchange*>(
            openAllocationInfoPtr->pPrivateDriverData);

    // Instantiate the new resource in the memory provided to us by the framework
    auto rosUmdResourcePtr = new (hResource.pDrvPrivate) RosUmdResource();

    rosUmdResourcePtr->InitSharedResourceFromExistingAllocation(
            rosAllocationExchangePtr,
            Args->hKMResource,
            openAllocationInfoPtr->hAllocation,
            hRTResource);
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

void RosUmdDevice::ConstantBufferUpdateSubresourceUP(
    RosUmdResource *pDstResource,
    UINT DstSubresource,
    _In_opt_ const D3D10_DDI_BOX *pDstBox,
    _In_ const VOID *pSysMemUP,
    UINT RowPitch,
    UINT DepthPitch,
    UINT CopyFlags)
{
    pDstResource->ConstantBufferUpdateSubresourceUP(DstSubresource, pDstBox, pSysMemUP, RowPitch, DepthPitch, CopyFlags);
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

void RosUmdDevice::ResourceMap(
    RosUmdResource * pResource,
    UINT subResource,
    D3D10_DDI_MAP mapType,
    UINT mapFlags,
    D3D10DDI_MAPPED_SUBRESOURCE* pMappedSubRes)
{
    pResource->Map(this, subResource, mapType, mapFlags, pMappedSubRes);
}

void RosUmdDevice::ResourceUnmap(
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
    UINT inFlags,
    UINT* pOutNumQualityLevels)
{
    inFormat; // unused
    inSampleCount; // unused
    inFlags; // unused

    *pOutNumQualityLevels = 0;
}

//
// Kernel mode callbacks
//

void RosUmdDevice::Lock(D3DDDICB_LOCK * pLock)
{
    HRESULT hr = m_pMSKTCallbacks->pfnLockCb(m_hRTDevice.handle, pLock);

    if (pLock->Flags.DonotWait && (hr == D3DDDIERR_WASSTILLDRAWING))
    {
        SetError(DXGI_DDI_ERR_WASSTILLDRAWING);
    }

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

HRESULT RosUmdDevice::Present(DXGI_DDI_ARG_PRESENT* Args)
{
    assert(this == CastFrom(Args->hDevice));

    if (Args->Flags.Flip)
    {
        if (Args->FlipInterval != DXGI_DDI_FLIP_INTERVAL_ONE)
        {
            assert(!"The only supported flip interval is 1.");
            return E_INVALIDARG;
        }
    }

    // Get the allocation for the source resource
    auto pSrcResource = RosUmdResource::CastFrom(Args->hSurfaceToPresent);

    // we only handle a single subresource for now
    if (Args->SrcSubResourceIndex != 0)
    {
        assert(!"Only a single subresource is supported currently");
        return E_NOTIMPL;
    }

    if (!pSrcResource->IsPrimary())
    {
        assert(!"Only primaries may be flipped");
        return E_INVALIDARG;
    }

    assert(pSrcResource->m_hKMAllocation);

    // hDstResource can be null
    D3DKMT_HANDLE hDstAllocation = {};
    if (Args->hDstResource)
    {
        auto pDstResource = RosUmdResource::CastFrom(Args->hDstResource);

        // we only handle a single subresource for now
        if (Args->DstSubResourceIndex != 0)
        {
            assert(!"Only a single subresource is support right now");
            return E_NOTIMPL;
        }
        assert(pDstResource->m_hKMAllocation);
        hDstAllocation = pDstResource->m_hKMAllocation;
    }

    DXGIDDICB_PRESENT args = {};
    args.hSrcAllocation = pSrcResource->m_hKMAllocation;
    args.hDstAllocation = hDstAllocation;
    args.pDXGIContext = Args->pDXGIContext;
    args.hContext = m_hContext;

    return m_pDXGICallbacks->pfnPresentCb(m_hRTDevice.handle, &args);
}

HRESULT RosUmdDevice::RotateResourceIdentities(
    DXGI_DDI_ARG_ROTATE_RESOURCE_IDENTITIES* Args)
{
    assert(RosUmdDevice::CastFrom(Args->hDevice) == this);

    assert(Args->Resources != 0);

    // Save the handles from the first resource
    const RosUmdResource* const firstResource = RosUmdResource::CastFrom(
            Args->pResources[0]);
    RosUmdResource* const lastResource = RosUmdResource::CastFrom(
            Args->pResources[Args->Resources - 1]);
    assert(lastResource->CanRotateFrom(firstResource));

    D3DKMT_HANDLE firstResourceKMResource = firstResource->m_hKMResource;
    D3DKMT_HANDLE firstResourceKMAllocation = firstResource->m_hKMAllocation;

    for (UINT i = 0; i < (Args->Resources - 1); ++i)
    {
        RosUmdResource* rotateTo = RosUmdResource::CastFrom(Args->pResources[i]);
        const RosUmdResource* rotateFrom = RosUmdResource::CastFrom(
                Args->pResources[i + 1]);

        assert(rotateTo->CanRotateFrom(rotateFrom));

        rotateTo->m_hKMResource = rotateFrom->m_hKMResource;
        rotateTo->m_hKMAllocation = rotateFrom->m_hKMAllocation;
    }

    // Replace the last resource's handles with those from the first resource
    lastResource->m_hKMResource = firstResourceKMResource;
    lastResource->m_hKMAllocation = firstResourceKMAllocation;

    return S_OK;
}

HRESULT RosUmdDevice::SetDisplayMode(DXGI_DDI_ARG_SETDISPLAYMODE* Args)
{
    assert(RosUmdDevice::CastFrom(Args->hDevice) == this);

    // Translate hResource and SubResourceIndex to a primary allocation handle
    auto pResource = reinterpret_cast<RosUmdResource*>(Args->hResource);

    if (Args->SubResourceIndex != 0)
    {
        assert(!"Only a single subresource is supported right now");
        return E_NOTIMPL;
    }

    // We are expecting the resource to represent a primary allocation
    assert(pResource->IsPrimary());

    // XXX Do we need to flush all rendering tasks before calling kernel side?

    D3DDDICB_SETDISPLAYMODE args = {};
    args.hPrimaryAllocation = pResource->m_hKMAllocation;

    HRESULT hr = m_pMSKTCallbacks->pfnSetDisplayModeCb(m_hRTDevice.handle, &args);
    if (FAILED(hr)) {
        switch (hr) {
        case D3DDDIERR_INCOMPATIBLEPRIVATEFORMAT:
            // TODO[jordanrh] Use args.PrivateDriverFormatAttribute to convert the
            // primary surface
            assert(args.PrivateDriverFormatAttribute != 0);
        default:
            return hr;
        }
    }

    assert(SUCCEEDED(hr));
    return S_OK;
}

HRESULT RosUmdDevice::Present1(DXGI_DDI_ARG_PRESENT1* Args)
{
    assert(this == CastFrom(Args->hDevice));

    if (Args->Flags.Flip)
    {
        if (Args->FlipInterval != DXGI_DDI_FLIP_INTERVAL_ONE)
        {
            assert(!"The only supported flip interval is 1.");
            return E_INVALIDARG;
        }
    }

    // TODO[jordanrh]: Ensure all rendering commands are flushed

    // Call pfnPresentCb for each source surface
    for (UINT i = 0; i < Args->SurfacesToPresent; ++i)
    {
        // Get the allocation for the source resource
        auto pSrcResource = RosUmdResource::CastFrom(
                Args->phSurfacesToPresent[i].hSurface);

        // we only handle a single subresource for now
        if (Args->phSurfacesToPresent[i].SubResourceIndex != 0)
        {
            assert(!"Only a single subresource is supported right now");
            return E_NOTIMPL;
        }

        if (Args->Flags.Flip && !pSrcResource->IsPrimary())
        {
            assert(!"Only primaries may be flipped");
            return E_INVALIDARG;
        }

        assert(pSrcResource->m_hKMAllocation);

        // hDstResource can be null
        D3DKMT_HANDLE hDstAllocation = {};
        if (Args->hDstResource)
        {
            auto pDstResource = RosUmdResource::CastFrom(
                    Args->hDstResource);

            // we only handle a single subresource for now
            if (Args->DstSubResourceIndex != 0)
            {
                assert(!"Only a single subresource is supported right now");
                return E_NOTIMPL;
            }
            assert(pDstResource->m_hKMAllocation);
            hDstAllocation = pDstResource->m_hKMAllocation;
        }

        DXGIDDICB_PRESENT args = {};
        args.hSrcAllocation = pSrcResource->m_hKMAllocation;
        args.hDstAllocation = hDstAllocation;
        args.pDXGIContext = Args->pDXGIContext;
        args.hContext = m_hContext;

        HRESULT hr = m_pDXGICallbacks->pfnPresentCb(m_hRTDevice.handle, &args);
        if (FAILED(hr))
            return hr;
    }

    return S_OK;
}

_Use_decl_annotations_
void RosUmdDevice::CheckDirectFlipSupport(
    D3D10DDI_HDEVICE hDevice,
    D3D10DDI_HRESOURCE hResource1,
    D3D10DDI_HRESOURCE hResource2,
    UINT CheckDirectFlipFlags,
    BOOL *pSupported
    )
{
    assert(CastFrom(hDevice) == this);
    assert((CheckDirectFlipFlags & ~D3D11_1DDI_CHECK_DIRECT_FLIP_IMMEDIATE) == 0);

    //
    // We can only flip to the resource if it has the same format as the
    // current primary since we do not support mode change in
    // SetVidPnSourceAddress() at this point.
    //
    const RosUmdResource* currentResourcePtr = RosUmdResource::CastFrom(hResource1);
    const RosUmdResource* directFlipResourcePtr = RosUmdResource::CastFrom(hResource2);

    // Resources must have same dimensions and format
    *pSupported =
        (directFlipResourcePtr->m_mip0Info == currentResourcePtr->m_mip0Info) &&
        (directFlipResourcePtr->m_format == currentResourcePtr->m_format) &&
        (directFlipResourcePtr->m_hwPitchBytes == currentResourcePtr->m_hwPitchBytes) &&
        (directFlipResourcePtr->m_hwSizeBytes == currentResourcePtr->m_hwSizeBytes);
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

void RosUmdDevice::SetException(const RosUmdException & e)
{
    SetError(e.m_hr);
}

void RosUmdDevice::SetException(const std::exception & e)
{
    auto rosUmdException = dynamic_cast<const RosUmdException*>(&e);
    if (rosUmdException)
    {
        SetError(rosUmdException->m_hr);
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
    //
    // Refresh render state
    //

    RefreshPipelineState(0);

    BYTE *  pCommandBuffer;

    m_commandBuffer.ReserveCommandBufferSpace(
        false,                                  // HW command
        sizeof(VC4VertexArrayPrimitives),
        &pCommandBuffer);

#if VC4

    VC4VertexArrayPrimitives *   pVC4VertexArrayPrimitives = (VC4VertexArrayPrimitives *)pCommandBuffer;

    *pVC4VertexArrayPrimitives = vc4VertexArrayPrimitives;

    pVC4VertexArrayPrimitives->PrimitiveMode = (BYTE)ConvertD3D11Topology(m_topology);

    pVC4VertexArrayPrimitives->Length = vertexCount;

    pVC4VertexArrayPrimitives->IndexOfFirstVertex = startVertexLocation;

#endif

    m_commandBuffer.CommitCommandBufferSpace(sizeof(VC4VertexArrayPrimitives), 1);

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
#if VC4

    RosUmdResource * pRenderTarget = RosUmdResource::CastFrom(pRenderTargetView->m_create.hDrvResource);

    //
    // KMD issumes Clear Colors command before Draw call
    //

    if (m_flags.m_hasDrawCall)
    {
        m_commandBuffer.Flush(0);
    }

    // Set clear color into command buffer header for KMD to generate Rendering Control List
    m_commandBuffer.UpdateClearColor(ConvertFloatColor(pRenderTarget->m_format, clearColor));

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

void RosUmdDevice::VsSetConstantBuffers11_1(
    UINT startBuffer,
    UINT numberBuffers,
    const D3D10DDI_HRESOURCE *  phResources,
    const UINT *    pFirstConstant,
    const UINT *    pNumberConstants)
{
    UINT bufIndex;

    bufIndex = startBuffer;
    for (UINT i = 0; i < numberBuffers; i++, bufIndex++)
    {
        m_vsConstantBuffer[bufIndex] = RosUmdResource::CastFrom(phResources[i]);
    }

    if (pFirstConstant && pNumberConstants)
    {
        bufIndex = startBuffer;
        for (UINT i = 0; i < numberBuffers; i++, bufIndex++)
        {
            m_vs1stConstant[bufIndex] = pFirstConstant[i];
            m_vsNumberContants[bufIndex] = pNumberConstants[i];
        }
    }
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

void RosUmdDevice::PSSetShaderResources(UINT offset, UINT numViews, const D3D10DDI_HSHADERRESOURCEVIEW * phShaderResourceViews)
{
    for (UINT i = 0; i < numViews; i++)
    {
        m_psResourceViews[offset + i] = RosUmdShaderResourceView::CastFrom(phShaderResourceViews[i]);
    }
}

void RosUmdDevice::PsSetConstantBuffers11_1(
    UINT startBuffer,
    UINT numberBuffers,
    const D3D10DDI_HRESOURCE *  phResources,
    const UINT *    pFirstConstant,
    const UINT *    pNumberConstants)
{
    UINT bufIndex;

    bufIndex = startBuffer;
    for (UINT i = 0; i < numberBuffers; i++, bufIndex++)
    {
        m_psConstantBuffer[bufIndex] = RosUmdResource::CastFrom(phResources[i]);
    }

    if (pFirstConstant && pNumberConstants)
    {
        bufIndex = startBuffer;
        for (UINT i = 0; i < numberBuffers; i++, bufIndex++)
        {
            m_ps1stConstant[bufIndex] = pFirstConstant[i];
            m_psNumberContants[bufIndex] = pNumberConstants[i];
        }
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

void RosUmdDevice::SetScissorRects(UINT NumScissorRects, UINT ClearScissorRects, const D3D10_DDI_RECT *pRects)
{
#if VC4
    // VC4 can handle only 1 scissor rect, so take 1st one only.
    assert(NumScissorRects <= 1);
    if (NumScissorRects)
    {
        assert(pRects);
        m_scissorRectSet = true;
        m_scissorRect = *pRects;
    }
    else if (ClearScissorRects)
    {
        // When NumScissorRects is zero and ClearScissorRects is not zero, then clear current scissor.
        m_scissorRectSet = false;
        ZeroMemory(&m_scissorRect, sizeof(m_scissorRect));
    }
#endif // VC4
}

void RosUmdDevice::RefreshPipelineState(UINT vertexOffset)
{
    RosUmdResource * pRenderTarget = RosUmdResource::CastFrom(m_renderTargetViews[0]->m_create.hDrvResource);

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
    BYTE *  pCurCommand;
    UINT    curCommandOffset;
    D3DDDI_PATCHLOCATIONLIST *  pPatchLocation;
    D3DDDI_PATCHLOCATIONLIST *  pCurPatchLocation;
    UINT    dummyAllocIndex;
    UINT    allocListIndex;

    //
    // TODO[indyz] : Decide maximal value to cover all cases
    //

    UINT    maxStateComamnds = 170;
    UINT    maxAllocationsUsed = 15;
    UINT    maxPatchLocations = 22;

    //
    // To simplify patching and merging of internal and user constant data,
    // the pixel shader constant buffer is copied directly into command buffer.
    //

    UINT    psContantDataSize;
    UINT    vsContantDataSize;
    UINT    csContantDataSize;

    VC4_UNIFORM_FORMAT *    pPSUniformEntries;
    VC4_UNIFORM_FORMAT *    pVSUniformEntries;
    VC4_UNIFORM_FORMAT *    pCSUniformEntries;
    UINT    numPSUniformEntries;
    UINT    numVSUniformEntries;
    UINT    numCSUniformEntries;

    //
    // Shader compiler specifies how multiple constant buffers are copied/merged into command buffer
    //

    pPSUniformEntries = m_pixelShader->GetShaderUniformFormat(ROS_PIXEL_SHADER_UNIFORM_STORAGE, &numPSUniformEntries);
    pVSUniformEntries = m_vertexShader->GetShaderUniformFormat(ROS_VERTEX_SHADER_UNIFORM_STORAGE, &numVSUniformEntries);
    pCSUniformEntries = m_vertexShader->GetShaderUniformFormat(ROS_COORDINATE_SHADER_UNIFORM_STORAGE, &numCSUniformEntries);

    psContantDataSize = numPSUniformEntries*sizeof(FLOAT);
    vsContantDataSize = numVSUniformEntries*sizeof(FLOAT);
    csContantDataSize = numCSUniformEntries*sizeof(FLOAT);

    maxStateComamnds += (psContantDataSize + vsContantDataSize + csContantDataSize);

    m_commandBuffer.ReserveCommandBufferSpace(
        false,                                  // HW command
        maxStateComamnds,
        &pCommandBuffer,
        maxAllocationsUsed,
        maxPatchLocations,
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

    if (m_scissorRectSet && m_rasterizerState->GetDesc()->ScissorEnable)
    {
        RECT Intersect;
        RECT Viewport = {
            (LONG)round(m_viewports[0].TopLeftX),
            (LONG)round(m_viewports[0].TopLeftY),
            (LONG)round((m_viewports[0].TopLeftX + m_viewports[0].Width)),
            (LONG)round((m_viewports[0].TopLeftY + m_viewports[0].Height)) };

        if (_IntersectRect(&Intersect, &Viewport, &m_scissorRect))
        {
            pVC4ClipWindow->ClipWindowLeft = (USHORT)Intersect.left;
            pVC4ClipWindow->ClipWindowBottom = (USHORT)Intersect.top;
            pVC4ClipWindow->ClipWindowWidth = (USHORT)(Intersect.right - Intersect.left);
            pVC4ClipWindow->ClipWindowHeight = (USHORT)(Intersect.bottom - Intersect.top);
        }
        else
        {
            assert(false); // NOTHING to draw.
        }
    }
    else
    {
        pVC4ClipWindow->ClipWindowLeft = (USHORT)round(m_viewports[0].TopLeftX);
        pVC4ClipWindow->ClipWindowBottom = (USHORT)round(m_viewports[0].TopLeftY);
        pVC4ClipWindow->ClipWindowWidth = (USHORT)round(m_viewports[0].Width);
        pVC4ClipWindow->ClipWindowHeight = (USHORT)round(m_viewports[0].Height);
    }

    //
    // Write Configuration Bits command to update render state
    //
    // TODO[indyz]: Set up more VC4ConfigBits from rasterizer state, etc
    //

    VC4ConfigBits *  pVC4ConfigBits;
    MoveToNextCommand(pVC4ClipWindow, pVC4ConfigBits, curCommandOffset);

    *pVC4ConfigBits = vc4ConfigBits;
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

    //
    // It looks like that VC4ConfigBits::ClockwisePrimitives
    // matches the D3D11_1_DDI_RASTERIZER_DESC::FrontCounterClockwise.
    // It must be set in the same way for proper behavior.
    //

    pVC4ConfigBits->ClockwisePrimitives = m_rasterizerState->m_desc.FrontCounterClockwise;

    //
    // The D3D11 default depth stencil state is DepthEnable of true with
    // comparison function of less, and VC4's Tile Buffer has Z of 0.0 by
    // default, without checking depth stencil view this combination would
    // cull all pixels.
    //

    if (m_depthStencilState->m_desc.DepthEnable && m_depthStencilView)
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
    else
    {
        pVC4ConfigBits->DepthTestFunction = VC4_DEPTH_TEST_ALWAYS;

        pVC4ConfigBits->EarlyZUpdatesEnable = 1;
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
    assert(patchLocationUsed <= maxPatchLocations);

    m_commandBuffer.CommitCommandBufferSpace(
        commandsWritten,
        patchLocationUsed);

#else

    //
    // Write Depth Offset, Point Size, Line Width command
    //

    VC4DepthOffset *    pVC4DepthOffset;
    MoveToNextCommand(pVC4ConfigBits, pVC4DepthOffset, curCommandOffset);

    *pVC4DepthOffset = vc4DepthOffset;

    VC4PointSize *  pVC4PointSize;
    MoveToNextCommand(pVC4DepthOffset, pVC4PointSize, curCommandOffset);

    *pVC4PointSize = vc4PointSize;

    VC4LineWidth *  pVC4LineWidth;
    MoveToNextCommand(pVC4PointSize, pVC4LineWidth, curCommandOffset);

    *pVC4LineWidth = vc4LineWidth;

    //
    // Write Clipper XY Scaling command
    //

    VC4ClipperXYScaling *   pVC4ClipperXYScaling;
    MoveToNextCommand(pVC4LineWidth, pVC4ClipperXYScaling, curCommandOffset);

    *pVC4ClipperXYScaling = vc4ClipperXYScaling;

    pVC4ClipperXYScaling->ViewportHalfWidth = m_viewports[0].Width / 2.0f * 16.0f;
    pVC4ClipperXYScaling->ViewportHalfHeight = -m_viewports[0].Height / 2.0f * 16.0f;

    //
    // Write Clipper Z Scale and Offset command
    //

    VC4ClipperZScaleAndOffset * pVC4ClipperZScaleAndOffset;
    MoveToNextCommand(pVC4ClipperXYScaling, pVC4ClipperZScaleAndOffset, curCommandOffset);

    *pVC4ClipperZScaleAndOffset = vc4ClipperZScaleAndOffset;

    // Scale and offset the depth range from MinDepth to MaxDepth to 0.0 to 1.0
    //

    pVC4ClipperZScaleAndOffset->ViewportZOffset = -m_viewports[0].MinDepth;

    if (m_viewports[0].MaxDepth != m_viewports[0].MinDepth)
    {
        pVC4ClipperZScaleAndOffset->ViewportZScale = 1.0f/(m_viewports[0].MaxDepth - m_viewports[0].MinDepth);
    }
    else
    {
        pVC4ClipperZScaleAndOffset->ViewportZScale = 0.0;
    }

    //
    // Write Viewport Offset command
    //

    VC4ViewportOffset * pVC4ViewportOffset;
    MoveToNextCommand(pVC4ClipperZScaleAndOffset, pVC4ViewportOffset, curCommandOffset);

    *pVC4ViewportOffset = vc4ViewportOffset;

    pVC4ViewportOffset->ViewportCenterX = (SHORT)(m_viewports[0].Width / 2.0f * 16.0f);
    pVC4ViewportOffset->ViewportCenterY = (SHORT)(m_viewports[0].Height / 2.0f * 16.0f);

    //
    // Write Flat Shade Flags command
    //

    VC4FlatShadeFlags * pVC4FlatShadeFlags;
    MoveToNextCommand(pVC4ViewportOffset, pVC4FlatShadeFlags, curCommandOffset);

    *pVC4FlatShadeFlags = vc4FlatShadeFlags;

#ifdef SSR_END_DMA

    UINT vc4GLShaderStateRecordOffset = PAGE_SIZE - sizeof(VC4GLShaderStateRecord);

    VC4GLShaderStateRecord  *pVC4GLShaderStateRecord = (VC4GLShaderStateRecord *)((((PBYTE)pVC4ViewportOffset) - curCommandOffset) + vc4GLShaderStateRecordOffset);

#else

    //
    // Write Branch command to skip over Fragment Shader Uniforms and Shader State Record
    //

    VC4Branch * pVC4Branch;
    MoveToNextCommand(pVC4FlatShadeFlags, pVC4Branch, curCommandOffset);

    UINT vc4GLShaderStateRecordOffset = curCommandOffset + sizeof(VC4Branch);
    AlignValue(vc4GLShaderStateRecordOffset, 16);

    *pVC4Branch = vc4Branch;

    m_commandBuffer.SetPatchLocation(
        pCurPatchLocation,
        dummyAllocIndex,
        curCommandOffset + offsetof(VC4Branch, BranchAddress),
        VC4_SLOT_BRANCH,
        vc4GLShaderStateRecordOffset +
            sizeof(VC4GLShaderStateRecord) +
            m_elementLayout->m_numElements*sizeof(VC4VertexAttribute) +
            psContantDataSize +
            vsContantDataSize +
            csContantDataSize);

    //
    // Write Shader State Record for the subsequent NV Shader State command
    //

    VC4GLShaderStateRecord  *pVC4GLShaderStateRecord = (VC4GLShaderStateRecord *)(((PBYTE)pVC4Branch) + (vc4GLShaderStateRecordOffset - curCommandOffset));

#endif

    *pVC4GLShaderStateRecord = vc4GLShaderStateRecord;


    pVC4GLShaderStateRecord->EnableClipping = 1;

    pVC4GLShaderStateRecord->FragmentShaderIsSingleThreaded = 1;

    UINT numVaryings = m_pixelShader->GetShaderInputCount();
    assert(numVaryings < 0x100);
    pVC4GLShaderStateRecord->FragmentShaderNumberOfVaryings = (BYTE)numVaryings;

#if DBG
    pVC4GLShaderStateRecord->FragmentShaderCodeAddress      = 0xDEADBEEF;
    pVC4GLShaderStateRecord->FragmentShaderUniformsAddress  = 0xDEADBEEF;
#endif

    allocListIndex = m_commandBuffer.UseResource(m_pixelShader->GetCodeResource(), false);

    m_commandBuffer.SetPatchLocation(
        pCurPatchLocation,
        allocListIndex,
        vc4GLShaderStateRecordOffset + offsetof(VC4GLShaderStateRecord, FragmentShaderCodeAddress));

    //
    // Set Fragment Shader Uniforms Address
    //

    UINT    psUniformOffset = vc4GLShaderStateRecordOffset +
                              sizeof(VC4GLShaderStateRecord) +
                              m_elementLayout->m_numElements*sizeof(VC4VertexAttribute);

    if (psContantDataSize)
    {
        m_commandBuffer.SetPatchLocation(
            pCurPatchLocation,
            dummyAllocIndex,
            vc4GLShaderStateRecordOffset + offsetof(VC4GLShaderStateRecord, FragmentShaderUniformsAddress),
            VC4_SLOT_FS_UNIFORM_ADDRESS,
            psUniformOffset);
    }
    else
    {
        //
        // Set the uniforms address to dummy allocation when there is not constant buffer
        // VC4 probably has read-ahead capability, it seems to hang without an valid address
        //

        m_commandBuffer.SetPatchLocation(
            pCurPatchLocation,
            dummyAllocIndex,
            vc4GLShaderStateRecordOffset + offsetof(VC4GLShaderStateRecord, FragmentShaderUniformsAddress));
    }

#if DBG
    pVC4GLShaderStateRecord->VertexShaderCodeAddress        = 0xDEADBEEF;
    pVC4GLShaderStateRecord->VertexShaderUniformsAddress    = 0xDEADBEEF;
#endif

    allocListIndex = m_commandBuffer.UseResource(m_vertexShader->GetCodeResource(), false);

    m_commandBuffer.SetPatchLocation(
        pCurPatchLocation,
        allocListIndex,
        vc4GLShaderStateRecordOffset + offsetof(VC4GLShaderStateRecord, VertexShaderCodeAddress));

    //
    // Set Vertex Shader Uniform Address
    //

    if (vsContantDataSize)
    {
        m_commandBuffer.SetPatchLocation(
            pCurPatchLocation,
            dummyAllocIndex,
            vc4GLShaderStateRecordOffset + offsetof(VC4GLShaderStateRecord, VertexShaderUniformsAddress),
            VC4_SLOT_VS_UNIFORM_ADDRESS,
            psUniformOffset + psContantDataSize);
    }
    else
    {
        m_commandBuffer.SetPatchLocation(
            pCurPatchLocation,
            dummyAllocIndex,
            vc4GLShaderStateRecordOffset + offsetof(VC4GLShaderStateRecord, VertexShaderUniformsAddress));
    }

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

    //
    // Set Vertex Shader Uniform Address
    //

    if (csContantDataSize)
    {
        m_commandBuffer.SetPatchLocation(
            pCurPatchLocation,
            dummyAllocIndex,
            vc4GLShaderStateRecordOffset + offsetof(VC4GLShaderStateRecord, CoordinateShaderUniformsAddress),
            VC4_SLOT_CS_UNIFORM_ADDRESS,
            psUniformOffset + psContantDataSize + vsContantDataSize);
    }
    else
    {
        m_commandBuffer.SetPatchLocation(
            pCurPatchLocation,
            dummyAllocIndex,
            vc4GLShaderStateRecordOffset + offsetof(VC4GLShaderStateRecord, CoordinateShaderUniformsAddress));
    }

    curCommandOffset = vc4GLShaderStateRecordOffset;

    VC4VertexAttribute *    pVC4VertexAttribute;
    MoveToNextCommand(pVC4GLShaderStateRecord, pVC4VertexAttribute, curCommandOffset);

    //
    // TODO[indyz]: Avoid reading vertex data that Coordinate shader doesn't need
    //

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
        pVC4VertexAttribute->MemoryStride = (BYTE)m_vertexStrides[pElementDesc[i].InputSlot];
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
    // Copy internal Fragment Shader Uniforms (Texture Config Paramater0/1/2/3)
    // and Uniforms from PS constant buffers into the command buffer
    //

    pCurCommand = (BYTE *)pVC4VertexAttribute;

    if (psContantDataSize)
    {
        WriteUniforms(
            true,
            pPSUniformEntries,
            numPSUniformEntries,
            pCurCommand,
            curCommandOffset,
            pCurPatchLocation);
    }

    if (vsContantDataSize)
    {
        WriteUniforms(
            false,
            pVSUniformEntries,
            numVSUniformEntries,
            pCurCommand,
            curCommandOffset,
            pCurPatchLocation);
    }

    if (csContantDataSize)
    {
        WriteUniforms(
            false,
            pCSUniformEntries,
            numCSUniformEntries,
            pCurCommand,
            curCommandOffset,
            pCurPatchLocation);
    }

    //
    // Write GL Shader State command
    //
    VC4GLShaderState *  pVC4GLShaderState;

#ifdef SSR_END_DMA

    MoveToNextCommand(pVC4ViewportOffset, pVC4GLShaderState, curCommandOffset);

#else

    pVC4GLShaderState = (VC4GLShaderState *)(pCurCommand);

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
    assert(patchLocationUsed <= maxPatchLocations);

    m_commandBuffer.CommitCommandBufferSpace(
        commandsWritten,
        patchLocationUsed);

#endif

#endif

}

#if VC4

VC4TextureType RosUmdDevice::MapDXGITextureFormatToVC4Type(RosHwLayout layout, DXGI_FORMAT format)
{   
    VC4TextureType textureType;
    textureType.TextureType = VC4_TEX_RGBA32R;

    // Map texture layout and DXGI format to HW format
    // Note: Some of the DXGI formats are emulated (for example, during 
    // initialization, texture is converted to HW-compatible format)
    if (layout == RosHwLayout::Tiled)
    {
        switch (format)
        {
            case DXGI_FORMAT_R8G8B8A8_UNORM:
            {
                textureType.TextureType = VC4_TEX_RGBX8888;
            } 
            break;
            case DXGI_FORMAT_R8_UNORM:
            {
                textureType.TextureType = VC4_TEX_RGBX8888;
            }
            break;
            case DXGI_FORMAT_R8G8_UNORM:
            {
                textureType.TextureType = VC4_TEX_RGBX8888;
            }
            break;
            case DXGI_FORMAT_A8_UNORM:
            {
                textureType.TextureType = VC4_TEX_ALPHA;
            }
            break;

            default:
            {
                assert(false);
            }
            break;
        }
    }
    else
    {
        // todo: create table with modes
        // Linear (raster) format
        textureType.TextureType = VC4_TEX_RGBA32R;
    }

    return textureType;
}

void RosUmdDevice::WriteUniforms(
    BOOLEAN                     bPSUniform,
    VC4_UNIFORM_FORMAT *        pUniformEntries,
    UINT                        numUniformEntries,
    BYTE *                     &pCurCommand,
    UINT                       &curCommandOffset,
    D3DDDI_PATCHLOCATIONLIST * &pCurPatchLocation)
{
    UINT                    allocListIndex;
    RosUmdResource *        pTexture;
    VC4_UNIFORM_FORMAT *    pCurUniformEntry = pUniformEntries;

    for (UINT i = 0; i < numUniformEntries; i++, pCurUniformEntry++)
    {
        switch (pCurUniformEntry->Type)
        {
        case VC4_UNIFORM_TYPE_USER_CONSTANT:
            {
                FLOAT * pUniform = (FLOAT *)pCurCommand;

                RosUmdResource *    pConstantBuffer;

                if (bPSUniform)
                {
                    pConstantBuffer = m_psConstantBuffer[pCurUniformEntry->userConstant.bufferSlot];
                }
                else
                {
                    pConstantBuffer = m_vsConstantBuffer[pCurUniformEntry->userConstant.bufferSlot];
                }

                *pUniform = *(((FLOAT *)pConstantBuffer->m_pSysMemCopy) + pCurUniformEntry->userConstant.bufferOffset);

                MoveToNextCommand(pUniform, pCurCommand, curCommandOffset);
            }
            break;
        case VC4_UNIFORM_TYPE_SAMPLER_CONFIG_P0:
            {
                VC4TextureConfigParameter0 *    pVC4TexConfigParam0 = (VC4TextureConfigParameter0 *)pCurCommand;

                pTexture = RosUmdResource::CastFrom(m_psResourceViews[pCurUniformEntry->samplerConfiguration.resourceIndex]->m_create.hDrvResource);

                pVC4TexConfigParam0->UInt0 = 0;

                //
                // TODO[indyz]: Support all VC4 texture formats and tiling
                //

                VC4TextureType  vc4TextureType = MapDXGITextureFormatToVC4Type(pTexture->m_hwLayout, pTexture->m_format);

                pVC4TexConfigParam0->TYPE = vc4TextureType.TYPE;

                allocListIndex = m_commandBuffer.UseResource(pTexture, false);

                m_commandBuffer.SetPatchLocation(
                    pCurPatchLocation,
                    allocListIndex,
                    curCommandOffset,
                    0,
                    pVC4TexConfigParam0->UInt0);

#if DBG

                pVC4TexConfigParam0->BASE = 0xDEADB;

#endif

                MoveToNextCommand(pVC4TexConfigParam0, pCurCommand, curCommandOffset);
            }
            break;
        case VC4_UNIFORM_TYPE_SAMPLER_CONFIG_P1:
            {
                VC4TextureConfigParameter1 *    pVC4TexConfigParam1 = (VC4TextureConfigParameter1 *)pCurCommand;

                pTexture = RosUmdResource::CastFrom(m_psResourceViews[pCurUniformEntry->samplerConfiguration.resourceIndex]->m_create.hDrvResource);

                pVC4TexConfigParam1->UInt0 = 0;

                VC4TextureType  vc4TextureType = MapDXGITextureFormatToVC4Type(pTexture->m_hwLayout, pTexture->m_format);
         
                RosUmdSampler * pSampler = m_pixelSamplers[pCurUniformEntry->samplerConfiguration.samplerIndex];
                D3D10_DDI_SAMPLER_DESC * pSamplerDesc = &pSampler->m_desc;

                //
                // TODO[indyz] : Support wrap mode of border (using border color)
                //
                assert(pSampler->m_desc.AddressU != D3D10_DDI_TEXTURE_ADDRESS_BORDER);
                assert(pSampler->m_desc.AddressV != D3D10_DDI_TEXTURE_ADDRESS_BORDER);

                // VC4 doesn't support Mirror Once wrap mode
                assert(pSampler->m_desc.AddressU != D3D10_DDI_TEXTURE_ADDRESS_MIRRORONCE);
                assert(pSampler->m_desc.AddressV != D3D10_DDI_TEXTURE_ADDRESS_MIRRORONCE);

                pVC4TexConfigParam1->WRAP_S = ConvertD3D11TextureAddressMode(pSamplerDesc->AddressU);
                pVC4TexConfigParam1->WRAP_T = ConvertD3D11TextureAddressMode(pSamplerDesc->AddressV);

                pVC4TexConfigParam1->MINFILT = ConvertD3D11TextureMinFilter(pSamplerDesc->Filter, pTexture->m_mipLevels <= 1);
                pVC4TexConfigParam1->MAGFILT = ConvertD3D11TextureMagFilter(pSamplerDesc->Filter);

                pVC4TexConfigParam1->WIDTH = pTexture->m_hwWidthPixels;
                pVC4TexConfigParam1->HEIGHT = pTexture->m_hwHeightPixels;

                pVC4TexConfigParam1->TYPE4 = vc4TextureType.TYPE4;

                MoveToNextCommand(pVC4TexConfigParam1, pCurCommand, curCommandOffset);
            }
            break;
        case VC4_UNIFORM_TYPE_SAMPLER_CONFIG_P2:
            {
                DWORD * pVC4TexConfigParam23 = (DWORD *)pCurCommand;

                pTexture = RosUmdResource::CastFrom(m_psResourceViews[pCurUniformEntry->samplerConfiguration.resourceIndex]->m_create.hDrvResource);

                assert(pTexture->m_resourceDimension == D3D10DDIRESOURCE_TEXTURECUBE);

                *pVC4TexConfigParam23 = 0;
                MoveToNextCommand(pVC4TexConfigParam23, pVC4TexConfigParam23, curCommandOffset);

                *pVC4TexConfigParam23 = 0;
                MoveToNextCommand(pVC4TexConfigParam23, pCurCommand, curCommandOffset);
            }
            break;
        case VC4_UNIFORM_TYPE_VIEWPORT_SCALE_X:
            {
                RosUmdResource * pRenderTarget = RosUmdResource::CastFrom(m_renderTargetViews[0]->m_create.hDrvResource);

                FLOAT * pScaleX = (FLOAT *)pCurCommand;

                *pScaleX = pRenderTarget->m_hwWidthPixels*16.0f/2.0f;
                MoveToNextCommand(pScaleX, pCurCommand, curCommandOffset);
            }
            break;
        case VC4_UNIFORM_TYPE_VIEWPORT_SCALE_Y:
            {
                RosUmdResource * pRenderTarget = RosUmdResource::CastFrom(m_renderTargetViews[0]->m_create.hDrvResource);

                FLOAT * pScaleY = (FLOAT *)pCurCommand;

                *pScaleY = pRenderTarget->m_hwHeightPixels*-16.0f/2.0f;
                MoveToNextCommand(pScaleY, pCurCommand, curCommandOffset);
            }
            break;
        case VC4_UNIFORM_TYPE_BLEND_FACTOR_R:
        case VC4_UNIFORM_TYPE_BLEND_FACTOR_G:
        case VC4_UNIFORM_TYPE_BLEND_FACTOR_B:
        case VC4_UNIFORM_TYPE_BLEND_FACTOR_A:
            {
                FLOAT * pBlendFactor = (FLOAT *)pCurCommand;

                *pBlendFactor = m_blendFactor[pCurUniformEntry->Type - VC4_UNIFORM_TYPE_BLEND_FACTOR_R];
                MoveToNextCommand(pBlendFactor, pCurCommand, curCommandOffset);
            }
            break;
        case VC4_UNIFORM_TYPE_BLEND_SAMPLE_MASK:
            {
                uint32_t *pSampleMask = (uint32_t *)pCurCommand;

                // TODO: this must be color channel aware depending on RT format.
                *pSampleMask = 0xFFFFFFFF;
                MoveToNextCommand(pSampleMask, pCurCommand, curCommandOffset);
            }
            break;
        default:
            assert(false);  // Invalid values for pixel/fragment shader
            break;
        }
    }
}

#endif

void RosUmdDevice::WriteEpilog()
{
    PBYTE   pCommandBuffer;

    m_commandBuffer.ReserveCommandBufferSpace(
        false,
        4,
        &pCommandBuffer);

    //
    // Write Flush All State, NOP and Halt commands
    //
    pCommandBuffer[0] = VC4_CMD_INCREMENT_SEMAPHORE;
    pCommandBuffer[1] = VC4_CMD_FLUSH_ALL_STATE;
    pCommandBuffer[2] = VC4_CMD_NOP;
    pCommandBuffer[3] = VC4_CMD_HALT;

    m_commandBuffer.CommitCommandBufferSpace(4);

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

void RosUmdDevice::SetPredication(D3D10DDI_HQUERY hQuery, BOOL bPredicateValue)
{
    //
    // https://msdn.microsoft.com/en-us/library/windows/hardware/ff569547(v=vs.85).aspx
    // per doc, hQuery can contain nullptr - supposed to save the predicate value for future use
    //

    if (nullptr == hQuery.pDrvPrivate)
    {
        m_bPredicateValue = bPredicateValue;
    }
    else
    {
        //
        // TODO: Implement remaining query values other than null
        //

        assert(false);
    }

    //
    // per MDSN Predication should set an error in the case one was seen
    // D3D will interpret an error as critical
    //
    /*
    HRESULT hr = S_OK;

    if (FAILED(hr))
    {
    SetError(hr);
    }
    */
}

void RosUmdDevice::ResourceCopyRegion11_1(
    RosUmdResource *pDestinationResource,
    UINT DstSubresource,
    UINT DstX,
    UINT DstY,
    UINT DstZ,
    RosUmdResource * pSourceResource,
    UINT SrcSubresource,
    const D3D10_DDI_BOX* pSrcBox,
    UINT copyFlags
    )
{
    bool bCopy = true;
    bool bCopyEntireSubresource = false;

    //
    // TODO: come back to philosophy if these need to be checked in FREE builds
    //

    assert(nullptr != pDestinationResource);
    assert(nullptr != pSourceResource);

    //
    // https://msdn.microsoft.com/en-us/library/windows/hardware/hh439845(v=vs.85).aspx
    //

    if (nullptr == pSrcBox)
    {
        //
        // when pSrcBox is null, copy the entire subresource
        //

        bCopyEntireSubresource = true;
    }
    else if (pSrcBox->left >= pSrcBox->right ||
             pSrcBox->top >= pSrcBox->bottom ||
             pSrcBox->front >= pSrcBox->back)
    {
        //
        // pSrcBox is considered empty, don't do anything, and exit
        //

        return;
    }
    else
    {
        //
        // TODO: the pSrcBox must not extend over the edges of the source subregion or the destination subregion
        //
    }

    //
    // both source and destination resources must be the same type of resource
    //

    if (pDestinationResource->m_primaryDesc.ModeDesc.Format != pSourceResource->m_primaryDesc.ModeDesc.Format)
    {
        return;
    }

    //
    // both source and destination must have format types that are convertible to each other
    //

    if (pDestinationResource->m_format == pSourceResource->m_format)
    {
        bCopy = true;
    }
    else
    {
        bCopy = false;

        //
        // TOOD: Implement conversion check logic, or DDI for ResourceConvertRegion
        //       ensure each source and destionation resource format in D3D11DDIARG_CREATERESOURCE.Format supports appropriate conversion
        //

        assert(false);
    }

    //
    // the source and destination resource must not be currently mapped
    //

    if (pDestinationResource->IsMapped() || pSourceResource->IsMapped())
    {
        return;
    }

    //
    // TODO: - Honor the copy flags
    //

    (copyFlags); //not used yet

    //
    // NOTE: for buffers, all coordinates are in bytes. for textures, all coordinates are in pixels
    //

    //
    // ensure the destination resource not created with D3D10_DDI_USAGE_IMMUTABLE
    //

    if (pDestinationResource->m_usage & D3D10_DDI_USAGE_IMMUTABLE)
    {
        return;
    }

    //
    // ensure either source or destination has D3D10_DDI_BIND_DEPTH_STENCIL in D3D11DDIARG_CREATERESOURCE.BindFlags OR is a multi - sampled resource :
    //          then verify pSrcBox is NULL and DstX, DstY and DstZ are 0

    if ((((pDestinationResource->m_bindFlags & D3D10_DDI_BIND_DEPTH_STENCIL) || pDestinationResource->IsMultisampled()) ||
        ((pSourceResource->m_bindFlags & D3D10_DDI_BIND_DEPTH_STENCIL) || pSourceResource->IsMultisampled())) &&
        (nullptr != pSrcBox || 0 != DstX || 0 != DstY || 0 != DstZ))
    {
        return;
    }

    //
    // ensure sub resource indicies are in range
    //

    if (DstSubresource >= pDestinationResource->m_arraySize ||
        SrcSubresource >= pSourceResource->m_arraySize)
    {
        return;
    }

    //
    // TODO: ensure the source and destination resources are NOT part of the exact same subresource
    //

    //
    // TODO: ensure alignment restrictions apply to coordinates
    //

    //
    // TODO: ensure each source and desitnation resource format in D3D11DDIARG_CREATERESOURCE.Format is in the same typeless group
    //

    //
    // ensure the source and destination resources have the same number of samples and quality level (except for single sampled resources, which only number of samples are the same)
    //

    if (pDestinationResource->m_sampleDesc.Count != pSourceResource->m_sampleDesc.Count ||
        pDestinationResource->m_sampleDesc.Quality != pSourceResource->m_sampleDesc.Quality)
    {
        return;
    }

    //
    // finally perform the copy or convert
    //

    if (bCopy)
    {
        //
        // TODO: implement the copy
        //

        assert(false);
    }
    else
    {
        //
        // TODO: convert path
        //

        assert(false);
    }

    //
    // per MDSN ResourceCopyRegion should set an error in the case one was seen
    // D3D will interpret an error as critical
    //

/*
    HRESULT hr = S_OK;

    if (FAILED(hr))
    {
        SetError(hr);
    }
*/
}
