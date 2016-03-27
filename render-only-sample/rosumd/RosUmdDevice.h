////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Device
//
// Copyright (C) Microsoft Corporation
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "RosUmdCommandBuffer.h"
#include "RosAllocation.h"
#include "RosUmdUtil.h"
#include "RosUmdDebug.h"

#include "RosUmdResource.h"

#include "RosUmdShader.h"

#include "RosUmdBlendState.h"
#include "RosUmdRasterizerState.h"
#include "RosUmdDepthStencilState.h"

#ifndef PAGE_SIZE
#define PAGE_SIZE 4096
#endif

class RosUmdAdapter;
class RosUmdRenderTargetView;
class RosUmdDepthStencilView;
class RosUmdShader;
class RosUmdElementLayout;

class RosUmdSampler;
class RosUmdShaderResourceView;

typedef union _RosUmdDeviceFlags
{
    struct
    {
#if VC4

        UINT    m_binningStarted : 1;       // Command buffer has binng start command

#endif

        UINT    m_hasDrawCall       : 1;    // Command buffer has draw call
    };

    UINT        m_value;
} RosUmdDeviceFlags;

//==================================================================================================================================
//
// RosUmdDevice
//
//==================================================================================================================================
class RosUmdDevice
{
public:
    explicit RosUmdDevice( RosUmdAdapter*, const D3D10DDIARG_CREATEDEVICE* );
    ~RosUmdDevice();

    void Standup();
    void Teardown();

    static RosUmdDevice* CastFrom( D3D10DDI_HDEVICE );
    static RosUmdDevice* CastFrom( DXGI_DDI_HDEVICE );
    D3D10DDI_HDEVICE CastTo() const;

public:

    void CreateResource(const D3D11DDIARG_CREATERESOURCE* pCreateResource, D3D10DDI_HRESOURCE hResource, D3D10DDI_HRTRESOURCE hRTResource);
    void OpenResource(const D3D10DDIARG_OPENRESOURCE*, D3D10DDI_HRESOURCE, D3D10DDI_HRTRESOURCE);
    void DestroyResource(RosUmdResource * pResource);
    void ResourceCopy(RosUmdResource *pDestinationResource, RosUmdResource * pSourceResource);
    void ResourceCopyRegion11_1(RosUmdResource *pDestinationResource, UINT DstSubresource, UINT DstX, UINT DstY, UINT DstZ, RosUmdResource * pSourceResource, UINT SrcSubresource, const D3D10_DDI_BOX* pSrcBox, UINT copyFlags);
    void ConstantBufferUpdateSubresourceUP(RosUmdResource *pDestinationResource, UINT DstSubresource, _In_opt_ const D3D10_DDI_BOX *pDstBox, _In_ const VOID *pSysMemUP, UINT RowPitch, UINT DepthPitch, UINT CopyFlags);

    void CreatePixelShader(const UINT* pCode, D3D10DDI_HSHADER hShader, D3D10DDI_HRTSHADER hRTShader, const D3D11_1DDIARG_STAGE_IO_SIGNATURES* pSignatures);
    void CreateVertexShader(const UINT* pCode, D3D10DDI_HSHADER hShader, D3D10DDI_HRTSHADER hRTShader, const D3D11_1DDIARG_STAGE_IO_SIGNATURES* pSignatures);
    void CreateGeometryShader(const UINT* pCode, D3D10DDI_HSHADER hShader, D3D10DDI_HRTSHADER hRTShader, const D3D11_1DDIARG_STAGE_IO_SIGNATURES* pSignatures);
    void CreateComputeShader(const UINT* pCode, D3D10DDI_HSHADER hShader, D3D10DDI_HRTSHADER hRTShader);
    void CreateTessellationShader(const UINT * pCode, D3D10DDI_HSHADER hShader, D3D10DDI_HRTSHADER hRTShader, const D3D11_1DDIARG_TESSELLATION_IO_SIGNATURES* pSignatures, D3D10_SB_TOKENIZED_PROGRAM_TYPE ProgramType);

    void DestroyShader(D3D10DDI_HSHADER hShader);

    void ResourceMap(RosUmdResource * pResource, UINT subResource, D3D10_DDI_MAP mapType, UINT mapFlags, D3D10DDI_MAPPED_SUBRESOURCE* pMappedSubRes);
    void ResourceUnmap(RosUmdResource * pResource, UINT subResource);

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
    void SetException(const RosUmdException & e);

public:
    HANDLE                          m_hContext;

    RosUmdAdapter*                  m_pAdapter;
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

    RosUmdCommandBuffer             m_commandBuffer;
    RosUmdDeviceFlags               m_flags;

    RosUmdResource                  m_dummyBuffer;

public:

    //
    // Draw Support
    //

    void Draw(UINT vertexCount, UINT startVertexLocation);
    void DrawIndexed(UINT indexCount, UINT startIndexLocation, INT baseVertexLocation);
    void ClearRenderTargetView(RosUmdRenderTargetView * pRenderTargetView, FLOAT clearColor[4]);
    void ClearDepthStencilView(RosUmdDepthStencilView * pDepthStencilView, UINT clearFlags, FLOAT depth, UINT8 stencil);

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
    void SetBlendState(RosUmdBlendState * pBlendState, const FLOAT pBlendFactor[4], UINT sampleMask);
    void SetPixelShader(RosUmdShader * pShader);
    void PSSetShaderResources(UINT, UINT, const D3D10DDI_HSHADERRESOURCEVIEW*);
    void PsSetConstantBuffers11_1(UINT, UINT, const D3D10DDI_HRESOURCE*, const UINT*, const UINT*);
    void SetPixelSamplers(UINT Offset, UINT NumSamplers, const D3D10DDI_HSAMPLER* phSamplers);
    void SetVertexShader(RosUmdShader * pShader);
    void SetVertexSamplers(UINT Offset, UINT NumSamplers, const D3D10DDI_HSAMPLER* phSamplers);
    void VsSetConstantBuffers11_1(UINT, UINT, const D3D10DDI_HRESOURCE*, const UINT*, const UINT*);
    void SetDomainShader(RosUmdShader * pShader);
    void SetDomainSamplers(UINT Offset, UINT NumSamplers, const D3D10DDI_HSAMPLER* phSamplers);
    void SetGeometryShader(RosUmdShader * pShader);
    void SetGeometrySamplers(UINT Offset, UINT NumSamplers, const D3D10DDI_HSAMPLER* phSamplers);
    void SetHullShader(RosUmdShader * pShader);
    void SetHullSamplers(UINT Offset, UINT NumSamplers, const D3D10DDI_HSAMPLER* phSamplers);
    void SetComputeShader(RosUmdShader * pShader);
    void SetComputeSamplers(UINT Offset, UINT NumSamplers, const D3D10DDI_HSAMPLER* phSamplers);
    void SetElementLayout(RosUmdElementLayout * pElementLayout);
    void SetDepthStencilState(RosUmdDepthStencilState * pDepthStencilState, UINT stencilRef);
    void SetRasterizerState(RosUmdRasterizerState * pRasterizerState);
    void SetScissorRects(UINT NumScissorRects, UINT ClearScissorRects, const D3D10_DDI_RECT *pRects);

    RosUmdResource *                m_vertexBuffers[kMaxVertexBuffers];
    UINT                            m_vertexStrides[kMaxVertexBuffers];
    UINT                            m_vertexOffsets[kMaxVertexBuffers];
    UINT                            m_numVertexBuffers;

    RosUmdResource *                m_indexBuffer;
    DXGI_FORMAT                     m_indexFormat;
    UINT                            m_indexOffset;

    D3D10_DDI_PRIMITIVE_TOPOLOGY    m_topology;

    D3D10_DDI_VIEWPORT              m_viewports[kMaxViewports];
    UINT                            m_numViewports;

    RosUmdRenderTargetView *        m_renderTargetViews[kMaxRenderTargets];
    UINT                            m_numRenderTargetViews;

    RosUmdDepthStencilView *        m_depthStencilView;

    RosUmdBlendState *              m_blendState;
    FLOAT                           m_blendFactor[4];
    UINT                            m_sampleMask;

    RosUmdShader *                  m_pixelShader;
    RosUmdShaderResourceView *      m_psResourceViews[kMaxShaderResourceViews];
    RosUmdSampler *                 m_pixelSamplers[kMaxSamplers];

    RosUmdResource *                m_psConstantBuffer[kMaxConstantBuffers];
    UINT                            m_ps1stConstant[kMaxConstantBuffers];
    UINT                            m_psNumberContants[kMaxConstantBuffers];

    RosUmdShader *                  m_vertexShader;
    RosUmdSampler *                 m_vertexSamplers[kMaxSamplers];

    RosUmdResource *                m_vsConstantBuffer[kMaxConstantBuffers];
    UINT                            m_vs1stConstant[kMaxConstantBuffers];
    UINT                            m_vsNumberContants[kMaxConstantBuffers];

    RosUmdShader *                  m_domainShader;
    RosUmdSampler *                 m_domainSamplers[kMaxSamplers];

    RosUmdShader *                  m_geometryShader;
    RosUmdSampler *                 m_geometrySamplers[kMaxSamplers];

    RosUmdShader *                  m_hullShader;
    RosUmdSampler *                 m_hullSamplers[kMaxSamplers];

    RosUmdShader *                  m_computeShader;
    RosUmdSampler *                 m_computeSamplers[kMaxSamplers];

    RosUmdElementLayout *           m_elementLayout;

    RosUmdDepthStencilState *       m_depthStencilState;
    UINT                            m_stencilRef;

    RosUmdRasterizerState *         m_rasterizerState;

    BOOL                            m_scissorRectSet;
    D3D10_DDI_RECT                  m_scissorRect;

    BOOL                            m_bPredicateValue;

public:

    void CreateInternalBuffer(RosUmdResource * pRes, UINT size);

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

    VC4TextureType MapDXGITextureFormatToVC4Type(
        RosHwLayout layout,
        DXGI_FORMAT format);

#endif

public:
    void WriteEpilog();
};

inline RosUmdDevice* RosUmdDevice::CastFrom(D3D10DDI_HDEVICE hDevice)
{
    return static_cast< RosUmdDevice* >(hDevice.pDrvPrivate);
}

inline RosUmdDevice* RosUmdDevice::CastFrom(DXGI_DDI_HDEVICE hDevice)
{
    return reinterpret_cast< RosUmdDevice* >(hDevice);
}

inline D3D10DDI_HDEVICE RosUmdDevice::CastTo() const
{
    return MAKE_D3D10DDI_HDEVICE(const_cast< RosUmdDevice* >(this));
}

