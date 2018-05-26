#include "CosUmd12.h"

HRESULT APIENTRY Ddi_Dxgi_Present(
    DXGI_DDI_ARG_PRESENT* pDesc)
{
    DebugBreak();
    return E_NOTIMPL;
}


HRESULT APIENTRY Ddi_Dxgi_GetGammaCaps(
    DXGI_DDI_ARG_GET_GAMMA_CONTROL_CAPS* pCaps)
{
    DebugBreak();
    return E_NOTIMPL;
}

HRESULT APIENTRY Ddi_Dxgi_SetDisplayMode(
    DXGI_DDI_ARG_SETDISPLAYMODE* pArgs)
{
    DebugBreak();
    return E_NOTIMPL;
}

HRESULT APIENTRY Ddi_Dxgi_SetResourcePriority(
    DXGI_DDI_ARG_SETRESOURCEPRIORITY* pPriority)
{
    DebugBreak();
    return E_NOTIMPL;
}

HRESULT APIENTRY Ddi_Dxgi_QueryResourceResidency(DXGI_DDI_ARG_QUERYRESOURCERESIDENCY* pArgs)
{
    DebugBreak();
    return E_NOTIMPL;
}

HRESULT APIENTRY Ddi_Dxgi_RotateResourceIdentities(
    DXGI_DDI_ARG_ROTATE_RESOURCE_IDENTITIES* pDesc)
{
    DebugBreak();
    return E_NOTIMPL;
}

HRESULT APIENTRY Ddi_Dxgi_Blt(
    DXGI_DDI_ARG_BLT* pBlt)
{
    DebugBreak();
    return E_NOTIMPL;
}

HRESULT APIENTRY Ddi_Dxgi_ResolveSharedResource(
    DXGI_DDI_ARG_RESOLVESHAREDRESOURCE* pResolve)
{
    DebugBreak();
    return E_NOTIMPL;
}

HRESULT APIENTRY Ddi_Dxgi_Blt1(
    DXGI_DDI_ARG_BLT1* pBlt)
{
    DebugBreak();
    return E_NOTIMPL;
}

HRESULT APIENTRY Ddi_Dxgi_OfferResources(
    DXGI_DDI_ARG_OFFERRESOURCES* pOffer)
{
    DebugBreak();
    return E_NOTIMPL;
}

HRESULT APIENTRY Ddi_Dxgi_ReclaimResources(
    DXGI_DDI_ARG_RECLAIMRESOURCES* pReclaim)
{   
    DebugBreak();
    return E_NOTIMPL;
}

HRESULT APIENTRY Ddi_Dxgi_GetMultiplaneOverlayCaps(
    DXGI_DDI_ARG_GETMULTIPLANEOVERLAYCAPS* pCaps)
{
    DebugBreak();
    return E_NOTIMPL;
}

HRESULT APIENTRY Ddi_Dxgi_GetMultiplaneOverlayGroupCaps(
    DXGI_DDI_ARG_GETMULTIPLANEOVERLAYGROUPCAPS* pCaps)
{
    DebugBreak();
    return E_NOTIMPL;
}

HRESULT APIENTRY Ddi_Dxgi_PresentMultiplaneOverlay(
    DXGI_DDI_ARG_PRESENTMULTIPLANEOVERLAY* pDesc)
{
    DebugBreak();
    return E_NOTIMPL;
}

HRESULT APIENTRY Ddi_Dxgi_Present1(
    DXGI_DDI_ARG_PRESENT1* pDesc)
{
    DebugBreak();
    return E_NOTIMPL;
}

HRESULT APIENTRY Ddi_Dxgi_CheckPresentDurationSupport(
    DXGI_DDI_ARG_CHECKPRESENTDURATIONSUPPORT* pSupport)
{
    DebugBreak();
    return E_NOTIMPL;
}

HRESULT APIENTRY Ddi_Dxgi_TrimResidencySet(
    DXGI_DDI_ARG_TRIMRESIDENCYSET* pDesc)
{
    DebugBreak();
    return E_NOTIMPL;
}

HRESULT APIENTRY Ddi_Dxgi_CheckMultiplaneOverlayColorSpaceSupport(
    DXGI_DDI_ARG_CHECKMULTIPLANEOVERLAYCOLORSPACESUPPORT* pSupport)
{
    DebugBreak();
    return E_NOTIMPL;
}

HRESULT APIENTRY Ddi_Dxgi_PresentMultiplaneOverlay1(
    DXGI_DDI_ARG_PRESENTMULTIPLANEOVERLAY1* pDesc)
{
    DebugBreak();
    return E_NOTIMPL;
}

DXGI1_4_DDI_BASE_FUNCTIONS g_CosUmd12Dxgi_Ddi =
{
    Ddi_Dxgi_Present,                                // pfnPresent
    Ddi_Dxgi_GetGammaCaps,                           // pfnGetGammaCaps
    Ddi_Dxgi_SetDisplayMode,                         // pfnSetDisplayMode
    Ddi_Dxgi_SetResourcePriority,                    // pfnSetResourcePriority
    Ddi_Dxgi_QueryResourceResidency,                 // pfnQueryResourceResidency
    Ddi_Dxgi_RotateResourceIdentities,               // pfnRotateResourceIdentities
    Ddi_Dxgi_Blt,                                    // pfnBlt
    Ddi_Dxgi_ResolveSharedResource,                  // pfnResolveSharedResource
    Ddi_Dxgi_Blt1,                                   // pfnBlt1
    Ddi_Dxgi_OfferResources,                         // pfnOfferResources
    Ddi_Dxgi_ReclaimResources,                       // pfnReclaimResources
    Ddi_Dxgi_GetMultiplaneOverlayCaps,               // pfnGetMultiplaneOverlayCaps
    Ddi_Dxgi_GetMultiplaneOverlayGroupCaps,          // pfnGetMultiplaneOverlayGroupCaps
    nullptr,                                         // pfnReserved1
    Ddi_Dxgi_PresentMultiplaneOverlay,               // pfnPresentMultiplaneOverlay
    nullptr,                                         // pfnReserved2
    Ddi_Dxgi_Present1,                               // pfnPresent1
    Ddi_Dxgi_CheckPresentDurationSupport,            // pfnCheckPresentDurationSupport
    Ddi_Dxgi_TrimResidencySet,                       // pfnTrimResidencySet
    Ddi_Dxgi_CheckMultiplaneOverlayColorSpaceSupport,// pfnCheckMultiplaneOverlayColorSpaceSupport
    Ddi_Dxgi_PresentMultiplaneOverlay1               // pfnPresentMultiplaneOverlay1
};
