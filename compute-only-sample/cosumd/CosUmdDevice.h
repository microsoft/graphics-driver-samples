////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Device
//
// Copyright (C) Microsoft Corporation
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "CosUmdCommandBuffer.h"
#include "CosAllocation.h"
#include "CosUmdUtil.h"
#include "CosUmdDebug.h"

#include "CosUmdResource.h"

#include "CosUmdShader.h"

#include "CosUmdBlendState.h"
#include "CosUmdRasterizerState.h"
#include "CosUmdDepthStencilState.h"

#ifndef PAGE_SIZE
#define PAGE_SIZE 4096
#endif

class CosUmdAdapter;
class CosUmdRenderTargetView;
class CosUmdDepthStencilView;
class CosUmdShader;
class CosUmdElementLayout;

class CosUmdSampler;
class CosUmdShaderResourceView;

typedef union _CosUmdDeviceFlags
{
    struct
    {
#if VC4

        UINT    m_binningStarted : 1;       // Command buffer has binng start command

#endif

        UINT    m_hasDrawCall       : 1;    // Command buffer has draw call
    };

    UINT        m_value;
} CosUmdDeviceFlags;

//==================================================================================================================================
//
// CosUmdDevice
//
//==================================================================================================================================
class CosUmdDevice
{
public:
    explicit CosUmdDevice( CosUmdAdapter*, const D3D10DDIARG_CREATEDEVICE* );
    ~CosUmdDevice();

    void Standup();
    void Teardown();

    static CosUmdDevice* CastFrom( D3D10DDI_HDEVICE );
    static CosUmdDevice* CastFrom( DXGI_DDI_HDEVICE );
    D3D10DDI_HDEVICE CastTo() const;

public:

    void CreateResource(const D3D11DDIARG_CREATERESOURCE* pCreateResource, D3D10DDI_HRESOURCE hResource, D3D10DDI_HRTRESOURCE hRTResource);
    void OpenResource(const D3D10DDIARG_OPENRESOURCE*, D3D10DDI_HRESOURCE, D3D10DDI_HRTRESOURCE);
    void DestroyResource(CosUmdResource * pResource);
    void ResourceCopy(CosUmdResource *pDestinationResource, CosUmdResource * pSourceResource);
    void ResourceCopyRegion11_1(CosUmdResource *pDestinationResource, UINT DstSubresource, UINT DstX, UINT DstY, UINT DstZ, CosUmdResource * pSourceResource, UINT SrcSubresource, const D3D10_DDI_BOX* pSrcBox, UINT copyFlags);
    void ConstantBufferUpdateSubresourceUP(CosUmdResource *pDestinationResource, UINT DstSubresource, _In_opt_ const D3D10_DDI_BOX *pDstBox, _In_ const VOID *pSysMemUP, UINT RowPitch, UINT DepthPitch, UINT CopyFlags);

    void CreatePixelShader(const UINT* pCode, D3D10DDI_HSHADER hShader, D3D10DDI_HRTSHADER hRTShader, const D3D11_1DDIARG_STAGE_IO_SIGNATURES* pSignatures);
    void CreateVertexShader(const UINT* pCode, D3D10DDI_HSHADER hShader, D3D10DDI_HRTSHADER hRTShader, const D3D11_1DDIARG_STAGE_IO_SIGNATURES* pSignatures);
    void CreateGeometryShader(const UINT* pCode, D3D10DDI_HSHADER hShader, D3D10DDI_HRTSHADER hRTShader, const D3D11_1DDIARG_STAGE_IO_SIGNATURES* pSignatures);
    void CreateComputeShader(const UINT* pCode, D3D10DDI_HSHADER hShader, D3D10DDI_HRTSHADER hRTShader);
    void CreateTessellationShader(const UINT * pCode, D3D10DDI_HSHADER hShader, D3D10DDI_HRTSHADER hRTShader, const D3D11_1DDIARG_TESSELLATION_IO_SIGNATURES* pSignatures, D3D10_SB_TOKENIZED_PROGRAM_TYPE ProgramType);

    void DestroyShader(D3D10DDI_HSHADER hShader);

    void ResourceMap(CosUmdResource * pResource, UINT subResource, D3D10_DDI_MAP mapType, UINT mapFlags, D3D10DDI_MAPPED_SUBRESOURCE* pMappedSubRes);
    void ResourceUnmap(CosUmdResource * pResource, UINT subResource);

    void SetPredication(D3D10DDI_HQUERY hQuery, BOOL bPredicateValue);

public:

    void CheckFormatSupport(DXGI_FORMAT inFormat, UINT* pOutFormatSupport);
    void CheckCounterInfo(D3D10DDI_COUNTER_INFO* pOutCounterInfo);
    void CheckMultisampleQualityLevels(DXGI_FORMAT inFormat, UINT inSampleCount, UINT inFlags, UINT* pOutNumQualityLevels);

public:

    //
    // Kernel call backs
    //

    void Allocate(D3DDDICB_ALLOCATE * pAllocate);
    void Lock(D3DDDICB_LOCK * pLock);
    void Unlock(D3DDDICB_UNLOCK * pLock);
    void Render(D3DDDICB_RENDER * pRender);
    void DestroyContext(D3DDDICB_DESTROYCONTEXT * pDestroyContext);

    HRESULT Present(DXGI_DDI_ARG_PRESENT* Args);
    HRESULT RotateResourceIdentities(DXGI_DDI_ARG_ROTATE_RESOURCE_IDENTITIES* Args);
    HRESULT SetDisplayMode(DXGI_DDI_ARG_SETDISPLAYMODE* Args);
    HRESULT Present1(DXGI_DDI_ARG_PRESENT1* Args);

    void CheckDirectFlipSupport(
        D3D10DDI_HDEVICE hDevice,
        D3D10DDI_HRESOURCE hResource1,
        D3D10DDI_HRESOURCE hResource2,
        UINT CheckDirectFlipFlags,
        _Out_ BOOL *pSupported
        );
    
    //
    // User mode call backs
    //

    void SetError(HRESULT hr);

public:

    void SetException(const std::exception & e);
    void SetException(const CosUmdException & e);

public:
    HANDLE                          m_hContext;

    CosUmdAdapter*                  m_pAdapter;
    UINT                            m_Interface;
    D3D11DDI_3DPIPELINELEVEL        m_PipelineLevel;

    // Pointer to function table that contains the callbacks to the runtime. Runtime is free to change pointers, so do not cache
    // function pointers themselves.
    const D3DDDI_DEVICECALLBACKS*   m_pMSKTCallbacks;

    const D3D11DDI_CORELAYER_DEVICECALLBACKS*   m_pMSUMCallbacks;
    const DXGI_DDI_BASE_CALLBACKS*              m_pDXGICallbacks;

    // Handle for runtime device to use with runtime callbacks.
    D3D10DDI_HRTDEVICE              m_hRTDevice;
    D3D10DDI_HRTCORELAYER           m_hRTCoreLayer;

    // Pointer to function table that runtime will use. Driver is free to change function pointers while the driver has context
    // within one of these entry points.
    union
    {
        D3D10DDI_DEVICEFUNCS*       m_pDeviceFuncs;
        D3D10_1DDI_DEVICEFUNCS*     m_p10_1DeviceFuncs;
        D3D11DDI_DEVICEFUNCS*       m_p11DeviceFuncs;
        D3D11_1DDI_DEVICEFUNCS*     m_p11_1DeviceFuncs;
        D3DWDDM1_3DDI_DEVICEFUNCS*  m_pWDDM1_3DeviceFuncs;
        D3DWDDM2_0DDI_DEVICEFUNCS*  m_pWDDM2_0DeviceFuncs;
    };

    CosUmdCommandBuffer             m_commandBuffer;
    CosUmdDeviceFlags               m_flags;

    CosUmdResource                  m_dummyBuffer;

public:

    //
    // Draw Support
    //

    void Draw(UINT vertexCount, UINT startVertexLocation);
    void DrawIndexed(UINT indexCount, UINT startIndexLocation, INT baseVertexLocation);
    void ClearRenderTargetView(CosUmdRenderTargetView * pRenderTargetView, FLOAT clearColor[4]);
    void ClearDepthStencilView(CosUmdDepthStencilView * pDepthStencilView, UINT clearFlags, FLOAT depth, UINT8 stencil);

public:

    //
    // Graphics State Management
    //
    static const UINT kMaxVertexBuffers = D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT;
    static const UINT kMaxViewports = D3D10_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
    static const UINT kMaxRenderTargets = D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT;
    static const UINT kMaxSamplers = D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT;
    static const UINT kMaxConstantBuffers = D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT;
    static const UINT kMaxShaderResourceViews = D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT;

    void SetVertexBuffers(UINT startBuffer, UINT numBuffers, const D3D10DDI_HRESOURCE* phBuffers, const UINT* pStrides,  const UINT* pOffsets);
    void SetIndexBuffer(const D3D10DDI_HRESOURCE indexBuffer, DXGI_FORMAT indexFormat, UINT offset);
    void SetTopology(D3D10_DDI_PRIMITIVE_TOPOLOGY topology);
    void SetViewports(UINT numViewports, UINT clearViewports, const D3D10_DDI_VIEWPORT* pViewports);
    void SetRenderTargets(const D3D10DDI_HRENDERTARGETVIEW* phRenderTargetView, UINT NumRTVs, UINT RTVNumbertoUnbind,
        D3D10DDI_HDEPTHSTENCILVIEW hDepthStencilView, const D3D11DDI_HUNORDEREDACCESSVIEW* phUnorderedAccessView,
        const UINT* pUAVInitialCounts, UINT UAVIndex, UINT NumUAVs, UINT UAVFirsttoSet, UINT UAVNumberUpdated);
    void SetBlendState(CosUmdBlendState * pBlendState, const FLOAT pBlendFactor[4], UINT sampleMask);
    void SetPixelShader(CosUmdShader * pShader);
    void PSSetShaderResources(UINT, UINT, const D3D10DDI_HSHADERRESOURCEVIEW*);
    void PsSetConstantBuffers11_1(UINT, UINT, const D3D10DDI_HRESOURCE*, const UINT*, const UINT*);
    void SetPixelSamplers(UINT Offset, UINT NumSamplers, const D3D10DDI_HSAMPLER* phSamplers);
    void SetVertexShader(CosUmdShader * pShader);
    void SetVertexSamplers(UINT Offset, UINT NumSamplers, const D3D10DDI_HSAMPLER* phSamplers);
    void VsSetConstantBuffers11_1(UINT, UINT, const D3D10DDI_HRESOURCE*, const UINT*, const UINT*);
    void SetDomainShader(CosUmdShader * pShader);
    void SetDomainSamplers(UINT Offset, UINT NumSamplers, const D3D10DDI_HSAMPLER* phSamplers);
    void SetGeometryShader(CosUmdShader * pShader);
    void SetGeometrySamplers(UINT Offset, UINT NumSamplers, const D3D10DDI_HSAMPLER* phSamplers);
    void SetHullShader(CosUmdShader * pShader);
    void SetHullSamplers(UINT Offset, UINT NumSamplers, const D3D10DDI_HSAMPLER* phSamplers);
    void SetComputeShader(CosUmdShader * pShader);
    void SetComputeSamplers(UINT Offset, UINT NumSamplers, const D3D10DDI_HSAMPLER* phSamplers);
    void SetElementLayout(CosUmdElementLayout * pElementLayout);
    void SetDepthStencilState(CosUmdDepthStencilState * pDepthStencilState, UINT stencilRef);
    void SetRasterizerState(CosUmdRasterizerState * pRasterizerState);
    void SetScissorRects(UINT NumScissorRects, UINT ClearScissorRects, const D3D10_DDI_RECT *pRects);

    CosUmdResource *                m_vertexBuffers[kMaxVertexBuffers];
    UINT                            m_vertexStrides[kMaxVertexBuffers];
    UINT                            m_vertexOffsets[kMaxVertexBuffers];
    UINT                            m_numVertexBuffers;

    CosUmdResource *                m_indexBuffer;
    DXGI_FORMAT                     m_indexFormat;
    UINT                            m_indexOffset;

    D3D10_DDI_PRIMITIVE_TOPOLOGY    m_topology;

    D3D10_DDI_VIEWPORT              m_viewports[kMaxViewports];
    UINT                            m_numViewports;

    CosUmdRenderTargetView *        m_renderTargetViews[kMaxRenderTargets];
    UINT                            m_numRenderTargetViews;

    CosUmdDepthStencilView *        m_depthStencilView;

    CosUmdBlendState *              m_blendState;
    FLOAT                           m_blendFactor[4];
    UINT                            m_sampleMask;

    CosUmdShader *                  m_pixelShader;
    CosUmdShaderResourceView *      m_psResourceViews[kMaxShaderResourceViews];
    CosUmdSampler *                 m_pixelSamplers[kMaxSamplers];

    CosUmdResource *                m_psConstantBuffer[kMaxConstantBuffers];
    UINT                            m_ps1stConstant[kMaxConstantBuffers];
    UINT                            m_psNumberContants[kMaxConstantBuffers];

    CosUmdShader *                  m_vertexShader;
    CosUmdSampler *                 m_vertexSamplers[kMaxSamplers];

    CosUmdResource *                m_vsConstantBuffer[kMaxConstantBuffers];
    UINT                            m_vs1stConstant[kMaxConstantBuffers];
    UINT                            m_vsNumberContants[kMaxConstantBuffers];

    CosUmdShader *                  m_domainShader;
    CosUmdSampler *                 m_domainSamplers[kMaxSamplers];

    CosUmdShader *                  m_geometryShader;
    CosUmdSampler *                 m_geometrySamplers[kMaxSamplers];

    CosUmdShader *                  m_hullShader;
    CosUmdSampler *                 m_hullSamplers[kMaxSamplers];

    CosUmdShader *                  m_computeShader;
    CosUmdSampler *                 m_computeSamplers[kMaxSamplers];

    CosUmdElementLayout *           m_elementLayout;

    CosUmdDepthStencilState *       m_depthStencilState;
    UINT                            m_stencilRef;

    CosUmdRasterizerState *         m_rasterizerState;

    BOOL                            m_scissorRectSet;
    D3D10_DDI_RECT                  m_scissorRect;

    BOOL                            m_bPredicateValue;

public:

    void CreateInternalBuffer(CosUmdResource * pRes, UINT size);

private:

    //
    // Internal support routines
    //

    void RefreshPipelineState(UINT vertexOffset);

#if VC4

    void WriteUniforms(
        BOOLEAN                     bPSUniform,
        VC4_UNIFORM_FORMAT *        pUniformEntries,
        UINT                        numUniformEntries,
        BYTE *                     &pCurCommand,
        UINT                       &curCommandOffset,
        D3DDDI_PATCHLOCATIONLIST * &pCurPatchLocation);

#endif

public:
    void WriteEpilog();
};

inline CosUmdDevice* CosUmdDevice::CastFrom(D3D10DDI_HDEVICE hDevice)
{
    return static_cast< CosUmdDevice* >(hDevice.pDrvPrivate);
}

inline CosUmdDevice* CosUmdDevice::CastFrom(DXGI_DDI_HDEVICE hDevice)
{
    return reinterpret_cast< CosUmdDevice* >(hDevice);
}

inline D3D10DDI_HDEVICE CosUmdDevice::CastTo() const
{
    return MAKE_D3D10DDI_HDEVICE(const_cast< CosUmdDevice* >(this));
}

