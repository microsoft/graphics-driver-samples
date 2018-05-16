#include "precomp.h"

#include "RosUmdLogging.h"
#include "RosUmdDeviceDdi.tmh"

#include "RosUmdDeviceDdi.h"
#include "RosUmdDevice.h"
#include "RosUmdResource.h"
#include "RosUmdDebug.h"
#include "RosUmdRasterizerState.h"
#include "RosUmdDepthStencilState.h"
#include "RosUmdSampler.h"
#include "RosUmdElementLayout.h"
#include "RosUmdShader.h"
#include "RosUmdBlendState.h"
#include "RosUmdRenderTargetView.h"
#include "RosUmdDepthStencilView.h"
#include "RosUmdShaderResourceView.h"

#include "RosContext.h"

//
// Ddi Tables
//

const D3DWDDM1_3DDI_DEVICEFUNCS RosUmdDeviceDdi::s_deviceFuncsWDDM1_3 =
{
    RosUmdDeviceDdi::DdiConstantBufferUpdateSubresourceUP11_1,
    RosUmdDeviceDdi::DdiVsSetConstantBuffers11_1,
    RosUmdDeviceDdi::DdiPSSetShaderResources,
    RosUmdDeviceDdi::DdiPsSetShader,
    RosUmdDeviceDdi::DdiPSSetSamplers,
    RosUmdDeviceDdi::DdiVsSetShader,
    RosUmdDeviceDdi::DdiDrawIndexed,
    RosUmdDeviceDdi::DdiDraw,
    RosUmdDeviceDdi::DdiDynamicIABufferMapNoOverwrite,
    RosUmdDeviceDdi::DdiDynamicIABufferUnmap,
    RosUmdDeviceDdi::DdiDynamicConstantBufferMapDiscard,
    RosUmdDeviceDdi::DdiDynamicIABufferMapDiscard,
    RosUmdDeviceDdi::DdiDynamicConstantBufferUnmap,
    RosUmdDeviceDdi::DdiPsSetConstantBuffers11_1,
    RosUmdDeviceDdi::DdiIaSetInputLayout,
    RosUmdDeviceDdi::DdiIaSetVertexBuffers,
    RosUmdDeviceDdi::DdiIaSetIndexBuffer,

    RosUmdDeviceDdi::DrawIndexedInstanced_Dirty,
    RosUmdDeviceDdi::DrawInstanced_Dirty,
    RosUmdDeviceDdi::DynamicResourceMapDiscard_Default,
    RosUmdDeviceDdi::DynamicResourceUnmap_Default,
    RosUmdDeviceDdi::GsSetConstantBuffers11_1_Default,
    RosUmdDeviceDdi::DdiGsSetShader,
    RosUmdDeviceDdi::DdiIaSetTopology,
    RosUmdDeviceDdi::DdiStagingResourceMap,
    RosUmdDeviceDdi::DdiStagingResourceUnmap,
    RosUmdDeviceDdi::VSSetShaderResources_Default,
    RosUmdDeviceDdi::DdiVSSetSamplers,
    RosUmdDeviceDdi::GSSetShaderResources_Default,
    RosUmdDeviceDdi::DdiGSSetSamplers,
    RosUmdDeviceDdi::DdiSetRenderTargets,
    RosUmdDeviceDdi::ShaderResourceViewReadAfterWriteHazard_Default,
    RosUmdDeviceDdi::ResourceReadAfterWriteHazard_Default,
    RosUmdDeviceDdi::DdiSetBlendState,
    RosUmdDeviceDdi::DdiSetDepthStencilState,
    RosUmdDeviceDdi::DdiSetRasterizerState,
    RosUmdDeviceDdi::QueryEnd_Default,
    RosUmdDeviceDdi::QueryBegin_Default,
    RosUmdDeviceDdi::DdiResourceCopyRegion11_1,
    RosUmdDeviceDdi::ResourceUpdateSubresourceUP11_1_Default,
    RosUmdDeviceDdi::SOSetTargets_Default,
    RosUmdDeviceDdi::DrawAuto_Dirty,
    RosUmdDeviceDdi::DdiSetViewports,
    RosUmdDeviceDdi::DdiSetScissorRects,
    RosUmdDeviceDdi::DdiClearRenderTargetView,
    RosUmdDeviceDdi::DdiClearDepthStencilView,
    RosUmdDeviceDdi::DdiSetPredication,
    RosUmdDeviceDdi::QueryGetData_Default,
    RosUmdDeviceDdi::DdiFlush,
    RosUmdDeviceDdi::GenerateMips_Default,
    RosUmdDeviceDdi::DdiResourceCopy,
    RosUmdDeviceDdi::ResourceResolveSubresource_Default,

    RosUmdDeviceDdi::ResourceMap_Default,
    RosUmdDeviceDdi::ResourceUnmap_Default,
    RosUmdDeviceDdi::ResourceIsStagingBusy_Default,
    RosUmdDeviceDdi::RelocateDeviceFuncs11_1_Default,
    RosUmdDeviceDdi::DdiCalcPrivateResourceSize,
    RosUmdDeviceDdi::DdiCalcPrivateOpenedResourceSize,
    RosUmdDeviceDdi::DdiCreateResource,
    RosUmdDeviceDdi::DdiOpenResource,
    RosUmdDeviceDdi::DdiDestroyResource,
    RosUmdDeviceDdi::DdiCalcPrivateShaderResourceViewSize11,
    RosUmdDeviceDdi::DdiCreateShaderResourceView11,
    RosUmdDeviceDdi::DdiDestroyShaderResourceView,
    RosUmdDeviceDdi::DdiCalcPrivateRenderTargetViewSize,
    RosUmdDeviceDdi::DdiCreateRenderTargetView,
    RosUmdDeviceDdi::DdiDestroyRenderTargetView,
    RosUmdDeviceDdi::DdiCalcPrivateDepthStencilViewSize11,
    RosUmdDeviceDdi::DdiCreateDepthStencilView11,
    RosUmdDeviceDdi::DdiDestroyDepthStencilView,
    RosUmdDeviceDdi::DdiCalcPrivateElementLayoutSize,
    RosUmdDeviceDdi::DdiCreateElementLayout,
    RosUmdDeviceDdi::DdiDestroyElementLayout,
    RosUmdDeviceDdi::DdiCalcPrivateBlendStateSize,
    RosUmdDeviceDdi::DdiCreateBlendState,
    RosUmdDeviceDdi::DdiDestroyBlendState,
    RosUmdDeviceDdi::DdiCalcPrivateDepthStencilStateSize,
    RosUmdDeviceDdi::DdiCreateDepthStencilState,
    RosUmdDeviceDdi::DdiDestroyDepthStencilState,
    RosUmdDeviceDdi::DdiCalcRasterizerStateSize,
    RosUmdDeviceDdi::DdiCreateRasterizerState,
    RosUmdDeviceDdi::DdiDestroyRasterizerState,
    RosUmdDeviceDdi::DdiCalcPrivateShaderSize,
    RosUmdDeviceDdi::DdiCreateVertexShader,
    RosUmdDeviceDdi::DdiCreateGeometryShader,
    RosUmdDeviceDdi::DdiCreatePixelShader,
    RosUmdDeviceDdi::CalcPrivateGeometryShaderWithStreamOutputSize11_1_Default,
    RosUmdDeviceDdi::CreateGeometryShaderWithStreamOutput11_1_Default,
    RosUmdDeviceDdi::DdiDestroyShader,
    RosUmdDeviceDdi::DdiCalcPrivateSamplerSize,
    RosUmdDeviceDdi::DdiCreateSampler,
    RosUmdDeviceDdi::DdiDestroySampler,
    RosUmdDeviceDdi::CalcPrivateQuerySize_Default,
    RosUmdDeviceDdi::CreateQuery_Default,
    RosUmdDeviceDdi::DestroyQuery_Default,

    RosUmdDeviceDdi::DdiCheckFormatSupport,
    RosUmdDeviceDdi::DdiCheckMultisampleQualityLevels,
    RosUmdDeviceDdi::DdiCheckCounterInfo,
    RosUmdDeviceDdi::CheckCounter_Default,
    RosUmdDeviceDdi::DdiDestroyDevice,
    RosUmdDeviceDdi::SetTextFilter_Default,
    RosUmdDeviceDdi::DdiResourceCopy,
    RosUmdDeviceDdi::DdiResourceCopyRegion11_1,

    RosUmdDeviceDdi::DrawIndexedInstancedIndirect_Dirty,
    RosUmdDeviceDdi::DrawInstancedIndirect_Dirty,
    NULL, // RosUmdDeviceDdi::CommandListExecute_Default,
    RosUmdDeviceDdi::HSSetShaderResources_Default,
    RosUmdDeviceDdi::DdiHsSetShader,
    RosUmdDeviceDdi::DdiHSSetSamplers,
    RosUmdDeviceDdi::HsSetConstantBuffers11_1_Default,
    RosUmdDeviceDdi::DSSetShaderResources_Default,
    RosUmdDeviceDdi::DdiDsSetShader,
    RosUmdDeviceDdi::DdiDSSetSamplers,
    RosUmdDeviceDdi::DsSetConstantBuffers11_1_Default,
    RosUmdDeviceDdi::DdiCreateHullShader,
    RosUmdDeviceDdi::DdiCreateDomainShader,
    NULL, // RosUmdDeviceDdi::CheckDeferredContextHandleSizes_Default,
    NULL, // RosUmdDeviceDdi::CalcDeferredContextHandleSize_Default,
    NULL, // RosUmdDeviceDdi::CalcPrivateDeferredContextSize_Default,
    NULL, // RosUmdDeviceDdi::CreateDeferredContext_Default,
    NULL, // RosUmdDeviceDdi::AbandonCommandList_Default,
    NULL, // RosUmdDeviceDdi::CalcPrivateCommandListSize_Default,
    NULL, // RosUmdDeviceDdi::CreateCommandList_Default,
    NULL, // RosUmdDeviceDdi::DestroyCommandList_Default,
    RosUmdDeviceDdi::DdiCalcPrivateTessellationShaderSize,
    RosUmdDeviceDdi::PsSetShaderWithInterfaces_Default,
    RosUmdDeviceDdi::VsSetShaderWithInterfaces_Default,
    RosUmdDeviceDdi::GsSetShaderWithInterfaces_Default,
    RosUmdDeviceDdi::HsSetShaderWithInterfaces_Default,
    RosUmdDeviceDdi::DsSetShaderWithInterfaces_Default,
    RosUmdDeviceDdi::CsSetShaderWithInterfaces_Default,
    RosUmdDeviceDdi::DdiCreateComputeShader,
    RosUmdDeviceDdi::DdiCsSetShader,
    RosUmdDeviceDdi::CSSetShaderResources_Default,
    RosUmdDeviceDdi::DdiCSSetSamplers,
    RosUmdDeviceDdi::CsSetConstantBuffers11_1_Default,
    RosUmdDeviceDdi::CalcPrivateUnorderedAccessViewSize_Default,
    RosUmdDeviceDdi::CreateUnorderedAccessView_Default,
    RosUmdDeviceDdi::DestroyUnorderedAccessView_Default,
    RosUmdDeviceDdi::ClearUnorderedAccessViewUint_Default,
    RosUmdDeviceDdi::ClearUnorderedAccessViewFloat_Default,
    RosUmdDeviceDdi::CSSetUnorderedAccessViews_Default,
    RosUmdDeviceDdi::Dispatch_Dirty,
    RosUmdDeviceDdi::DispatchIndirect_Dirty,
    RosUmdDeviceDdi::SetResourceMinLOD_Default,
    RosUmdDeviceDdi::CopyStructureCount_Default,
    NULL, // RosUmdDeviceDdi::RecycleCommandList_Default,
    NULL, // RosUmdDeviceDdi::RecycleCreateCommandList_Default,
    NULL, // RosUmdDeviceDdi::RecycleCreateDeferredContext_Default,
    NULL, // RosUmdDeviceDdi::RecycleDestroyCommandList_Default,
    RosUmdDeviceDdi::Discard_Default,
    RosUmdDeviceDdi::AssignDebugBinary_Default,
    RosUmdDeviceDdi::DynamicConstantBufferMapNoOverwrite_Default,
    RosUmdDeviceDdi::CheckDirectFlipSupport,
    RosUmdDeviceDdi::ClearView_Default,
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

const DXGI1_3_DDI_BASE_FUNCTIONS RosUmdDeviceDdi::s_dxgiDeviceFuncs4 =
{
    RosUmdDeviceDdi::Present,
    NULL, // RosUmdDeviceDdi::GetGammaCaps,     //GetGammaCaps
    RosUmdDeviceDdi::SetDisplayMode,
    RosUmdDeviceDdi::SetResourcePriority,
    NULL, // RosUmdDeviceDdi::QueryResourceResidency,
    RosUmdDeviceDdi::RotateResourceIdentities,
    NULL, // RosUmdDeviceDdi::Blt,
    RosUmdDeviceDdi::ResolveSharedResource,
    NULL, // RosUmdDeviceDdi::Blt1,
    NULL, // pfnOfferResources
    NULL, // pfnReclaimResources
    NULL, // pfnGetMultiplaneOverlayCaps
    NULL, // pfnGetMultiplaneOverlayGroupCaps
    NULL, // pfnReserved1
    NULL, // pfnPresentMultiplaneOverlay
    NULL, // pfnReserved2
    RosUmdDeviceDdi::Present1, // pfnPresent1
    NULL, // pfnCheckPresentDurationSupport
};

//
// DDI entry points
//

void APIENTRY RosUmdDeviceDdi::DdiDestroyDevice(
    D3D10DDI_HDEVICE hDevice)
{
    RosUmdDevice* pRosUmdDevice = RosUmdDevice::CastFrom(hDevice);

    try {
        pRosUmdDevice->Teardown();
        pRosUmdDevice->~RosUmdDevice();
    }

    catch (std::exception &)
    {
        // do nothing
    }
}

void APIENTRY RosUmdDeviceDdi::DdiCreateResource(
    D3D10DDI_HDEVICE hDevice,
    const D3D11DDIARG_CREATERESOURCE* pCreateResource,
    D3D10DDI_HRESOURCE hResource,
    D3D10DDI_HRTRESOURCE hRTResource)
{
    RosUmdDevice* pRosUmdDevice = RosUmdDevice::CastFrom(hDevice);

    try {
        pRosUmdDevice->CreateResource(pCreateResource, hResource, hRTResource);
    }

    catch (std::exception & e)
    {
        pRosUmdDevice->SetException(e);
    }
}

void APIENTRY RosUmdDeviceDdi::DdiOpenResource(
    D3D10DDI_HDEVICE hDevice,
    const D3D10DDIARG_OPENRESOURCE* Args,
    D3D10DDI_HRESOURCE hResource,
    D3D10DDI_HRTRESOURCE hRTResource)
{
    RosUmdDevice* pRosUmdDevice = RosUmdDevice::CastFrom(hDevice);

    try {
        pRosUmdDevice->OpenResource(Args, hResource, hRTResource);
    }

    catch (const std::exception & e)
    {
        pRosUmdDevice->SetException(e);
    }
}

void APIENTRY RosUmdDeviceDdi::DdiDestroyResource(
    D3D10DDI_HDEVICE hDevice,
    D3D10DDI_HRESOURCE hResource)
{   
    RosUmdDevice* pRosUmdDevice = RosUmdDevice::CastFrom(hDevice);

    RosUmdResource * pResource = (RosUmdResource *)hResource.pDrvPrivate;

    pRosUmdDevice->DestroyResource(pResource);
}

SIZE_T APIENTRY RosUmdDeviceDdi::DdiCalcPrivateShaderResourceViewSize11(
    D3D10DDI_HDEVICE,
    const D3D11DDIARG_CREATESHADERRESOURCEVIEW*)
{
    return sizeof(RosUmdShaderResourceView);
}

void APIENTRY RosUmdDeviceDdi::DdiCreateShaderResourceView11(
    D3D10DDI_HDEVICE,
    const D3D11DDIARG_CREATESHADERRESOURCEVIEW* pCreate,
    D3D10DDI_HSHADERRESOURCEVIEW hShaderResourceView,
    D3D10DDI_HRTSHADERRESOURCEVIEW hRTShaderResourceView)
{
    new(hShaderResourceView.pDrvPrivate) RosUmdShaderResourceView(pCreate, hRTShaderResourceView);
}

void APIENTRY RosUmdDeviceDdi::DdiDestroyShaderResourceView(
    D3D10DDI_HDEVICE,
    D3D10DDI_HSHADERRESOURCEVIEW hShaderResourceView)
{
    RosUmdShaderResourceView * pShaderResourceView = RosUmdShaderResourceView::CastFrom(hShaderResourceView);
    pShaderResourceView->~RosUmdShaderResourceView();
}

void APIENTRY RosUmdDeviceDdi::DdiResourceCopy(
    D3D10DDI_HDEVICE hDevice,
    D3D10DDI_HRESOURCE hDestinationResource,
    D3D10DDI_HRESOURCE hSourceResource)
{
    RosUmdDevice* pRosUmdDevice = RosUmdDevice::CastFrom(hDevice);
    RosUmdResource * pDestinationResource = (RosUmdResource *)hDestinationResource.pDrvPrivate;
    RosUmdResource * pSourceResource = (RosUmdResource *)hSourceResource.pDrvPrivate;

    try
    {
        pRosUmdDevice->ResourceCopy(pDestinationResource, pSourceResource);
    }

    catch (std::exception & e)
    {
        pRosUmdDevice->SetException(e);
    }
}

void APIENTRY RosUmdDeviceDdi::DdiConstantBufferUpdateSubresourceUP11_1(
    D3D10DDI_HDEVICE   hDevice,
    D3D10DDI_HRESOURCE hDstResource,
    UINT               DstSubresource,
    _In_opt_ const D3D10_DDI_BOX  *pDstBox,
    _In_     const VOID           *pSysMemUP,
    UINT               RowPitch,
    UINT               DepthPitch,
    UINT               CopyFlags)
{
    RosUmdDevice* pRosUmdDevice = RosUmdDevice::CastFrom(hDevice);
    RosUmdResource * pResource = (RosUmdResource *)hDstResource.pDrvPrivate;

    try
    {
       pRosUmdDevice->ConstantBufferUpdateSubresourceUP(pResource, DstSubresource, pDstBox, pSysMemUP, RowPitch, DepthPitch, CopyFlags);
    }

    catch (std::exception & e)
    {
        pRosUmdDevice->SetException(e);
    }
}

void APIENTRY RosUmdDeviceDdi::DdiStagingResourceMap(
    D3D10DDI_HDEVICE hDevice,
    D3D10DDI_HRESOURCE hResource,
    UINT subResource,
    D3D10_DDI_MAP mapType,
    UINT mapFlags,
    D3D10DDI_MAPPED_SUBRESOURCE* pMappedSubRes)
{
    RosUmdDevice* pRosUmdDevice = RosUmdDevice::CastFrom(hDevice);
    RosUmdResource * pResource = (RosUmdResource *)hResource.pDrvPrivate;

    try
    {
        pRosUmdDevice->ResourceMap(pResource, subResource, mapType, mapFlags, pMappedSubRes);
    }

    catch (std::exception & e)
    {
        pRosUmdDevice->SetException(e);
    }
}

void APIENTRY RosUmdDeviceDdi::DdiStagingResourceUnmap(
    D3D10DDI_HDEVICE hDevice,
    D3D10DDI_HRESOURCE hResource,
    UINT subResource)
{
    RosUmdDevice* pRosUmdDevice = RosUmdDevice::CastFrom(hDevice);
    RosUmdResource * pResource = (RosUmdResource *)hResource.pDrvPrivate;

    try
    {
        pRosUmdDevice->ResourceUnmap(pResource, subResource);
    }

    catch (std::exception & e)
    {
        pRosUmdDevice->SetException(e);
    }
}

BOOL APIENTRY RosUmdDeviceDdi::DdiFlush(D3D10DDI_HDEVICE hDevice, UINT flushFlags)
{
    RosUmdDevice* pRosUmdDevice = RosUmdDevice::CastFrom(hDevice);

    BOOL bSuccess = TRUE;

    try
    {
        pRosUmdDevice->m_commandBuffer.Flush(flushFlags);
    }

    catch (std::exception & e)
    {
        pRosUmdDevice->SetException(e);
        bSuccess = FALSE;
    }

    return bSuccess;
}

void APIENTRY RosUmdDeviceDdi::DdiCheckFormatSupport(
    D3D10DDI_HDEVICE hDevice,
    DXGI_FORMAT Format,
    UINT* pFormatSupport)
{
    RosUmdDevice* pRosUmdDevice = RosUmdDevice::CastFrom(hDevice);

    pRosUmdDevice->CheckFormatSupport(Format, pFormatSupport);
}

void APIENTRY RosUmdDeviceDdi::DdiCheckCounterInfo(
    D3D10DDI_HDEVICE hDevice,
    D3D10DDI_COUNTER_INFO* pCounterInfo)
{
    RosUmdDevice* pRosUmdDevice = RosUmdDevice::CastFrom(hDevice);
    pRosUmdDevice->CheckCounterInfo(pCounterInfo);
}

void APIENTRY RosUmdDeviceDdi::DdiCheckMultisampleQualityLevels(
    D3D10DDI_HDEVICE hDevice,
    DXGI_FORMAT Format,
    UINT SampleCount,
    UINT Flags,
    UINT* pNumQualityLevels)
{
    RosUmdDevice* pRosUmdDevice = RosUmdDevice::CastFrom(hDevice);
    pRosUmdDevice->CheckMultisampleQualityLevels(Format, SampleCount, Flags, pNumQualityLevels);
}

SIZE_T APIENTRY RosUmdDeviceDdi::DdiCalcPrivateResourceSize(
    D3D10DDI_HDEVICE,
    const D3D11DDIARG_CREATERESOURCE*)
{
    return sizeof(RosUmdResource);
}

SIZE_T RosUmdDeviceDdi::DdiCalcPrivateOpenedResourceSize(
    D3D10DDI_HDEVICE,
    const D3D10DDIARG_OPENRESOURCE*)
{
    return sizeof(RosUmdResource);
}

//
// Rasterizer State
//

SIZE_T APIENTRY RosUmdDeviceDdi::DdiCalcRasterizerStateSize(
    D3D10DDI_HDEVICE,
    const D3D11_1_DDI_RASTERIZER_DESC*)
{
    return sizeof(RosUmdRasterizerState);
}

void APIENTRY RosUmdDeviceDdi::DdiCreateRasterizerState(
    D3D10DDI_HDEVICE,
    const D3D11_1_DDI_RASTERIZER_DESC* desc,
    D3D10DDI_HRASTERIZERSTATE hRasterizerState,
    D3D10DDI_HRTRASTERIZERSTATE hRTRasterizerState)
{
    RosUmdRasterizerState* pRasterizerState = new (hRasterizerState.pDrvPrivate) RosUmdRasterizerState(desc, hRTRasterizerState);
    pRasterizerState; // unused
}

void APIENTRY RosUmdDeviceDdi::DdiDestroyRasterizerState(
    D3D10DDI_HDEVICE hDevice,
    D3D10DDI_HRASTERIZERSTATE hRasterizerState)
{
    RosUmdDevice * pDevice = RosUmdDevice::CastFrom(hDevice);
    RosUmdRasterizerState * pRasterizerState = RosUmdRasterizerState::CastFrom(hRasterizerState);

    pDevice; // unused
    pRasterizerState; // unused
}

void APIENTRY RosUmdDeviceDdi::DdiSetRasterizerState(
    D3D10DDI_HDEVICE hDevice,
    D3D10DDI_HRASTERIZERSTATE hRasterizerState)
{
    RosUmdDevice * pDevice = RosUmdDevice::CastFrom(hDevice);
    RosUmdRasterizerState * pRasterizerState = RosUmdRasterizerState::CastFrom(hRasterizerState);
    pDevice->SetRasterizerState(pRasterizerState);
}

void APIENTRY RosUmdDeviceDdi::DdiSetScissorRects(
    _In_       D3D10DDI_HDEVICE hDevice,
    _In_       UINT             NumScissorRects,
    _In_       UINT             ClearScissorRects,
    _In_ const D3D10_DDI_RECT   *pRects
    )
{
    RosUmdDevice * pDevice = RosUmdDevice::CastFrom(hDevice);
    pDevice->SetScissorRects(NumScissorRects, ClearScissorRects, pRects);
}

//
// Depth Stencil State
//

SIZE_T APIENTRY RosUmdDeviceDdi::DdiCalcPrivateDepthStencilStateSize(
    D3D10DDI_HDEVICE hDevice,
    const D3D10_DDI_DEPTH_STENCIL_DESC* desc)
{
    RosUmdDevice * pDevice = RosUmdDevice::CastFrom(hDevice);
    pDevice; // unused
    desc; // unused

    return sizeof(RosUmdDepthStencilState);
}

void APIENTRY RosUmdDeviceDdi::DdiCreateDepthStencilState(
    D3D10DDI_HDEVICE hDevice,
    const D3D10_DDI_DEPTH_STENCIL_DESC* desc,
    D3D10DDI_HDEPTHSTENCILSTATE hDepthStencilState,
    D3D10DDI_HRTDEPTHSTENCILSTATE hRTDepthStencilState)
{
    RosUmdDevice * pDevice = RosUmdDevice::CastFrom(hDevice);
    pDevice; // unused

    RosUmdDepthStencilState* pDepthStencilState = new (hDepthStencilState.pDrvPrivate) RosUmdDepthStencilState(desc, hRTDepthStencilState);
    pDepthStencilState; // unused
}

void APIENTRY RosUmdDeviceDdi::DdiDestroyDepthStencilState(
    D3D10DDI_HDEVICE hDevice,
    D3D10DDI_HDEPTHSTENCILSTATE hDepthStencilState)
{
    RosUmdDevice * pDevice = RosUmdDevice::CastFrom(hDevice);
    RosUmdDepthStencilState * pDepthStencilState = RosUmdDepthStencilState::CastFrom(hDepthStencilState);

    pDevice; // unusd
    pDepthStencilState; // unused

}

void APIENTRY RosUmdDeviceDdi::DdiSetDepthStencilState(
    D3D10DDI_HDEVICE hDevice,
    D3D10DDI_HDEPTHSTENCILSTATE hDepthStencilState,
    UINT StencilRef)
{
    RosUmdDevice * pDevice = RosUmdDevice::CastFrom(hDevice);
    RosUmdDepthStencilState * pDepthStencilState = RosUmdDepthStencilState::CastFrom(hDepthStencilState);
    pDevice->SetDepthStencilState(pDepthStencilState, StencilRef);
}

//
// Sampler
//

SIZE_T APIENTRY RosUmdDeviceDdi::DdiCalcPrivateSamplerSize(
    D3D10DDI_HDEVICE hDevice,
    const D3D10_DDI_SAMPLER_DESC* desc)
{
    RosUmdDevice * pDevice = RosUmdDevice::CastFrom(hDevice);
    pDevice; // unused
    desc; // unused

    return sizeof(RosUmdSampler);
}

void APIENTRY RosUmdDeviceDdi::DdiCreateSampler(
    D3D10DDI_HDEVICE hDevice,
    const D3D10_DDI_SAMPLER_DESC* desc,
    D3D10DDI_HSAMPLER hSampler,
    D3D10DDI_HRTSAMPLER hRTSampler)
{
    RosUmdDevice * pDevice = RosUmdDevice::CastFrom(hDevice);
    pDevice; // unusd

    RosUmdSampler* pSampler = new (hSampler.pDrvPrivate) RosUmdSampler(desc, hRTSampler);
    pSampler; // unused
}

void APIENTRY RosUmdDeviceDdi::DdiDestroySampler(
    D3D10DDI_HDEVICE hDevice,
    D3D10DDI_HSAMPLER hSampler)
{
    RosUmdDevice * pDevice = RosUmdDevice::CastFrom(hDevice);
    pDevice; // unusd

    RosUmdSampler * pSampler = RosUmdSampler::CastFrom(hSampler);
    pSampler; // unused
}

void APIENTRY RosUmdDeviceDdi::DdiPSSetSamplers(
    D3D10DDI_HDEVICE hDevice,
    UINT Offset,
    UINT NumSamplers,
    const D3D10DDI_HSAMPLER* phSamplers)
{
    RosUmdDevice * pDevice = RosUmdDevice::CastFrom(hDevice);
    pDevice->SetPixelSamplers(Offset, NumSamplers, phSamplers);
}

void APIENTRY RosUmdDeviceDdi::DdiVSSetSamplers(
    D3D10DDI_HDEVICE hDevice,
    UINT Offset,
    UINT NumSamplers,
    const D3D10DDI_HSAMPLER* phSamplers)
{
    RosUmdDevice * pDevice = RosUmdDevice::CastFrom(hDevice);
    pDevice->SetVertexSamplers(Offset, NumSamplers, phSamplers);
}

void APIENTRY RosUmdDeviceDdi::DdiGSSetSamplers(
    D3D10DDI_HDEVICE hDevice,
    UINT Offset,
    UINT NumSamplers,
    const D3D10DDI_HSAMPLER* phSamplers)
{
    RosUmdDevice * pDevice = RosUmdDevice::CastFrom(hDevice);
    pDevice->SetGeometrySamplers(Offset, NumSamplers, phSamplers);
}

void APIENTRY RosUmdDeviceDdi::DdiCSSetSamplers(
    D3D10DDI_HDEVICE hDevice,
    UINT Offset,
    UINT NumSamplers,
    const D3D10DDI_HSAMPLER* phSamplers)
{
    RosUmdDevice * pDevice = RosUmdDevice::CastFrom(hDevice);
    pDevice->SetComputeSamplers(Offset, NumSamplers, phSamplers);
}

void APIENTRY RosUmdDeviceDdi::DdiHSSetSamplers(
    D3D10DDI_HDEVICE hDevice,
    UINT Offset,
    UINT NumSamplers,
    const D3D10DDI_HSAMPLER* phSamplers)
{
    RosUmdDevice * pDevice = RosUmdDevice::CastFrom(hDevice);
    pDevice->SetHullSamplers(Offset, NumSamplers, phSamplers);
}

void APIENTRY RosUmdDeviceDdi::DdiDSSetSamplers(
    D3D10DDI_HDEVICE hDevice,
    UINT Offset,
    UINT NumSamplers,
    const D3D10DDI_HSAMPLER* phSamplers)
{
    RosUmdDevice * pDevice = RosUmdDevice::CastFrom(hDevice);
    pDevice->SetDomainSamplers(Offset, NumSamplers, phSamplers);
}

//
// Element Layout
//

SIZE_T APIENTRY RosUmdDeviceDdi::DdiCalcPrivateElementLayoutSize(
    D3D10DDI_HDEVICE hDevice,
    const D3D10DDIARG_CREATEELEMENTLAYOUT* pCreate)
{
    RosUmdDevice * pDevice = RosUmdDevice::CastFrom(hDevice);
    pDevice; // unused
    pCreate; // unused

    return sizeof(RosUmdElementLayout);
}

void APIENTRY RosUmdDeviceDdi::DdiCreateElementLayout(
    D3D10DDI_HDEVICE hDevice,
    const D3D10DDIARG_CREATEELEMENTLAYOUT* pCreate,
    D3D10DDI_HELEMENTLAYOUT hElementLayout,
    D3D10DDI_HRTELEMENTLAYOUT hRTElementLayout)
{
    RosUmdDevice * pDevice = RosUmdDevice::CastFrom(hDevice);
    pDevice; // unused

    RosUmdElementLayout* pElementLayout = new (hElementLayout.pDrvPrivate) RosUmdElementLayout(pCreate, hRTElementLayout);
    pElementLayout; // unused
}

void APIENTRY RosUmdDeviceDdi::DdiDestroyElementLayout(
    D3D10DDI_HDEVICE hDevice,
    D3D10DDI_HELEMENTLAYOUT hElementLayout)
{
    RosUmdDevice * pDevice = RosUmdDevice::CastFrom(hDevice);
    pDevice; // unused

    RosUmdElementLayout * pElementLayout = RosUmdElementLayout::CastFrom(hElementLayout);
    pElementLayout->~RosUmdElementLayout();

}

void APIENTRY RosUmdDeviceDdi::DdiPsSetConstantBuffers11_1(
    D3D10DDI_HDEVICE hDevice,
    UINT offset,
    UINT numBuffers,
    const D3D10DDI_HRESOURCE* phBuffers,
    const UINT* pFirstConstant,
    const UINT* pNumConstants)
{
    RosUmdDevice * pDevice = RosUmdDevice::CastFrom(hDevice);

    pDevice->PsSetConstantBuffers11_1(offset, numBuffers, phBuffers, pFirstConstant, pNumConstants);
}
void APIENTRY RosUmdDeviceDdi::DdiIaSetInputLayout(
    D3D10DDI_HDEVICE hDevice,
    D3D10DDI_HELEMENTLAYOUT hElementLayout)
{
    RosUmdDevice * pDevice = RosUmdDevice::CastFrom(hDevice);
    RosUmdElementLayout * pElementLayout = RosUmdElementLayout::CastFrom(hElementLayout);
    pDevice->SetElementLayout(pElementLayout);
}

//
// Shaders
//

SIZE_T APIENTRY RosUmdDeviceDdi::DdiCalcPrivateShaderSize(
    D3D10DDI_HDEVICE hDevice,
    const UINT* pCode,
    const D3D11_1DDIARG_STAGE_IO_SIGNATURES* pSignatures)
{
    RosUmdDevice * pDevice = RosUmdDevice::CastFrom(hDevice);
    pDevice; // unused
    pCode; // unused
    pSignatures; // unused

    return sizeof(RosUmdPipelineShader);
}

void APIENTRY RosUmdDeviceDdi::DdiCreatePixelShader(
    D3D10DDI_HDEVICE hDevice,
    const UINT* pCode,
    D3D10DDI_HSHADER hShader,
    D3D10DDI_HRTSHADER hRTShader,
    const D3D11_1DDIARG_STAGE_IO_SIGNATURES* pSignatures)
{
    RosUmdDevice * pRosUmdDevice = RosUmdDevice::CastFrom(hDevice);

    try
    {
        pRosUmdDevice->CreatePixelShader(pCode, hShader, hRTShader, pSignatures);
    }

    catch (std::exception & e)
    {
        pRosUmdDevice->SetException(e);
    }
}

void APIENTRY RosUmdDeviceDdi::DdiCreateVertexShader(
    D3D10DDI_HDEVICE hDevice,
    const UINT* pCode,
    D3D10DDI_HSHADER hShader,
    D3D10DDI_HRTSHADER hRTShader,
    const D3D11_1DDIARG_STAGE_IO_SIGNATURES* pSignatures)
{
    RosUmdDevice * pRosUmdDevice = RosUmdDevice::CastFrom(hDevice);

    try
    {
        pRosUmdDevice->CreateVertexShader(pCode, hShader, hRTShader, pSignatures);
    }

    catch (std::exception & e)
    {
        pRosUmdDevice->SetException(e);
    }
}

void APIENTRY RosUmdDeviceDdi::DdiCreateGeometryShader(
    D3D10DDI_HDEVICE hDevice,
    const UINT* pCode,
    D3D10DDI_HSHADER hShader,
    D3D10DDI_HRTSHADER hRTShader,
    const D3D11_1DDIARG_STAGE_IO_SIGNATURES* pSignatures)
{
    RosUmdDevice * pRosUmdDevice = RosUmdDevice::CastFrom(hDevice);

    try
    {
        pRosUmdDevice->CreateGeometryShader(pCode, hShader, hRTShader, pSignatures);
    }

    catch (std::exception & e)
    {
        pRosUmdDevice->SetException(e);
    }
}

void APIENTRY RosUmdDeviceDdi::DdiCreateComputeShader(
    D3D10DDI_HDEVICE hDevice,
    const UINT* pCode,
    D3D10DDI_HSHADER hShader,
    D3D10DDI_HRTSHADER hRTShader)
{
    RosUmdDevice * pRosUmdDevice = RosUmdDevice::CastFrom(hDevice);

    try
    {
        pRosUmdDevice->CreateComputeShader(pCode, hShader, hRTShader);
    }

    catch (std::exception & e)
    {
        pRosUmdDevice->SetException(e);
    }
}

SIZE_T APIENTRY RosUmdDeviceDdi::DdiCalcPrivateTessellationShaderSize(
    D3D10DDI_HDEVICE hDevice,
    const UINT* pCode,
    const D3D11_1DDIARG_TESSELLATION_IO_SIGNATURES* pSignatures)
{
    RosUmdDevice * pDevice = RosUmdDevice::CastFrom(hDevice);

    pDevice; // unused
    pCode; // unused
    pSignatures; // unused

    return sizeof(RosUmdTesselationShader);
}

void APIENTRY RosUmdDeviceDdi::DdiCreateHullShader(
    D3D10DDI_HDEVICE hDevice,
    const UINT* pCode,
    D3D10DDI_HSHADER hShader,
    D3D10DDI_HRTSHADER hRTShader,
    const D3D11_1DDIARG_TESSELLATION_IO_SIGNATURES* pSignatures)
{
    RosUmdDevice * pRosUmdDevice = RosUmdDevice::CastFrom(hDevice);

    try
    {
        pRosUmdDevice->CreateTessellationShader(pCode, hShader, hRTShader, pSignatures, D3D11_SB_HULL_SHADER);
    }

    catch (std::exception & e)
    {
        pRosUmdDevice->SetException(e);
    }
}

void APIENTRY RosUmdDeviceDdi::DdiCreateDomainShader(
    D3D10DDI_HDEVICE hDevice,
    const UINT* pCode,
    D3D10DDI_HSHADER hShader,
    D3D10DDI_HRTSHADER hRTShader,
    const D3D11_1DDIARG_TESSELLATION_IO_SIGNATURES* pSignatures)
{
    RosUmdDevice * pRosUmdDevice = RosUmdDevice::CastFrom(hDevice);

    try
    {
        pRosUmdDevice->CreateTessellationShader(pCode, hShader, hRTShader, pSignatures, D3D11_SB_DOMAIN_SHADER);
    }

    catch (std::exception & e)
    {
        pRosUmdDevice->SetException(e);
    }
}

void APIENTRY RosUmdDeviceDdi::DdiDestroyShader(
    D3D10DDI_HDEVICE hDevice,
    D3D10DDI_HSHADER hShader)
{
    RosUmdDevice * pDevice = RosUmdDevice::CastFrom(hDevice);

    pDevice->DestroyShader(hShader);
}

void APIENTRY RosUmdDeviceDdi::DdiPSSetShaderResources(
    D3D10DDI_HDEVICE hDevice,
    UINT offset,
    UINT numViews,
    const D3D10DDI_HSHADERRESOURCEVIEW* pShaderResourceViews)
{
    RosUmdDevice * pDevice = RosUmdDevice::CastFrom(hDevice);

    pDevice->PSSetShaderResources(offset, numViews, pShaderResourceViews);
}

void APIENTRY RosUmdDeviceDdi::DdiPsSetShader(D3D10DDI_HDEVICE hDevice, D3D10DDI_HSHADER hShader)
{
    RosUmdDevice * pDevice = RosUmdDevice::CastFrom(hDevice);
    RosUmdShader * pShader = RosUmdShader::CastFrom(hShader);
    pDevice->SetPixelShader(pShader);
}

void APIENTRY RosUmdDeviceDdi::DdiVsSetShader(D3D10DDI_HDEVICE hDevice, D3D10DDI_HSHADER hShader)
{
    RosUmdDevice * pDevice = RosUmdDevice::CastFrom(hDevice);
    RosUmdShader * pShader = RosUmdShader::CastFrom(hShader);
    pDevice->SetVertexShader(pShader);
}

void APIENTRY RosUmdDeviceDdi::DdiGsSetShader(D3D10DDI_HDEVICE hDevice, D3D10DDI_HSHADER hShader)
{
    RosUmdDevice * pDevice = RosUmdDevice::CastFrom(hDevice);
    RosUmdShader * pShader = RosUmdShader::CastFrom(hShader);
    pDevice->SetGeometryShader(pShader);
}

void APIENTRY RosUmdDeviceDdi::DdiHsSetShader(D3D10DDI_HDEVICE hDevice, D3D10DDI_HSHADER hShader)
{
    RosUmdDevice * pDevice = RosUmdDevice::CastFrom(hDevice);
    RosUmdShader * pShader = RosUmdShader::CastFrom(hShader);
    pDevice->SetHullShader(pShader);
}

void APIENTRY RosUmdDeviceDdi::DdiDsSetShader(D3D10DDI_HDEVICE hDevice, D3D10DDI_HSHADER hShader)
{
    RosUmdDevice * pDevice = RosUmdDevice::CastFrom(hDevice);
    RosUmdShader * pShader = RosUmdShader::CastFrom(hShader);
    pDevice->SetDomainShader(pShader);
}

void APIENTRY RosUmdDeviceDdi::DdiCsSetShader(D3D10DDI_HDEVICE hDevice, D3D10DDI_HSHADER hShader)
{
    RosUmdDevice * pDevice = RosUmdDevice::CastFrom(hDevice);
    RosUmdShader * pShader = RosUmdShader::CastFrom(hShader);

    pDevice->SetComputeShader(pShader);
}

//
// Blend State
//

SIZE_T APIENTRY RosUmdDeviceDdi::DdiCalcPrivateBlendStateSize(
    D3D10DDI_HDEVICE hDevice,
    const D3D11_1_DDI_BLEND_DESC* desc)
{
    RosUmdDevice * pDevice = RosUmdDevice::CastFrom(hDevice);
    pDevice; // unused
    desc; // unused

    return sizeof(RosUmdBlendState);
}

void APIENTRY RosUmdDeviceDdi::DdiCreateBlendState(
    D3D10DDI_HDEVICE hDevice,
    const D3D11_1_DDI_BLEND_DESC* desc,
    D3D10DDI_HBLENDSTATE hBlendState,
    D3D10DDI_HRTBLENDSTATE hRTBlendState)
{
    RosUmdDevice * pDevice = RosUmdDevice::CastFrom(hDevice);
    pDevice; // unused

    RosUmdBlendState* pBlendState = new (hBlendState.pDrvPrivate) RosUmdBlendState(desc, hRTBlendState);
    pBlendState; // unused

}

void APIENTRY RosUmdDeviceDdi::DdiDestroyBlendState(
    D3D10DDI_HDEVICE hDevice,
    D3D10DDI_HBLENDSTATE hBlendState)
{
    RosUmdDevice * pDevice = RosUmdDevice::CastFrom(hDevice);
    pDevice; // unused

    RosUmdBlendState * pBlendState = RosUmdBlendState::CastFrom(hBlendState);
    pBlendState; // unused
}

void APIENTRY RosUmdDeviceDdi::DdiSetBlendState(
    D3D10DDI_HDEVICE hDevice,
    D3D10DDI_HBLENDSTATE hBlendState,
    const FLOAT pBlendFactor[4],
    UINT sampleMask)
{
    RosUmdDevice * pDevice = RosUmdDevice::CastFrom(hDevice);
    RosUmdBlendState * pBlendState = RosUmdBlendState::CastFrom(hBlendState);
    pDevice->SetBlendState(pBlendState, pBlendFactor, sampleMask);
}

//
// Render Target View
//

SIZE_T APIENTRY RosUmdDeviceDdi::DdiCalcPrivateRenderTargetViewSize(
    D3D10DDI_HDEVICE hDevice,
    const D3D10DDIARG_CREATERENDERTARGETVIEW* pCreate)
{
    RosUmdDevice * pDevice = RosUmdDevice::CastFrom(hDevice);
    pDevice; // unused
    pCreate; // unused

    return sizeof(RosUmdRenderTargetView);
}

void APIENTRY RosUmdDeviceDdi::DdiCreateRenderTargetView(
    D3D10DDI_HDEVICE hDevice,
    const D3D10DDIARG_CREATERENDERTARGETVIEW* pCreate,
    D3D10DDI_HRENDERTARGETVIEW hRenderTargetView,
    D3D10DDI_HRTRENDERTARGETVIEW hRTRenderTargetView)
{
    RosUmdDevice * pDevice = RosUmdDevice::CastFrom(hDevice);
    pDevice; // unused

    RosUmdRenderTargetView * pRenderTargetView = new(hRenderTargetView.pDrvPrivate) RosUmdRenderTargetView(pCreate, hRTRenderTargetView);
    pRenderTargetView; // unused

}

void APIENTRY RosUmdDeviceDdi::DdiDestroyRenderTargetView(
    D3D10DDI_HDEVICE hDevice,
    D3D10DDI_HRENDERTARGETVIEW hRenderTargetView)
{
    RosUmdDevice * pDevice = RosUmdDevice::CastFrom(hDevice);
    pDevice; // unused

    RosUmdRenderTargetView * pRenderTargetView = RosUmdRenderTargetView::CastFrom(hRenderTargetView);
    pRenderTargetView->~RosUmdRenderTargetView();

}

SIZE_T APIENTRY RosUmdDeviceDdi::DdiCalcPrivateDepthStencilViewSize11(
    D3D10DDI_HDEVICE,
    const D3D11DDIARG_CREATEDEPTHSTENCILVIEW*)
{
    return sizeof(RosUmdDepthStencilView);
}

void APIENTRY RosUmdDeviceDdi::DdiCreateDepthStencilView11(
    D3D10DDI_HDEVICE,
    const D3D11DDIARG_CREATEDEPTHSTENCILVIEW* pCreate,
    D3D10DDI_HDEPTHSTENCILVIEW hDepthStencilView,
    D3D10DDI_HRTDEPTHSTENCILVIEW hRTDepthStencilView)
{
    RosUmdDepthStencilView * pDepthStencilView = new(hDepthStencilView.pDrvPrivate) RosUmdDepthStencilView(pCreate, hRTDepthStencilView);
    pDepthStencilView;  // unused
}

void APIENTRY RosUmdDeviceDdi::DdiDestroyDepthStencilView(
    D3D10DDI_HDEVICE,
    D3D10DDI_HDEPTHSTENCILVIEW hDepthStencilView)
{
    RosUmdDepthStencilView * pDepthStencilView = RosUmdDepthStencilView::CastFrom(hDepthStencilView);
    pDepthStencilView->~RosUmdDepthStencilView();
}

void APIENTRY RosUmdDeviceDdi::DdiClearRenderTargetView(
    D3D10DDI_HDEVICE hDevice,
    D3D10DDI_HRENDERTARGETVIEW hRenderTargetView,
    FLOAT clearColor[4])
{
    RosUmdDevice * pDevice = RosUmdDevice::CastFrom(hDevice);
    RosUmdRenderTargetView * pRenderTargetView = RosUmdRenderTargetView::CastFrom(hRenderTargetView);
    pDevice->ClearRenderTargetView(pRenderTargetView, clearColor);
}

void APIENTRY RosUmdDeviceDdi::DdiClearDepthStencilView(
    D3D10DDI_HDEVICE hDevice,
    D3D10DDI_HDEPTHSTENCILVIEW hDepthStencilView,
    UINT clearFlags,
    FLOAT depthValue,
    UINT8 stencilValue)
{
    RosUmdDevice * pDevice = RosUmdDevice::CastFrom(hDevice);
    RosUmdDepthStencilView * pDepthStencilView = RosUmdDepthStencilView::CastFrom(hDepthStencilView);

    pDevice->ClearDepthStencilView(pDepthStencilView, clearFlags, depthValue, stencilValue);
}

void APIENTRY RosUmdDeviceDdi::DdiSetRenderTargets(
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
    RosUmdDevice * pDevice = RosUmdDevice::CastFrom(hDevice);

    pDevice->SetRenderTargets(phRenderTargetView, NumRTVs, RTVNumbertoUnbind, hDepthStencilView, phUnorderedAccessView, pUAVInitialCounts, UAVIndex,
        NumUAVs, UAVFirsttoSet, UAVNumberUpdated);
}

//
// View port
//

void APIENTRY RosUmdDeviceDdi::DdiSetViewports(
    D3D10DDI_HDEVICE hDevice,
    UINT numViewports,
    UINT clearViewports,
    const D3D10_DDI_VIEWPORT* pViewports)
{
    RosUmdDevice * pDevice = RosUmdDevice::CastFrom(hDevice);
    pDevice->SetViewports(numViewports, clearViewports, pViewports);
}

//
// Topology
//

void APIENTRY RosUmdDeviceDdi::DdiIaSetTopology(
    D3D10DDI_HDEVICE hDevice,
    D3D10_DDI_PRIMITIVE_TOPOLOGY topology)
{
    RosUmdDevice * pDevice = RosUmdDevice::CastFrom(hDevice);
    pDevice->SetTopology(topology);
}

//
// Draw
//

void APIENTRY RosUmdDeviceDdi::DdiDraw(
    D3D10DDI_HDEVICE hDevice,
    UINT vertexCount,
    UINT startVertexLocation)
{
    RosUmdDevice * pDevice = RosUmdDevice::CastFrom(hDevice);
    pDevice->Draw(vertexCount, startVertexLocation);
}

void APIENTRY RosUmdDeviceDdi::DdiDrawIndexed(
    D3D10DDI_HDEVICE hDevice,
    UINT indexCount,
    UINT startIndexLocation,
    INT baseVertexLocation)
{
    RosUmdDevice * pDevice = RosUmdDevice::CastFrom(hDevice);
    pDevice->DrawIndexed(indexCount, startIndexLocation, baseVertexLocation);
}

//
// Vertex Buffers
//

void APIENTRY RosUmdDeviceDdi::DdiIaSetVertexBuffers(
    D3D10DDI_HDEVICE hDevice,
    UINT startBuffer,
    UINT numBuffers,
    const D3D10DDI_HRESOURCE* phBuffers,
    const UINT* pStrides,
    const UINT* pOffsets)
{
    RosUmdDevice * pDevice = RosUmdDevice::CastFrom(hDevice);
    pDevice->SetVertexBuffers(startBuffer, numBuffers, phBuffers, pStrides, pOffsets);
}

//
// Index Buffer
//

void APIENTRY RosUmdDeviceDdi::DdiIaSetIndexBuffer(
    D3D10DDI_HDEVICE hDevice,
    D3D10DDI_HRESOURCE hIndexBuffer,
    DXGI_FORMAT hIndexFormat,
    UINT offset)
{
    RosUmdDevice * pDevice = RosUmdDevice::CastFrom(hDevice);
    pDevice->SetIndexBuffer(hIndexBuffer, hIndexFormat, offset);
}

void APIENTRY RosUmdDeviceDdi::DdiDynamicIABufferMapNoOverwrite(
    D3D10DDI_HDEVICE hDevice,
    D3D10DDI_HRESOURCE hResource,
    UINT subResource,
    D3D10_DDI_MAP mapType,
    UINT mapFlags,
    D3D10DDI_MAPPED_SUBRESOURCE* pMappedSubRes)
{
    RosUmdDevice* pRosUmdDevice = RosUmdDevice::CastFrom(hDevice);
    RosUmdResource * pResource = (RosUmdResource *)hResource.pDrvPrivate;

    try
    {
        pRosUmdDevice->ResourceMap(pResource, subResource, mapType, mapFlags, pMappedSubRes);
    }

    catch (std::exception & e)
    {
        pRosUmdDevice->SetException(e);
    }
}

void APIENTRY RosUmdDeviceDdi::DdiDynamicIABufferMapDiscard(
    D3D10DDI_HDEVICE hDevice,
    D3D10DDI_HRESOURCE hResource,
    UINT subResource,
    D3D10_DDI_MAP mapType,
    UINT mapFlags,
    D3D10DDI_MAPPED_SUBRESOURCE* pMappedSubRes)
{
    RosUmdDevice* pRosUmdDevice = RosUmdDevice::CastFrom(hDevice);
    RosUmdResource * pResource = (RosUmdResource *)hResource.pDrvPrivate;

    try
    {
        pRosUmdDevice->ResourceMap(pResource, subResource, mapType, mapFlags, pMappedSubRes);
    }

    catch (std::exception & e)
    {
        pRosUmdDevice->SetException(e);
    }
}

void APIENTRY RosUmdDeviceDdi::DdiDynamicIABufferUnmap(
    D3D10DDI_HDEVICE hDevice,
    D3D10DDI_HRESOURCE hResource,
    UINT subResource)
{
    RosUmdDevice* pRosUmdDevice = RosUmdDevice::CastFrom(hDevice);
    RosUmdResource * pResource = (RosUmdResource *)hResource.pDrvPrivate;

    try
    {
        pRosUmdDevice->ResourceUnmap(pResource, subResource);
    }

    catch (std::exception & e)
    {
        pRosUmdDevice->SetException(e);
    }
}

void APIENTRY RosUmdDeviceDdi::DdiVsSetConstantBuffers11_1(
    D3D10DDI_HDEVICE hDevice,
    UINT startBuffer,
    UINT numberBuffers,
    const D3D10DDI_HRESOURCE *  phResources,
    const UINT *    pFirstConstant,
    const UINT *    pNumberConstants)
{
    RosUmdDevice * pDevice = RosUmdDevice::CastFrom(hDevice);

    pDevice->VsSetConstantBuffers11_1(startBuffer, numberBuffers, phResources, pFirstConstant, pNumberConstants);
}

void APIENTRY RosUmdDeviceDdi::DdiDynamicConstantBufferMapDiscard(
    D3D10DDI_HDEVICE hDevice,
    D3D10DDI_HRESOURCE hResource,
    UINT subResource,
    D3D10_DDI_MAP mapType,
    UINT mapFlags,
    D3D10DDI_MAPPED_SUBRESOURCE* pMappedSubRes)
{
    RosUmdDevice* pRosUmdDevice = RosUmdDevice::CastFrom(hDevice);
    RosUmdResource * pResource = (RosUmdResource *)hResource.pDrvPrivate;

    try
    {
        pRosUmdDevice->ResourceMap(pResource, subResource, mapType, mapFlags, pMappedSubRes);
    }

    catch (std::exception & e)
    {
        pRosUmdDevice->SetException(e);
    }
}

void APIENTRY RosUmdDeviceDdi::DdiDynamicConstantBufferUnmap(
    D3D10DDI_HDEVICE hDevice,
    D3D10DDI_HRESOURCE hResource,
    UINT subResource)
{
    RosUmdDevice* pRosUmdDevice = RosUmdDevice::CastFrom(hDevice);
    RosUmdResource * pResource = (RosUmdResource *)hResource.pDrvPrivate;

    try
    {
        pRosUmdDevice->ResourceUnmap(pResource, subResource);
    }

    catch (std::exception & e)
    {
        pRosUmdDevice->SetException(e);
    }
}

HRESULT RosUmdDeviceDdi::Present(DXGI_DDI_ARG_PRESENT* pPresentData)
{
    RosUmdDevice* pRosUmdDevice = RosUmdDevice::CastFrom(pPresentData->hDevice);
    HRESULT hr = pRosUmdDevice->Present(pPresentData);
    if (FAILED(hr))
    {
        pRosUmdDevice->SetError(hr);
    }

    return hr;
}

HRESULT RosUmdDeviceDdi::RotateResourceIdentities(
    DXGI_DDI_ARG_ROTATE_RESOURCE_IDENTITIES* Args)
{
    RosUmdDevice* pRosUmdDevice = RosUmdDevice::CastFrom(Args->hDevice);
    HRESULT hr = pRosUmdDevice->RotateResourceIdentities(Args);
    if (FAILED(hr))
    {
        pRosUmdDevice->SetError(hr);
    }
    
    return hr;
}

HRESULT RosUmdDeviceDdi::SetDisplayMode(
    DXGI_DDI_ARG_SETDISPLAYMODE* pDisplayModeData)
{
    RosUmdDevice* pRosUmdDevice = RosUmdDevice::CastFrom(pDisplayModeData->hDevice);
    HRESULT hr = pRosUmdDevice->SetDisplayMode(pDisplayModeData);
    if (FAILED(hr))
    {
        pRosUmdDevice->SetError(hr);
    }

    return hr;
}

HRESULT RosUmdDeviceDdi::Present1(DXGI_DDI_ARG_PRESENT1* pPresentData)
{
    RosUmdDevice* pRosUmdDevice = RosUmdDevice::CastFrom(pPresentData->hDevice);
    HRESULT hr = pRosUmdDevice->Present1(pPresentData);
    if (FAILED(hr))
    {
        pRosUmdDevice->SetError(hr);
    }

    return hr;
}

_Use_decl_annotations_
void RosUmdDeviceDdi::CheckDirectFlipSupport(
    D3D10DDI_HDEVICE hDevice,
    D3D10DDI_HRESOURCE hResource1,
    D3D10DDI_HRESOURCE hResource2,
    UINT CheckDirectFlipFlags,
    BOOL *pSupported
    )
{
    RosUmdDevice* pRosUmdDevice = RosUmdDevice::CastFrom(hDevice);
    pRosUmdDevice->CheckDirectFlipSupport(
        hDevice,
        hResource1,
        hResource2,
        CheckDirectFlipFlags,
        pSupported);
}

void APIENTRY RosUmdDeviceDdi::DdiSetPredication(
    D3D10DDI_HDEVICE hDevice,
    D3D10DDI_HQUERY hQuery,
    BOOL bPredicateValue
    )
{
    RosUmdDevice* pRosUmdDevice = RosUmdDevice::CastFrom(hDevice);
    
    try
    {
        pRosUmdDevice->SetPredication(hQuery, bPredicateValue);
    }
    catch (std::exception & e)
    {
        pRosUmdDevice->SetException(e);
    }
}

void APIENTRY RosUmdDeviceDdi::DdiResourceCopyRegion11_1(
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
    RosUmdDevice* pRosUmdDevice = RosUmdDevice::CastFrom(hDevice);
    RosUmdResource * pDestinationResource = (RosUmdResource *) hDstResource.pDrvPrivate;
    RosUmdResource * pSourceResource = (RosUmdResource *) hSrcResource.pDrvPrivate;

    try
    {
        pRosUmdDevice->ResourceCopyRegion11_1(pDestinationResource, DstSubresource, DstX, DstY, DstZ, pSourceResource, SrcSubresource, pSrcBox, copyFlags);
    }

    catch (std::exception & e)
    {
        pRosUmdDevice->SetException(e);
    }
}

void APIENTRY RosUmdDeviceDdi::DdiResourceCopyRegion(
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