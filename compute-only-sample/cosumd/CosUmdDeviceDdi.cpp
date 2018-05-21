#include "CosUmd.h"

#include "CosUmdLogging.h"
#include "CosUmdDeviceDdi.tmh"

#include "CosUmdDeviceDdi.h"
#include "CosUmdDevice.h"
#include "CosUmdResource.h"
#include "CosUmdDebug.h"
#include "CosUmdRasterizerState.h"
#include "CosUmdDepthStencilState.h"
#include "CosUmdSampler.h"
#include "CosUmdElementLayout.h"
#include "CosUmdShader.h"
#include "CosUmdBlendState.h"
#include "CosUmdRenderTargetView.h"
#include "CosUmdDepthStencilView.h"
#include "CosUmdShaderResourceView.h"
#include "CosUmdUnorderedAccessView.h"

#include "CosContext.h"

//
// Ddi Tables
//

const D3DWDDM1_3DDI_DEVICEFUNCS CosUmdDeviceDdi::s_deviceFuncsWDDM1_3 =
{
    CosUmdDeviceDdi::DdiConstantBufferUpdateSubresourceUP11_1,
    CosUmdDeviceDdi::DdiVsSetConstantBuffers11_1,
    CosUmdDeviceDdi::DdiPSSetShaderResources,
    CosUmdDeviceDdi::DdiPsSetShader,
    CosUmdDeviceDdi::DdiPSSetSamplers,
    CosUmdDeviceDdi::DdiVsSetShader,
    CosUmdDeviceDdi::DdiDrawIndexed,
    CosUmdDeviceDdi::DdiDraw,
    CosUmdDeviceDdi::DdiDynamicIABufferMapNoOverwrite,
    CosUmdDeviceDdi::DdiDynamicIABufferUnmap,
    CosUmdDeviceDdi::DdiDynamicConstantBufferMapDiscard,
    CosUmdDeviceDdi::DdiDynamicIABufferMapDiscard,
    CosUmdDeviceDdi::DdiDynamicConstantBufferUnmap,
    CosUmdDeviceDdi::DdiPsSetConstantBuffers11_1,
    CosUmdDeviceDdi::DdiIaSetInputLayout,
    CosUmdDeviceDdi::DdiIaSetVertexBuffers,
    CosUmdDeviceDdi::DdiIaSetIndexBuffer,

    CosUmdDeviceDdi::DrawIndexedInstanced_Dirty,
    CosUmdDeviceDdi::DrawInstanced_Dirty,
    CosUmdDeviceDdi::DynamicResourceMapDiscard_Default,
    CosUmdDeviceDdi::DynamicResourceUnmap_Default,
    CosUmdDeviceDdi::GsSetConstantBuffers11_1_Default,
    CosUmdDeviceDdi::DdiGsSetShader,
    CosUmdDeviceDdi::DdiIaSetTopology,
    CosUmdDeviceDdi::DdiStagingResourceMap,
    CosUmdDeviceDdi::DdiStagingResourceUnmap,
    CosUmdDeviceDdi::VSSetShaderResources_Default,
    CosUmdDeviceDdi::DdiVSSetSamplers,
    CosUmdDeviceDdi::GSSetShaderResources_Default,
    CosUmdDeviceDdi::DdiGSSetSamplers,
    CosUmdDeviceDdi::DdiSetRenderTargets,
    CosUmdDeviceDdi::ShaderResourceViewReadAfterWriteHazard_Default,
    CosUmdDeviceDdi::ResourceReadAfterWriteHazard_Default,
    CosUmdDeviceDdi::DdiSetBlendState,
    CosUmdDeviceDdi::DdiSetDepthStencilState,
    CosUmdDeviceDdi::DdiSetRasterizerState,
    CosUmdDeviceDdi::QueryEnd_Default,
    CosUmdDeviceDdi::QueryBegin_Default,
    CosUmdDeviceDdi::DdiResourceCopyRegion11_1,
    CosUmdDeviceDdi::ResourceUpdateSubresourceUP11_1_Default,
    CosUmdDeviceDdi::SOSetTargets_Default,
    CosUmdDeviceDdi::DrawAuto_Dirty,
    CosUmdDeviceDdi::DdiSetViewports,
    CosUmdDeviceDdi::DdiSetScissorRects,
    CosUmdDeviceDdi::DdiClearRenderTargetView,
    CosUmdDeviceDdi::DdiClearDepthStencilView,
    CosUmdDeviceDdi::DdiSetPredication,
    CosUmdDeviceDdi::QueryGetData_Default,
    CosUmdDeviceDdi::DdiFlush,
    CosUmdDeviceDdi::GenerateMips_Default,
    CosUmdDeviceDdi::DdiResourceCopy,
    CosUmdDeviceDdi::ResourceResolveSubresource_Default,

    CosUmdDeviceDdi::ResourceMap_Default,
    CosUmdDeviceDdi::ResourceUnmap_Default,
    CosUmdDeviceDdi::ResourceIsStagingBusy_Default,
    CosUmdDeviceDdi::RelocateDeviceFuncs11_1_Default,
    CosUmdDeviceDdi::DdiCalcPrivateResourceSize,
    CosUmdDeviceDdi::DdiCalcPrivateOpenedResourceSize,
    CosUmdDeviceDdi::DdiCreateResource,
    CosUmdDeviceDdi::DdiOpenResource,
    CosUmdDeviceDdi::DdiDestroyResource,
    CosUmdDeviceDdi::DdiCalcPrivateShaderResourceViewSize11,
    CosUmdDeviceDdi::DdiCreateShaderResourceView11,
    CosUmdDeviceDdi::DdiDestroyShaderResourceView,
    CosUmdDeviceDdi::DdiCalcPrivateRenderTargetViewSize,
    CosUmdDeviceDdi::DdiCreateRenderTargetView,
    CosUmdDeviceDdi::DdiDestroyRenderTargetView,
    CosUmdDeviceDdi::DdiCalcPrivateDepthStencilViewSize11,
    CosUmdDeviceDdi::DdiCreateDepthStencilView11,
    CosUmdDeviceDdi::DdiDestroyDepthStencilView,
    CosUmdDeviceDdi::DdiCalcPrivateElementLayoutSize,
    CosUmdDeviceDdi::DdiCreateElementLayout,
    CosUmdDeviceDdi::DdiDestroyElementLayout,
    CosUmdDeviceDdi::DdiCalcPrivateBlendStateSize,
    CosUmdDeviceDdi::DdiCreateBlendState,
    CosUmdDeviceDdi::DdiDestroyBlendState,
    CosUmdDeviceDdi::DdiCalcPrivateDepthStencilStateSize,
    CosUmdDeviceDdi::DdiCreateDepthStencilState,
    CosUmdDeviceDdi::DdiDestroyDepthStencilState,
    CosUmdDeviceDdi::DdiCalcRasterizerStateSize,
    CosUmdDeviceDdi::DdiCreateRasterizerState,
    CosUmdDeviceDdi::DdiDestroyRasterizerState,
    CosUmdDeviceDdi::DdiCalcPrivateShaderSize,
    CosUmdDeviceDdi::DdiCreateVertexShader,
    CosUmdDeviceDdi::DdiCreateGeometryShader,
    CosUmdDeviceDdi::DdiCreatePixelShader,
    CosUmdDeviceDdi::CalcPrivateGeometryShaderWithStreamOutputSize11_1_Default,
    CosUmdDeviceDdi::CreateGeometryShaderWithStreamOutput11_1_Default,
    CosUmdDeviceDdi::DdiDestroyShader,
    CosUmdDeviceDdi::DdiCalcPrivateSamplerSize,
    CosUmdDeviceDdi::DdiCreateSampler,
    CosUmdDeviceDdi::DdiDestroySampler,
    CosUmdDeviceDdi::CalcPrivateQuerySize_Default,
    CosUmdDeviceDdi::CreateQuery_Default,
    CosUmdDeviceDdi::DestroyQuery_Default,

    CosUmdDeviceDdi::DdiCheckFormatSupport,
    CosUmdDeviceDdi::DdiCheckMultisampleQualityLevels,
    CosUmdDeviceDdi::DdiCheckCounterInfo,
    CosUmdDeviceDdi::CheckCounter_Default,
    CosUmdDeviceDdi::DdiDestroyDevice,
    CosUmdDeviceDdi::SetTextFilter_Default,
    CosUmdDeviceDdi::DdiResourceCopy,
    CosUmdDeviceDdi::DdiResourceCopyRegion11_1,

    CosUmdDeviceDdi::DrawIndexedInstancedIndirect_Dirty,
    CosUmdDeviceDdi::DrawInstancedIndirect_Dirty,
    NULL, // CosUmdDeviceDdi::CommandListExecute_Default,
    CosUmdDeviceDdi::HSSetShaderResources_Default,
    CosUmdDeviceDdi::DdiHsSetShader,
    CosUmdDeviceDdi::DdiHSSetSamplers,
    CosUmdDeviceDdi::HsSetConstantBuffers11_1_Default,
    CosUmdDeviceDdi::DSSetShaderResources_Default,
    CosUmdDeviceDdi::DdiDsSetShader,
    CosUmdDeviceDdi::DdiDSSetSamplers,
    CosUmdDeviceDdi::DsSetConstantBuffers11_1_Default,
    CosUmdDeviceDdi::DdiCreateHullShader,
    CosUmdDeviceDdi::DdiCreateDomainShader,
    NULL, // CosUmdDeviceDdi::CheckDeferredContextHandleSizes_Default,
    NULL, // CosUmdDeviceDdi::CalcDeferredContextHandleSize_Default,
    NULL, // CosUmdDeviceDdi::CalcPrivateDeferredContextSize_Default,
    NULL, // CosUmdDeviceDdi::CreateDeferredContext_Default,
    NULL, // CosUmdDeviceDdi::AbandonCommandList_Default,
    NULL, // CosUmdDeviceDdi::CalcPrivateCommandListSize_Default,
    NULL, // CosUmdDeviceDdi::CreateCommandList_Default,
    NULL, // CosUmdDeviceDdi::DestroyCommandList_Default,
    CosUmdDeviceDdi::DdiCalcPrivateTessellationShaderSize,
    CosUmdDeviceDdi::PsSetShaderWithInterfaces_Default,
    CosUmdDeviceDdi::VsSetShaderWithInterfaces_Default,
    CosUmdDeviceDdi::GsSetShaderWithInterfaces_Default,
    CosUmdDeviceDdi::HsSetShaderWithInterfaces_Default,
    CosUmdDeviceDdi::DsSetShaderWithInterfaces_Default,
    CosUmdDeviceDdi::CsSetShaderWithInterfaces_Default,
    CosUmdDeviceDdi::DdiCreateComputeShader,
    CosUmdDeviceDdi::DdiCsSetShader,
    CosUmdDeviceDdi::CSSetShaderResources,
    CosUmdDeviceDdi::DdiCSSetSamplers,
    CosUmdDeviceDdi::CsSetConstantBuffers11_1_Default,
    CosUmdDeviceDdi::DdiCalcPrivateUnorderedAccessViewSize,
    CosUmdDeviceDdi::DdiCreateUnorderedAccessView,
    CosUmdDeviceDdi::DdiDestroyUnorderedAccessView,
    CosUmdDeviceDdi::ClearUnorderedAccessViewUint,
    CosUmdDeviceDdi::ClearUnorderedAccessViewFloat,
    CosUmdDeviceDdi::CSSetUnorderedAccessViews,
    CosUmdDeviceDdi::Dispatch,
    CosUmdDeviceDdi::DispatchIndirect_Dirty,
    CosUmdDeviceDdi::SetResourceMinLOD_Default,
    CosUmdDeviceDdi::CopyStructureCount_Default,
    NULL, // CosUmdDeviceDdi::RecycleCommandList_Default,
    NULL, // CosUmdDeviceDdi::RecycleCreateCommandList_Default,
    NULL, // CosUmdDeviceDdi::RecycleCreateDeferredContext_Default,
    NULL, // CosUmdDeviceDdi::RecycleDestroyCommandList_Default,
    CosUmdDeviceDdi::Discard_Default,
    CosUmdDeviceDdi::AssignDebugBinary_Default,
    CosUmdDeviceDdi::DynamicConstantBufferMapNoOverwrite_Default,
    CosUmdDeviceDdi::CheckDirectFlipSupport,
    CosUmdDeviceDdi::ClearView_Default,
    NULL, // PFND3DWDDM1_3DDI_UPDATETILEMAPPINGS
    NULL, // PFND3DWDDM1_3DDI_COPYTILEMAPPINGS
    NULL, // PFND3DWDDM1_3DDI_COPYTILES
    NULL, // PFND3DWDDM1_3DDI_UPDATETILES
    NULL, // PFND3DWDDM1_3DDI_TILEDRESOURCEBARRIER
    NULL, // PFND3DWDDM1_3DDI_GETMIPPACKING
    NULL, // PFND3DWDDM1_3DDI_RESIZETILEPOOL
    NULL, // PFND3DWDDM1_3DDI_SETMARKER
    NULL, // PFND3DWDDM1_3DDI_SETMARKERMODE
};

const DXGI1_3_DDI_BASE_FUNCTIONS CosUmdDeviceDdi::s_dxgiDeviceFuncs4 =
{
    CosUmdDeviceDdi::Present,
    NULL, // CosUmdDeviceDdi::GetGammaCaps,     //GetGammaCaps
    CosUmdDeviceDdi::SetDisplayMode,
    CosUmdDeviceDdi::SetResourcePriority,
    NULL, // CosUmdDeviceDdi::QueryResourceResidency,
    CosUmdDeviceDdi::RotateResourceIdentities,
    NULL, // CosUmdDeviceDdi::Blt,
    CosUmdDeviceDdi::ResolveSharedResource,
    NULL, // CosUmdDeviceDdi::Blt1,
    NULL, // pfnOfferResources
    NULL, // pfnReclaimResources
    NULL, // pfnGetMultiplaneOverlayCaps
    NULL, // pfnGetMultiplaneOverlayGroupCaps
    NULL, // pfnReserved1
    NULL, // pfnPresentMultiplaneOverlay
    NULL, // pfnReserved2
    CosUmdDeviceDdi::Present1, // pfnPresent1
    NULL, // pfnCheckPresentDurationSupport
};

//
// DDI entry points
//

void APIENTRY CosUmdDeviceDdi::DdiDestroyDevice(
    D3D10DDI_HDEVICE hDevice)
{
    CosUmdDevice* pCosUmdDevice = CosUmdDevice::CastFrom(hDevice);

    try {
        pCosUmdDevice->Teardown();
        pCosUmdDevice->~CosUmdDevice();
    }

    catch (std::exception &)
    {
        // do nothing
    }
}

void APIENTRY CosUmdDeviceDdi::DdiCreateResource(
    D3D10DDI_HDEVICE hDevice,
    const D3D11DDIARG_CREATERESOURCE* pCreateResource,
    D3D10DDI_HRESOURCE hResource,
    D3D10DDI_HRTRESOURCE hRTResource)
{
    CosUmdDevice* pCosUmdDevice = CosUmdDevice::CastFrom(hDevice);

    try {
        pCosUmdDevice->CreateResource(pCreateResource, hResource, hRTResource);
    }

    catch (std::exception & e)
    {
        pCosUmdDevice->SetException(e);
    }
}

void APIENTRY CosUmdDeviceDdi::DdiOpenResource(
    D3D10DDI_HDEVICE hDevice,
    const D3D10DDIARG_OPENRESOURCE* Args,
    D3D10DDI_HRESOURCE hResource,
    D3D10DDI_HRTRESOURCE hRTResource)
{
    CosUmdDevice* pCosUmdDevice = CosUmdDevice::CastFrom(hDevice);

    try {
        pCosUmdDevice->OpenResource(Args, hResource, hRTResource);
    }

    catch (const std::exception & e)
    {
        pCosUmdDevice->SetException(e);
    }
}

void APIENTRY CosUmdDeviceDdi::DdiDestroyResource(
    D3D10DDI_HDEVICE hDevice,
    D3D10DDI_HRESOURCE hResource)
{   
    CosUmdDevice* pCosUmdDevice = CosUmdDevice::CastFrom(hDevice);

    CosUmdResource * pResource = (CosUmdResource *)hResource.pDrvPrivate;

    pCosUmdDevice->DestroyResource(pResource);
}

SIZE_T APIENTRY CosUmdDeviceDdi::DdiCalcPrivateShaderResourceViewSize11(
    D3D10DDI_HDEVICE,
    const D3D11DDIARG_CREATESHADERRESOURCEVIEW*)
{
    return sizeof(CosUmdShaderResourceView);
}

void APIENTRY CosUmdDeviceDdi::DdiCreateShaderResourceView11(
    D3D10DDI_HDEVICE,
    const D3D11DDIARG_CREATESHADERRESOURCEVIEW* pCreate,
    D3D10DDI_HSHADERRESOURCEVIEW hShaderResourceView,
    D3D10DDI_HRTSHADERRESOURCEVIEW hRTShaderResourceView)
{
    new(hShaderResourceView.pDrvPrivate) CosUmdShaderResourceView(pCreate, hRTShaderResourceView);
}

void APIENTRY CosUmdDeviceDdi::DdiDestroyShaderResourceView(
    D3D10DDI_HDEVICE,
    D3D10DDI_HSHADERRESOURCEVIEW hShaderResourceView)
{
    CosUmdShaderResourceView * pShaderResourceView = CosUmdShaderResourceView::CastFrom(hShaderResourceView);
    pShaderResourceView->~CosUmdShaderResourceView();
}

void APIENTRY CosUmdDeviceDdi::DdiResourceCopy(
    D3D10DDI_HDEVICE hDevice,
    D3D10DDI_HRESOURCE hDestinationResource,
    D3D10DDI_HRESOURCE hSourceResource)
{
    CosUmdDevice* pCosUmdDevice = CosUmdDevice::CastFrom(hDevice);
    CosUmdResource * pDestinationResource = (CosUmdResource *)hDestinationResource.pDrvPrivate;
    CosUmdResource * pSourceResource = (CosUmdResource *)hSourceResource.pDrvPrivate;

    try
    {
        pCosUmdDevice->ResourceCopy(pDestinationResource, pSourceResource);
    }

    catch (std::exception & e)
    {
        pCosUmdDevice->SetException(e);
    }
}

void APIENTRY CosUmdDeviceDdi::DdiConstantBufferUpdateSubresourceUP11_1(
    D3D10DDI_HDEVICE   hDevice,
    D3D10DDI_HRESOURCE hDstResource,
    UINT               DstSubresource,
    _In_opt_ const D3D10_DDI_BOX  *pDstBox,
    _In_     const VOID           *pSysMemUP,
    UINT               RowPitch,
    UINT               DepthPitch,
    UINT               CopyFlags)
{
    CosUmdDevice* pCosUmdDevice = CosUmdDevice::CastFrom(hDevice);
    CosUmdResource * pResource = (CosUmdResource *)hDstResource.pDrvPrivate;

    try
    {
       pCosUmdDevice->ConstantBufferUpdateSubresourceUP(pResource, DstSubresource, pDstBox, pSysMemUP, RowPitch, DepthPitch, CopyFlags);
    }

    catch (std::exception & e)
    {
        pCosUmdDevice->SetException(e);
    }
}

void APIENTRY CosUmdDeviceDdi::DdiStagingResourceMap(
    D3D10DDI_HDEVICE hDevice,
    D3D10DDI_HRESOURCE hResource,
    UINT subResource,
    D3D10_DDI_MAP mapType,
    UINT mapFlags,
    D3D10DDI_MAPPED_SUBRESOURCE* pMappedSubRes)
{
    CosUmdDevice* pCosUmdDevice = CosUmdDevice::CastFrom(hDevice);
    CosUmdResource * pResource = (CosUmdResource *)hResource.pDrvPrivate;

    try
    {
        pCosUmdDevice->ResourceMap(pResource, subResource, mapType, mapFlags, pMappedSubRes);
    }

    catch (std::exception & e)
    {
        pCosUmdDevice->SetException(e);
    }
}

void APIENTRY CosUmdDeviceDdi::DdiStagingResourceUnmap(
    D3D10DDI_HDEVICE hDevice,
    D3D10DDI_HRESOURCE hResource,
    UINT subResource)
{
    CosUmdDevice* pCosUmdDevice = CosUmdDevice::CastFrom(hDevice);
    CosUmdResource * pResource = (CosUmdResource *)hResource.pDrvPrivate;

    try
    {
        pCosUmdDevice->ResourceUnmap(pResource, subResource);
    }

    catch (std::exception & e)
    {
        pCosUmdDevice->SetException(e);
    }
}

BOOL APIENTRY CosUmdDeviceDdi::DdiFlush(D3D10DDI_HDEVICE hDevice, UINT flushFlags)
{
    CosUmdDevice* pCosUmdDevice = CosUmdDevice::CastFrom(hDevice);

    BOOL bSuccess = TRUE;

    try
    {
        pCosUmdDevice->m_commandBuffer.Flush(flushFlags);
    }

    catch (std::exception & e)
    {
        pCosUmdDevice->SetException(e);
        bSuccess = FALSE;
    }

    return bSuccess;
}

void APIENTRY CosUmdDeviceDdi::DdiCheckFormatSupport(
    D3D10DDI_HDEVICE hDevice,
    DXGI_FORMAT Format,
    UINT* pFormatSupport)
{
    CosUmdDevice* pCosUmdDevice = CosUmdDevice::CastFrom(hDevice);

    pCosUmdDevice->CheckFormatSupport(Format, pFormatSupport);
}

void APIENTRY CosUmdDeviceDdi::DdiCheckCounterInfo(
    D3D10DDI_HDEVICE hDevice,
    D3D10DDI_COUNTER_INFO* pCounterInfo)
{
    CosUmdDevice* pCosUmdDevice = CosUmdDevice::CastFrom(hDevice);
    pCosUmdDevice->CheckCounterInfo(pCounterInfo);
}

void APIENTRY CosUmdDeviceDdi::DdiCheckMultisampleQualityLevels(
    D3D10DDI_HDEVICE hDevice,
    DXGI_FORMAT Format,
    UINT SampleCount,
    UINT Flags,
    UINT* pNumQualityLevels)
{
    CosUmdDevice* pCosUmdDevice = CosUmdDevice::CastFrom(hDevice);
    pCosUmdDevice->CheckMultisampleQualityLevels(Format, SampleCount, Flags, pNumQualityLevels);
}

SIZE_T APIENTRY CosUmdDeviceDdi::DdiCalcPrivateResourceSize(
    D3D10DDI_HDEVICE,
    const D3D11DDIARG_CREATERESOURCE*)
{
    return sizeof(CosUmdResource);
}

SIZE_T CosUmdDeviceDdi::DdiCalcPrivateOpenedResourceSize(
    D3D10DDI_HDEVICE,
    const D3D10DDIARG_OPENRESOURCE*)
{
    return sizeof(CosUmdResource);
}

//
// Rasterizer State
//

SIZE_T APIENTRY CosUmdDeviceDdi::DdiCalcRasterizerStateSize(
    D3D10DDI_HDEVICE,
    const D3D11_1_DDI_RASTERIZER_DESC*)
{
    return sizeof(CosUmdRasterizerState);
}

void APIENTRY CosUmdDeviceDdi::DdiCreateRasterizerState(
    D3D10DDI_HDEVICE,
    const D3D11_1_DDI_RASTERIZER_DESC* desc,
    D3D10DDI_HRASTERIZERSTATE hRasterizerState,
    D3D10DDI_HRTRASTERIZERSTATE hRTRasterizerState)
{
    CosUmdRasterizerState* pRasterizerState = new (hRasterizerState.pDrvPrivate) CosUmdRasterizerState(desc, hRTRasterizerState);
    pRasterizerState; // unused
}

void APIENTRY CosUmdDeviceDdi::DdiDestroyRasterizerState(
    D3D10DDI_HDEVICE hDevice,
    D3D10DDI_HRASTERIZERSTATE hRasterizerState)
{
    CosUmdDevice * pDevice = CosUmdDevice::CastFrom(hDevice);
    CosUmdRasterizerState * pRasterizerState = CosUmdRasterizerState::CastFrom(hRasterizerState);

    pDevice; // unused
    pRasterizerState; // unused
}

void APIENTRY CosUmdDeviceDdi::DdiSetRasterizerState(
    D3D10DDI_HDEVICE hDevice,
    D3D10DDI_HRASTERIZERSTATE hRasterizerState)
{
    CosUmdDevice * pDevice = CosUmdDevice::CastFrom(hDevice);
    CosUmdRasterizerState * pRasterizerState = CosUmdRasterizerState::CastFrom(hRasterizerState);
    pDevice->SetRasterizerState(pRasterizerState);
}

void APIENTRY CosUmdDeviceDdi::DdiSetScissorRects(
    _In_       D3D10DDI_HDEVICE hDevice,
    _In_       UINT             NumScissorRects,
    _In_       UINT             ClearScissorRects,
    _In_ const D3D10_DDI_RECT   *pRects
    )
{
    CosUmdDevice * pDevice = CosUmdDevice::CastFrom(hDevice);
    pDevice->SetScissorRects(NumScissorRects, ClearScissorRects, pRects);
}

//
// Depth Stencil State
//

SIZE_T APIENTRY CosUmdDeviceDdi::DdiCalcPrivateDepthStencilStateSize(
    D3D10DDI_HDEVICE hDevice,
    const D3D10_DDI_DEPTH_STENCIL_DESC* desc)
{
    CosUmdDevice * pDevice = CosUmdDevice::CastFrom(hDevice);
    pDevice; // unused
    desc; // unused

    return sizeof(CosUmdDepthStencilState);
}

void APIENTRY CosUmdDeviceDdi::DdiCreateDepthStencilState(
    D3D10DDI_HDEVICE hDevice,
    const D3D10_DDI_DEPTH_STENCIL_DESC* desc,
    D3D10DDI_HDEPTHSTENCILSTATE hDepthStencilState,
    D3D10DDI_HRTDEPTHSTENCILSTATE hRTDepthStencilState)
{
    CosUmdDevice * pDevice = CosUmdDevice::CastFrom(hDevice);
    pDevice; // unused

    CosUmdDepthStencilState* pDepthStencilState = new (hDepthStencilState.pDrvPrivate) CosUmdDepthStencilState(desc, hRTDepthStencilState);
    pDepthStencilState; // unused
}

void APIENTRY CosUmdDeviceDdi::DdiDestroyDepthStencilState(
    D3D10DDI_HDEVICE hDevice,
    D3D10DDI_HDEPTHSTENCILSTATE hDepthStencilState)
{
    CosUmdDevice * pDevice = CosUmdDevice::CastFrom(hDevice);
    CosUmdDepthStencilState * pDepthStencilState = CosUmdDepthStencilState::CastFrom(hDepthStencilState);

    pDevice; // unusd
    pDepthStencilState; // unused

}

void APIENTRY CosUmdDeviceDdi::DdiSetDepthStencilState(
    D3D10DDI_HDEVICE hDevice,
    D3D10DDI_HDEPTHSTENCILSTATE hDepthStencilState,
    UINT StencilRef)
{
    CosUmdDevice * pDevice = CosUmdDevice::CastFrom(hDevice);
    CosUmdDepthStencilState * pDepthStencilState = CosUmdDepthStencilState::CastFrom(hDepthStencilState);
    pDevice->SetDepthStencilState(pDepthStencilState, StencilRef);
}

//
// Sampler
//

SIZE_T APIENTRY CosUmdDeviceDdi::DdiCalcPrivateSamplerSize(
    D3D10DDI_HDEVICE hDevice,
    const D3D10_DDI_SAMPLER_DESC* desc)
{
    CosUmdDevice * pDevice = CosUmdDevice::CastFrom(hDevice);
    pDevice; // unused
    desc; // unused

    return sizeof(CosUmdSampler);
}

void APIENTRY CosUmdDeviceDdi::DdiCreateSampler(
    D3D10DDI_HDEVICE hDevice,
    const D3D10_DDI_SAMPLER_DESC* desc,
    D3D10DDI_HSAMPLER hSampler,
    D3D10DDI_HRTSAMPLER hRTSampler)
{
    CosUmdDevice * pDevice = CosUmdDevice::CastFrom(hDevice);
    pDevice; // unusd

    CosUmdSampler* pSampler = new (hSampler.pDrvPrivate) CosUmdSampler(desc, hRTSampler);
    pSampler; // unused
}

void APIENTRY CosUmdDeviceDdi::DdiDestroySampler(
    D3D10DDI_HDEVICE hDevice,
    D3D10DDI_HSAMPLER hSampler)
{
    CosUmdDevice * pDevice = CosUmdDevice::CastFrom(hDevice);
    pDevice; // unusd

    CosUmdSampler * pSampler = CosUmdSampler::CastFrom(hSampler);
    pSampler; // unused
}

void APIENTRY CosUmdDeviceDdi::DdiPSSetSamplers(
    D3D10DDI_HDEVICE hDevice,
    UINT Offset,
    UINT NumSamplers,
    const D3D10DDI_HSAMPLER* phSamplers)
{
    CosUmdDevice * pDevice = CosUmdDevice::CastFrom(hDevice);
    pDevice->SetPixelSamplers(Offset, NumSamplers, phSamplers);
}

void APIENTRY CosUmdDeviceDdi::DdiVSSetSamplers(
    D3D10DDI_HDEVICE hDevice,
    UINT Offset,
    UINT NumSamplers,
    const D3D10DDI_HSAMPLER* phSamplers)
{
    CosUmdDevice * pDevice = CosUmdDevice::CastFrom(hDevice);
    pDevice->SetVertexSamplers(Offset, NumSamplers, phSamplers);
}

void APIENTRY CosUmdDeviceDdi::DdiGSSetSamplers(
    D3D10DDI_HDEVICE hDevice,
    UINT Offset,
    UINT NumSamplers,
    const D3D10DDI_HSAMPLER* phSamplers)
{
    CosUmdDevice * pDevice = CosUmdDevice::CastFrom(hDevice);
    pDevice->SetGeometrySamplers(Offset, NumSamplers, phSamplers);
}

void APIENTRY CosUmdDeviceDdi::DdiCSSetSamplers(
    D3D10DDI_HDEVICE hDevice,
    UINT Offset,
    UINT NumSamplers,
    const D3D10DDI_HSAMPLER* phSamplers)
{
    CosUmdDevice * pDevice = CosUmdDevice::CastFrom(hDevice);
    pDevice->SetComputeSamplers(Offset, NumSamplers, phSamplers);
}

void APIENTRY CosUmdDeviceDdi::DdiHSSetSamplers(
    D3D10DDI_HDEVICE hDevice,
    UINT Offset,
    UINT NumSamplers,
    const D3D10DDI_HSAMPLER* phSamplers)
{
    CosUmdDevice * pDevice = CosUmdDevice::CastFrom(hDevice);
    pDevice->SetHullSamplers(Offset, NumSamplers, phSamplers);
}

void APIENTRY CosUmdDeviceDdi::DdiDSSetSamplers(
    D3D10DDI_HDEVICE hDevice,
    UINT Offset,
    UINT NumSamplers,
    const D3D10DDI_HSAMPLER* phSamplers)
{
    CosUmdDevice * pDevice = CosUmdDevice::CastFrom(hDevice);
    pDevice->SetDomainSamplers(Offset, NumSamplers, phSamplers);
}

//
// Element Layout
//

SIZE_T APIENTRY CosUmdDeviceDdi::DdiCalcPrivateElementLayoutSize(
    D3D10DDI_HDEVICE hDevice,
    const D3D10DDIARG_CREATEELEMENTLAYOUT* pCreate)
{
    CosUmdDevice * pDevice = CosUmdDevice::CastFrom(hDevice);
    pDevice; // unused
    pCreate; // unused

    return sizeof(CosUmdElementLayout);
}

void APIENTRY CosUmdDeviceDdi::DdiCreateElementLayout(
    D3D10DDI_HDEVICE hDevice,
    const D3D10DDIARG_CREATEELEMENTLAYOUT* pCreate,
    D3D10DDI_HELEMENTLAYOUT hElementLayout,
    D3D10DDI_HRTELEMENTLAYOUT hRTElementLayout)
{
    CosUmdDevice * pDevice = CosUmdDevice::CastFrom(hDevice);
    pDevice; // unused

    CosUmdElementLayout* pElementLayout = new (hElementLayout.pDrvPrivate) CosUmdElementLayout(pCreate, hRTElementLayout);
    pElementLayout; // unused
}

void APIENTRY CosUmdDeviceDdi::DdiDestroyElementLayout(
    D3D10DDI_HDEVICE hDevice,
    D3D10DDI_HELEMENTLAYOUT hElementLayout)
{
    CosUmdDevice * pDevice = CosUmdDevice::CastFrom(hDevice);
    pDevice; // unused

    CosUmdElementLayout * pElementLayout = CosUmdElementLayout::CastFrom(hElementLayout);
    pElementLayout->~CosUmdElementLayout();

}

void APIENTRY CosUmdDeviceDdi::DdiPsSetConstantBuffers11_1(
    D3D10DDI_HDEVICE hDevice,
    UINT offset,
    UINT numBuffers,
    const D3D10DDI_HRESOURCE* phBuffers,
    const UINT* pFirstConstant,
    const UINT* pNumConstants)
{
    CosUmdDevice * pDevice = CosUmdDevice::CastFrom(hDevice);

    pDevice->PsSetConstantBuffers11_1(offset, numBuffers, phBuffers, pFirstConstant, pNumConstants);
}
void APIENTRY CosUmdDeviceDdi::DdiIaSetInputLayout(
    D3D10DDI_HDEVICE hDevice,
    D3D10DDI_HELEMENTLAYOUT hElementLayout)
{
    CosUmdDevice * pDevice = CosUmdDevice::CastFrom(hDevice);
    CosUmdElementLayout * pElementLayout = CosUmdElementLayout::CastFrom(hElementLayout);
    pDevice->SetElementLayout(pElementLayout);
}

//
// Shaders
//

SIZE_T APIENTRY CosUmdDeviceDdi::DdiCalcPrivateShaderSize(
    D3D10DDI_HDEVICE hDevice,
    const UINT* pCode,
    const D3D11_1DDIARG_STAGE_IO_SIGNATURES* pSignatures)
{
    CosUmdDevice * pDevice = CosUmdDevice::CastFrom(hDevice);
    pDevice; // unused
    pCode; // unused
    pSignatures; // unused

    return sizeof(CosUmdPipelineShader);
}

void APIENTRY CosUmdDeviceDdi::DdiCreatePixelShader(
    D3D10DDI_HDEVICE hDevice,
    const UINT* pCode,
    D3D10DDI_HSHADER hShader,
    D3D10DDI_HRTSHADER hRTShader,
    const D3D11_1DDIARG_STAGE_IO_SIGNATURES* pSignatures)
{
    CosUmdDevice * pCosUmdDevice = CosUmdDevice::CastFrom(hDevice);

    try
    {
        pCosUmdDevice->CreatePixelShader(pCode, hShader, hRTShader, pSignatures);
    }

    catch (std::exception & e)
    {
        pCosUmdDevice->SetException(e);
    }
}

void APIENTRY CosUmdDeviceDdi::DdiCreateVertexShader(
    D3D10DDI_HDEVICE hDevice,
    const UINT* pCode,
    D3D10DDI_HSHADER hShader,
    D3D10DDI_HRTSHADER hRTShader,
    const D3D11_1DDIARG_STAGE_IO_SIGNATURES* pSignatures)
{
    CosUmdDevice * pCosUmdDevice = CosUmdDevice::CastFrom(hDevice);

    try
    {
        pCosUmdDevice->CreateVertexShader(pCode, hShader, hRTShader, pSignatures);
    }

    catch (std::exception & e)
    {
        pCosUmdDevice->SetException(e);
    }
}

void APIENTRY CosUmdDeviceDdi::DdiCreateGeometryShader(
    D3D10DDI_HDEVICE hDevice,
    const UINT* pCode,
    D3D10DDI_HSHADER hShader,
    D3D10DDI_HRTSHADER hRTShader,
    const D3D11_1DDIARG_STAGE_IO_SIGNATURES* pSignatures)
{
    CosUmdDevice * pCosUmdDevice = CosUmdDevice::CastFrom(hDevice);

    try
    {
        pCosUmdDevice->CreateGeometryShader(pCode, hShader, hRTShader, pSignatures);
    }

    catch (std::exception & e)
    {
        pCosUmdDevice->SetException(e);
    }
}

void APIENTRY CosUmdDeviceDdi::DdiCreateComputeShader(
    D3D10DDI_HDEVICE hDevice,
    const UINT* pCode,
    D3D10DDI_HSHADER hShader,
    D3D10DDI_HRTSHADER hRTShader)
{
    CosUmdDevice * pCosUmdDevice = CosUmdDevice::CastFrom(hDevice);

    try
    {
        pCosUmdDevice->CreateComputeShader(pCode, hShader, hRTShader);
    }

    catch (std::exception & e)
    {
        pCosUmdDevice->SetException(e);
    }
}

SIZE_T APIENTRY CosUmdDeviceDdi::DdiCalcPrivateTessellationShaderSize(
    D3D10DDI_HDEVICE hDevice,
    const UINT* pCode,
    const D3D11_1DDIARG_TESSELLATION_IO_SIGNATURES* pSignatures)
{
    CosUmdDevice * pDevice = CosUmdDevice::CastFrom(hDevice);

    pDevice; // unused
    pCode; // unused
    pSignatures; // unused

    return sizeof(CosUmdTesselationShader);
}

void APIENTRY CosUmdDeviceDdi::DdiCreateHullShader(
    D3D10DDI_HDEVICE hDevice,
    const UINT* pCode,
    D3D10DDI_HSHADER hShader,
    D3D10DDI_HRTSHADER hRTShader,
    const D3D11_1DDIARG_TESSELLATION_IO_SIGNATURES* pSignatures)
{
    CosUmdDevice * pCosUmdDevice = CosUmdDevice::CastFrom(hDevice);

    try
    {
        pCosUmdDevice->CreateTessellationShader(pCode, hShader, hRTShader, pSignatures, D3D11_SB_HULL_SHADER);
    }

    catch (std::exception & e)
    {
        pCosUmdDevice->SetException(e);
    }
}

void APIENTRY CosUmdDeviceDdi::DdiCreateDomainShader(
    D3D10DDI_HDEVICE hDevice,
    const UINT* pCode,
    D3D10DDI_HSHADER hShader,
    D3D10DDI_HRTSHADER hRTShader,
    const D3D11_1DDIARG_TESSELLATION_IO_SIGNATURES* pSignatures)
{
    CosUmdDevice * pCosUmdDevice = CosUmdDevice::CastFrom(hDevice);

    try
    {
        pCosUmdDevice->CreateTessellationShader(pCode, hShader, hRTShader, pSignatures, D3D11_SB_DOMAIN_SHADER);
    }

    catch (std::exception & e)
    {
        pCosUmdDevice->SetException(e);
    }
}

void APIENTRY CosUmdDeviceDdi::DdiDestroyShader(
    D3D10DDI_HDEVICE hDevice,
    D3D10DDI_HSHADER hShader)
{
    CosUmdDevice * pDevice = CosUmdDevice::CastFrom(hDevice);

    pDevice->DestroyShader(hShader);
}

void APIENTRY CosUmdDeviceDdi::DdiPSSetShaderResources(
    D3D10DDI_HDEVICE hDevice,
    UINT offset,
    UINT numViews,
    const D3D10DDI_HSHADERRESOURCEVIEW* pShaderResourceViews)
{
    CosUmdDevice * pDevice = CosUmdDevice::CastFrom(hDevice);

    pDevice->PSSetShaderResources(offset, numViews, pShaderResourceViews);
}

void APIENTRY CosUmdDeviceDdi::DdiPsSetShader(D3D10DDI_HDEVICE hDevice, D3D10DDI_HSHADER hShader)
{
    CosUmdDevice * pDevice = CosUmdDevice::CastFrom(hDevice);
    CosUmdShader * pShader = CosUmdShader::CastFrom(hShader);
    pDevice->SetPixelShader(pShader);
}

void APIENTRY CosUmdDeviceDdi::DdiVsSetShader(D3D10DDI_HDEVICE hDevice, D3D10DDI_HSHADER hShader)
{
    CosUmdDevice * pDevice = CosUmdDevice::CastFrom(hDevice);
    CosUmdShader * pShader = CosUmdShader::CastFrom(hShader);
    pDevice->SetVertexShader(pShader);
}

void APIENTRY CosUmdDeviceDdi::DdiGsSetShader(D3D10DDI_HDEVICE hDevice, D3D10DDI_HSHADER hShader)
{
    CosUmdDevice * pDevice = CosUmdDevice::CastFrom(hDevice);
    CosUmdShader * pShader = CosUmdShader::CastFrom(hShader);
    pDevice->SetGeometryShader(pShader);
}

void APIENTRY CosUmdDeviceDdi::DdiHsSetShader(D3D10DDI_HDEVICE hDevice, D3D10DDI_HSHADER hShader)
{
    CosUmdDevice * pDevice = CosUmdDevice::CastFrom(hDevice);
    CosUmdShader * pShader = CosUmdShader::CastFrom(hShader);
    pDevice->SetHullShader(pShader);
}

void APIENTRY CosUmdDeviceDdi::DdiDsSetShader(D3D10DDI_HDEVICE hDevice, D3D10DDI_HSHADER hShader)
{
    CosUmdDevice * pDevice = CosUmdDevice::CastFrom(hDevice);
    CosUmdShader * pShader = CosUmdShader::CastFrom(hShader);
    pDevice->SetDomainShader(pShader);
}

void APIENTRY CosUmdDeviceDdi::DdiCsSetShader(D3D10DDI_HDEVICE hDevice, D3D10DDI_HSHADER hShader)
{
    CosUmdDevice * pDevice = CosUmdDevice::CastFrom(hDevice);
    CosUmdShader * pShader = CosUmdShader::CastFrom(hShader);

    pDevice->SetComputeShader(pShader);
}

//
// Blend State
//

SIZE_T APIENTRY CosUmdDeviceDdi::DdiCalcPrivateBlendStateSize(
    D3D10DDI_HDEVICE hDevice,
    const D3D11_1_DDI_BLEND_DESC* desc)
{
    CosUmdDevice * pDevice = CosUmdDevice::CastFrom(hDevice);
    pDevice; // unused
    desc; // unused

    return sizeof(CosUmdBlendState);
}

void APIENTRY CosUmdDeviceDdi::DdiCreateBlendState(
    D3D10DDI_HDEVICE hDevice,
    const D3D11_1_DDI_BLEND_DESC* desc,
    D3D10DDI_HBLENDSTATE hBlendState,
    D3D10DDI_HRTBLENDSTATE hRTBlendState)
{
    CosUmdDevice * pDevice = CosUmdDevice::CastFrom(hDevice);
    pDevice; // unused

    CosUmdBlendState* pBlendState = new (hBlendState.pDrvPrivate) CosUmdBlendState(desc, hRTBlendState);
    pBlendState; // unused

}

void APIENTRY CosUmdDeviceDdi::DdiDestroyBlendState(
    D3D10DDI_HDEVICE hDevice,
    D3D10DDI_HBLENDSTATE hBlendState)
{
    CosUmdDevice * pDevice = CosUmdDevice::CastFrom(hDevice);
    pDevice; // unused

    CosUmdBlendState * pBlendState = CosUmdBlendState::CastFrom(hBlendState);
    pBlendState; // unused
}

void APIENTRY CosUmdDeviceDdi::DdiSetBlendState(
    D3D10DDI_HDEVICE hDevice,
    D3D10DDI_HBLENDSTATE hBlendState,
    const FLOAT pBlendFactor[4],
    UINT sampleMask)
{
    CosUmdDevice * pDevice = CosUmdDevice::CastFrom(hDevice);
    CosUmdBlendState * pBlendState = CosUmdBlendState::CastFrom(hBlendState);
    pDevice->SetBlendState(pBlendState, pBlendFactor, sampleMask);
}

//
// Render Target View
//

SIZE_T APIENTRY CosUmdDeviceDdi::DdiCalcPrivateRenderTargetViewSize(
    D3D10DDI_HDEVICE hDevice,
    const D3D10DDIARG_CREATERENDERTARGETVIEW* pCreate)
{
    CosUmdDevice * pDevice = CosUmdDevice::CastFrom(hDevice);
    pDevice; // unused
    pCreate; // unused

    return sizeof(CosUmdRenderTargetView);
}

void APIENTRY CosUmdDeviceDdi::DdiCreateRenderTargetView(
    D3D10DDI_HDEVICE hDevice,
    const D3D10DDIARG_CREATERENDERTARGETVIEW* pCreate,
    D3D10DDI_HRENDERTARGETVIEW hRenderTargetView,
    D3D10DDI_HRTRENDERTARGETVIEW hRTRenderTargetView)
{
    CosUmdDevice * pDevice = CosUmdDevice::CastFrom(hDevice);
    pDevice; // unused

    CosUmdRenderTargetView * pRenderTargetView = new(hRenderTargetView.pDrvPrivate) CosUmdRenderTargetView(pCreate, hRTRenderTargetView);
    pRenderTargetView; // unused

}

void APIENTRY CosUmdDeviceDdi::DdiDestroyRenderTargetView(
    D3D10DDI_HDEVICE hDevice,
    D3D10DDI_HRENDERTARGETVIEW hRenderTargetView)
{
    CosUmdDevice * pDevice = CosUmdDevice::CastFrom(hDevice);
    pDevice; // unused

    CosUmdRenderTargetView * pRenderTargetView = CosUmdRenderTargetView::CastFrom(hRenderTargetView);
    pRenderTargetView->~CosUmdRenderTargetView();

}

SIZE_T APIENTRY CosUmdDeviceDdi::DdiCalcPrivateDepthStencilViewSize11(
    D3D10DDI_HDEVICE,
    const D3D11DDIARG_CREATEDEPTHSTENCILVIEW*)
{
    return sizeof(CosUmdDepthStencilView);
}

void APIENTRY CosUmdDeviceDdi::DdiCreateDepthStencilView11(
    D3D10DDI_HDEVICE,
    const D3D11DDIARG_CREATEDEPTHSTENCILVIEW* pCreate,
    D3D10DDI_HDEPTHSTENCILVIEW hDepthStencilView,
    D3D10DDI_HRTDEPTHSTENCILVIEW hRTDepthStencilView)
{
    CosUmdDepthStencilView * pDepthStencilView = new(hDepthStencilView.pDrvPrivate) CosUmdDepthStencilView(pCreate, hRTDepthStencilView);
    pDepthStencilView;  // unused
}

void APIENTRY CosUmdDeviceDdi::DdiDestroyDepthStencilView(
    D3D10DDI_HDEVICE,
    D3D10DDI_HDEPTHSTENCILVIEW hDepthStencilView)
{
    CosUmdDepthStencilView * pDepthStencilView = CosUmdDepthStencilView::CastFrom(hDepthStencilView);
    pDepthStencilView->~CosUmdDepthStencilView();
}

void APIENTRY CosUmdDeviceDdi::DdiClearRenderTargetView(
    D3D10DDI_HDEVICE hDevice,
    D3D10DDI_HRENDERTARGETVIEW hRenderTargetView,
    FLOAT clearColor[4])
{
    CosUmdDevice * pDevice = CosUmdDevice::CastFrom(hDevice);
    CosUmdRenderTargetView * pRenderTargetView = CosUmdRenderTargetView::CastFrom(hRenderTargetView);
    pDevice->ClearRenderTargetView(pRenderTargetView, clearColor);
}

void APIENTRY CosUmdDeviceDdi::DdiClearDepthStencilView(
    D3D10DDI_HDEVICE hDevice,
    D3D10DDI_HDEPTHSTENCILVIEW hDepthStencilView,
    UINT clearFlags,
    FLOAT depthValue,
    UINT8 stencilValue)
{
    CosUmdDevice * pDevice = CosUmdDevice::CastFrom(hDevice);
    CosUmdDepthStencilView * pDepthStencilView = CosUmdDepthStencilView::CastFrom(hDepthStencilView);

    pDevice->ClearDepthStencilView(pDepthStencilView, clearFlags, depthValue, stencilValue);
}

void APIENTRY CosUmdDeviceDdi::DdiSetRenderTargets(
    D3D10DDI_HDEVICE hDevice,
    const D3D10DDI_HRENDERTARGETVIEW* phRenderTargetView,
    UINT NumRTVs,
    UINT RTVNumbertoUnbind,
    D3D10DDI_HDEPTHSTENCILVIEW hDepthStencilView,
    const D3D11DDI_HUNORDEREDACCESSVIEW* phUnorderedAccessView,
    const UINT* pUAVInitialCounts,
    UINT UAVIndex,
    UINT NumUAVs,
    UINT UAVFirsttoSet,
    UINT UAVNumberUpdated)
{
    CosUmdDevice * pDevice = CosUmdDevice::CastFrom(hDevice);

    pDevice->SetRenderTargets(phRenderTargetView, NumRTVs, RTVNumbertoUnbind, hDepthStencilView, phUnorderedAccessView, pUAVInitialCounts, UAVIndex,
        NumUAVs, UAVFirsttoSet, UAVNumberUpdated);
}

//
// View port
//

void APIENTRY CosUmdDeviceDdi::DdiSetViewports(
    D3D10DDI_HDEVICE hDevice,
    UINT numViewports,
    UINT clearViewports,
    const D3D10_DDI_VIEWPORT* pViewports)
{
    CosUmdDevice * pDevice = CosUmdDevice::CastFrom(hDevice);
    pDevice->SetViewports(numViewports, clearViewports, pViewports);
}

//
// Topology
//

void APIENTRY CosUmdDeviceDdi::DdiIaSetTopology(
    D3D10DDI_HDEVICE hDevice,
    D3D10_DDI_PRIMITIVE_TOPOLOGY topology)
{
    CosUmdDevice * pDevice = CosUmdDevice::CastFrom(hDevice);
    pDevice->SetTopology(topology);
}

//
// Draw
//

void APIENTRY CosUmdDeviceDdi::DdiDraw(
    D3D10DDI_HDEVICE hDevice,
    UINT vertexCount,
    UINT startVertexLocation)
{
    CosUmdDevice * pDevice = CosUmdDevice::CastFrom(hDevice);
    pDevice->Draw(vertexCount, startVertexLocation);
}

void APIENTRY CosUmdDeviceDdi::DdiDrawIndexed(
    D3D10DDI_HDEVICE hDevice,
    UINT indexCount,
    UINT startIndexLocation,
    INT baseVertexLocation)
{
    CosUmdDevice * pDevice = CosUmdDevice::CastFrom(hDevice);
    pDevice->DrawIndexed(indexCount, startIndexLocation, baseVertexLocation);
}

//
// Vertex Buffers
//

void APIENTRY CosUmdDeviceDdi::DdiIaSetVertexBuffers(
    D3D10DDI_HDEVICE hDevice,
    UINT startBuffer,
    UINT numBuffers,
    const D3D10DDI_HRESOURCE* phBuffers,
    const UINT* pStrides,
    const UINT* pOffsets)
{
    CosUmdDevice * pDevice = CosUmdDevice::CastFrom(hDevice);
    pDevice->SetVertexBuffers(startBuffer, numBuffers, phBuffers, pStrides, pOffsets);
}

//
// Index Buffer
//

void APIENTRY CosUmdDeviceDdi::DdiIaSetIndexBuffer(
    D3D10DDI_HDEVICE hDevice,
    D3D10DDI_HRESOURCE hIndexBuffer,
    DXGI_FORMAT hIndexFormat,
    UINT offset)
{
    CosUmdDevice * pDevice = CosUmdDevice::CastFrom(hDevice);
    pDevice->SetIndexBuffer(hIndexBuffer, hIndexFormat, offset);
}

void APIENTRY CosUmdDeviceDdi::DdiDynamicIABufferMapNoOverwrite(
    D3D10DDI_HDEVICE hDevice,
    D3D10DDI_HRESOURCE hResource,
    UINT subResource,
    D3D10_DDI_MAP mapType,
    UINT mapFlags,
    D3D10DDI_MAPPED_SUBRESOURCE* pMappedSubRes)
{
    CosUmdDevice* pCosUmdDevice = CosUmdDevice::CastFrom(hDevice);
    CosUmdResource * pResource = (CosUmdResource *)hResource.pDrvPrivate;

    try
    {
        pCosUmdDevice->ResourceMap(pResource, subResource, mapType, mapFlags, pMappedSubRes);
    }

    catch (std::exception & e)
    {
        pCosUmdDevice->SetException(e);
    }
}

void APIENTRY CosUmdDeviceDdi::DdiDynamicIABufferMapDiscard(
    D3D10DDI_HDEVICE hDevice,
    D3D10DDI_HRESOURCE hResource,
    UINT subResource,
    D3D10_DDI_MAP mapType,
    UINT mapFlags,
    D3D10DDI_MAPPED_SUBRESOURCE* pMappedSubRes)
{
    CosUmdDevice* pCosUmdDevice = CosUmdDevice::CastFrom(hDevice);
    CosUmdResource * pResource = (CosUmdResource *)hResource.pDrvPrivate;

    try
    {
        pCosUmdDevice->ResourceMap(pResource, subResource, mapType, mapFlags, pMappedSubRes);
    }

    catch (std::exception & e)
    {
        pCosUmdDevice->SetException(e);
    }
}

void APIENTRY CosUmdDeviceDdi::DdiDynamicIABufferUnmap(
    D3D10DDI_HDEVICE hDevice,
    D3D10DDI_HRESOURCE hResource,
    UINT subResource)
{
    CosUmdDevice* pCosUmdDevice = CosUmdDevice::CastFrom(hDevice);
    CosUmdResource * pResource = (CosUmdResource *)hResource.pDrvPrivate;

    try
    {
        pCosUmdDevice->ResourceUnmap(pResource, subResource);
    }

    catch (std::exception & e)
    {
        pCosUmdDevice->SetException(e);
    }
}

void APIENTRY CosUmdDeviceDdi::DdiVsSetConstantBuffers11_1(
    D3D10DDI_HDEVICE hDevice,
    UINT startBuffer,
    UINT numberBuffers,
    const D3D10DDI_HRESOURCE *  phResources,
    const UINT *    pFirstConstant,
    const UINT *    pNumberConstants)
{
    CosUmdDevice * pDevice = CosUmdDevice::CastFrom(hDevice);

    pDevice->VsSetConstantBuffers11_1(startBuffer, numberBuffers, phResources, pFirstConstant, pNumberConstants);
}

void APIENTRY CosUmdDeviceDdi::DdiDynamicConstantBufferMapDiscard(
    D3D10DDI_HDEVICE hDevice,
    D3D10DDI_HRESOURCE hResource,
    UINT subResource,
    D3D10_DDI_MAP mapType,
    UINT mapFlags,
    D3D10DDI_MAPPED_SUBRESOURCE* pMappedSubRes)
{
    CosUmdDevice* pCosUmdDevice = CosUmdDevice::CastFrom(hDevice);
    CosUmdResource * pResource = (CosUmdResource *)hResource.pDrvPrivate;

    try
    {
        pCosUmdDevice->ResourceMap(pResource, subResource, mapType, mapFlags, pMappedSubRes);
    }

    catch (std::exception & e)
    {
        pCosUmdDevice->SetException(e);
    }
}

void APIENTRY CosUmdDeviceDdi::DdiDynamicConstantBufferUnmap(
    D3D10DDI_HDEVICE hDevice,
    D3D10DDI_HRESOURCE hResource,
    UINT subResource)
{
    CosUmdDevice* pCosUmdDevice = CosUmdDevice::CastFrom(hDevice);
    CosUmdResource * pResource = (CosUmdResource *)hResource.pDrvPrivate;

    try
    {
        pCosUmdDevice->ResourceUnmap(pResource, subResource);
    }

    catch (std::exception & e)
    {
        pCosUmdDevice->SetException(e);
    }
}

HRESULT CosUmdDeviceDdi::Present(DXGI_DDI_ARG_PRESENT* pPresentData)
{
    CosUmdDevice* pCosUmdDevice = CosUmdDevice::CastFrom(pPresentData->hDevice);
    HRESULT hr = pCosUmdDevice->Present(pPresentData);
    if (FAILED(hr))
    {
        pCosUmdDevice->SetError(hr);
    }

    return hr;
}

HRESULT CosUmdDeviceDdi::RotateResourceIdentities(
    DXGI_DDI_ARG_ROTATE_RESOURCE_IDENTITIES* Args)
{
    CosUmdDevice* pCosUmdDevice = CosUmdDevice::CastFrom(Args->hDevice);
    HRESULT hr = pCosUmdDevice->RotateResourceIdentities(Args);
    if (FAILED(hr))
    {
        pCosUmdDevice->SetError(hr);
    }
    
    return hr;
}

HRESULT CosUmdDeviceDdi::SetDisplayMode(
    DXGI_DDI_ARG_SETDISPLAYMODE* pDisplayModeData)
{
    CosUmdDevice* pCosUmdDevice = CosUmdDevice::CastFrom(pDisplayModeData->hDevice);
    HRESULT hr = pCosUmdDevice->SetDisplayMode(pDisplayModeData);
    if (FAILED(hr))
    {
        pCosUmdDevice->SetError(hr);
    }

    return hr;
}

HRESULT CosUmdDeviceDdi::Present1(DXGI_DDI_ARG_PRESENT1* pPresentData)
{
    CosUmdDevice* pCosUmdDevice = CosUmdDevice::CastFrom(pPresentData->hDevice);
    HRESULT hr = pCosUmdDevice->Present1(pPresentData);
    if (FAILED(hr))
    {
        pCosUmdDevice->SetError(hr);
    }

    return hr;
}

_Use_decl_annotations_
void CosUmdDeviceDdi::CheckDirectFlipSupport(
    D3D10DDI_HDEVICE hDevice,
    D3D10DDI_HRESOURCE hResource1,
    D3D10DDI_HRESOURCE hResource2,
    UINT CheckDirectFlipFlags,
    BOOL *pSupported
    )
{
    CosUmdDevice* pCosUmdDevice = CosUmdDevice::CastFrom(hDevice);
    pCosUmdDevice->CheckDirectFlipSupport(
        hDevice,
        hResource1,
        hResource2,
        CheckDirectFlipFlags,
        pSupported);
}

void APIENTRY CosUmdDeviceDdi::DdiSetPredication(
    D3D10DDI_HDEVICE hDevice,
    D3D10DDI_HQUERY hQuery,
    BOOL bPredicateValue
    )
{
    CosUmdDevice* pCosUmdDevice = CosUmdDevice::CastFrom(hDevice);
    
    try
    {
        pCosUmdDevice->SetPredication(hQuery, bPredicateValue);
    }
    catch (std::exception & e)
    {
        pCosUmdDevice->SetException(e);
    }
}

void APIENTRY CosUmdDeviceDdi::DdiResourceCopyRegion11_1(
    D3D10DDI_HDEVICE hDevice,
    D3D10DDI_HRESOURCE hDstResource,
    UINT DstSubresource,
    UINT DstX,
    UINT DstY,
    UINT DstZ,
    D3D10DDI_HRESOURCE hSrcResource,
    UINT SrcSubresource,
    const D3D10_DDI_BOX* pSrcBox,
    UINT copyFlags
    )
{
    CosUmdDevice* pCosUmdDevice = CosUmdDevice::CastFrom(hDevice);
    CosUmdResource * pDestinationResource = (CosUmdResource *) hDstResource.pDrvPrivate;
    CosUmdResource * pSourceResource = (CosUmdResource *) hSrcResource.pDrvPrivate;

    try
    {
        pCosUmdDevice->ResourceCopyRegion11_1(pDestinationResource, DstSubresource, DstX, DstY, DstZ, pSourceResource, SrcSubresource, pSrcBox, copyFlags);
    }

    catch (std::exception & e)
    {
        pCosUmdDevice->SetException(e);
    }
}

void APIENTRY CosUmdDeviceDdi::DdiResourceCopyRegion(
    D3D10DDI_HDEVICE hDevice,
    D3D10DDI_HRESOURCE hDstResource,
    UINT DstSubresource,
    UINT DstX,
    UINT DstY,
    UINT DstZ,
    D3D10DDI_HRESOURCE hSrcResource,
    UINT SrcSubresource,
    const D3D10_DDI_BOX* pSrcBox
    )
{
    DdiResourceCopyRegion11_1(hDevice, hDstResource, DstSubresource, DstX, DstY, DstZ, hSrcResource, SrcSubresource, pSrcBox, 0);
}

//
// Unordered Acciew View
//

SIZE_T APIENTRY CosUmdDeviceDdi::DdiCalcPrivateUnorderedAccessViewSize(D3D10DDI_HDEVICE, const D3D11DDIARG_CREATEUNORDEREDACCESSVIEW*)
{
    return sizeof(CosUmdUnorderedAccessView);
}

void APIENTRY CosUmdDeviceDdi::DdiCreateUnorderedAccessView(
    D3D10DDI_HDEVICE hDevice,
    const D3D11DDIARG_CREATEUNORDEREDACCESSVIEW* pCreate,
    D3D11DDI_HUNORDEREDACCESSVIEW hUnorderedAccessView,
    D3D11DDI_HRTUNORDEREDACCESSVIEW hRTUnorderedAccessView)
{
    CosUmdDevice * pDevice = CosUmdDevice::CastFrom(hDevice);
    pDevice;

    CosUmdUnorderedAccessView * pUnorderedAccessView = new(hUnorderedAccessView.pDrvPrivate) CosUmdUnorderedAccessView(pCreate, hRTUnorderedAccessView);
    pUnorderedAccessView;
}

void APIENTRY CosUmdDeviceDdi::DdiDestroyUnorderedAccessView(
    D3D10DDI_HDEVICE hDevice,
    D3D11DDI_HUNORDEREDACCESSVIEW hUnorderedAccessView) 
{
    CosUmdDevice * pDevice = CosUmdDevice::CastFrom(hDevice);
    pDevice; // unused

    CosUmdUnorderedAccessView * pUnorderedAccessView = CosUmdUnorderedAccessView::CastFrom(hUnorderedAccessView);
    pUnorderedAccessView->~CosUmdUnorderedAccessView();
}

void APIENTRY CosUmdDeviceDdi::CSSetShaderResources(
    D3D10DDI_HDEVICE hDevice,
    UINT offset,
    UINT numViews, 
    const D3D10DDI_HSHADERRESOURCEVIEW* pShaderResourceViews)
{
    CosUmdDevice * pDevice = CosUmdDevice::CastFrom(hDevice);

    pDevice->CSSetShaderResources(offset, numViews, pShaderResourceViews);
}

void APIENTRY CosUmdDeviceDdi::CSSetUnorderedAccessViews(
    D3D10DDI_HDEVICE hDevice,
    UINT offset,
    UINT numViews,
    const D3D11DDI_HUNORDEREDACCESSVIEW* pUnorderedAccessViews,
    const UINT* pUnorderedAccessViewsInitialCounts)
{
    CosUmdDevice * pDevice = CosUmdDevice::CastFrom(hDevice);

    pDevice->CSSetUnorderedAccessViews(offset, numViews, pUnorderedAccessViews, pUnorderedAccessViewsInitialCounts);
}

void APIENTRY CosUmdDeviceDdi::ClearUnorderedAccessViewUint(
    D3D10DDI_HDEVICE hDevice, 
    D3D11DDI_HUNORDEREDACCESSVIEW hUnorderdAccessView,
    const UINT clearColor[4])
{
    CosUmdDevice * pDevice = CosUmdDevice::CastFrom(hDevice);
    CosUmdUnorderedAccessView * pUnorderedAccessView = CosUmdUnorderedAccessView::CastFrom(hUnorderdAccessView);
    pDevice->ClearUnorderedAccessView(pUnorderedAccessView, clearColor);
}

void APIENTRY CosUmdDeviceDdi::ClearUnorderedAccessViewFloat(
    D3D10DDI_HDEVICE hDevice, 
    D3D11DDI_HUNORDEREDACCESSVIEW hUnorderdAccessView,
    const FLOAT clearColor[4])
{
    CosUmdDevice * pDevice = CosUmdDevice::CastFrom(hDevice);
    CosUmdUnorderedAccessView * pUnorderedAccessView = CosUmdUnorderedAccessView::CastFrom(hUnorderdAccessView);
    pDevice->ClearUnorderedAccessView(pUnorderedAccessView, clearColor);
}

void APIENTRY CosUmdDeviceDdi::Dispatch(D3D10DDI_HDEVICE hDevice, UINT threadGroupCountX, UINT threadGroupCountY, UINT threadGroupCountZ)
{
    CosUmdDevice * pDevice = CosUmdDevice::CastFrom(hDevice);

    pDevice->Dispatch(threadGroupCountX, threadGroupCountY, threadGroupCountZ);
}
