////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// UMD Device DDI
//
// Copyright (C) Microsoft Corporation
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "RosUmdLogging.h"

class RosUmdDeviceDdi
{
public:

    static void APIENTRY Draw_Default(D3D10DDI_HDEVICE, UINT, UINT) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY DdiDraw(D3D10DDI_HDEVICE, UINT, UINT);
    static void APIENTRY DdiDrawIndexed(D3D10DDI_HDEVICE, UINT, UINT, INT);
    static void APIENTRY DrawInstanced_Default(D3D10DDI_HDEVICE, UINT, UINT, UINT, UINT) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY DrawInstanced_Dirty(D3D10DDI_HDEVICE, UINT, UINT, UINT, UINT) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY DrawIndexedInstanced_Default(D3D10DDI_HDEVICE, UINT, UINT, UINT, INT, UINT) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY DrawIndexedInstanced_Dirty(D3D10DDI_HDEVICE, UINT, UINT, UINT, INT, UINT) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY DrawAuto_Default(D3D10DDI_HDEVICE) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY DrawAuto_Dirty(D3D10DDI_HDEVICE) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY DrawIndexedInstancedIndirect_Default(D3D10DDI_HDEVICE, D3D10DDI_HRESOURCE, UINT) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY DrawIndexedInstancedIndirect_Dirty(D3D10DDI_HDEVICE, D3D10DDI_HRESOURCE, UINT) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY DrawInstancedIndirect_Default(D3D10DDI_HDEVICE, D3D10DDI_HRESOURCE, UINT) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY DrawInstancedIndirect_Dirty(D3D10DDI_HDEVICE, D3D10DDI_HRESOURCE, UINT) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY Dispatch_Default(D3D10DDI_HDEVICE, UINT, UINT, UINT) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY Dispatch_Dirty(D3D10DDI_HDEVICE, UINT, UINT, UINT) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY DispatchIndirect_Default(D3D10DDI_HDEVICE, D3D10DDI_HRESOURCE, UINT) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY DispatchIndirect_Dirty(D3D10DDI_HDEVICE, D3D10DDI_HRESOURCE, UINT) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY DdiIaSetVertexBuffers(D3D10DDI_HDEVICE, UINT, UINT, const D3D10DDI_HRESOURCE*, const UINT*, const UINT*);
//    static void APIENTRY IaSetVertexBuffers_Preamble(D3D10DDI_HDEVICE, UINT, UINT, const D3D10DDI_HRESOURCE*, const UINT*, const UINT*) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY DdiIaSetIndexBuffer(D3D10DDI_HDEVICE, D3D10DDI_HRESOURCE, DXGI_FORMAT, UINT);
//    static void APIENTRY IaSetIndexBuffer_Default(D3D10DDI_HDEVICE, D3D10DDI_HRESOURCE, DXGI_FORMAT, UINT) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY IaSetIndexBuffer_Preamble(D3D10DDI_HDEVICE, D3D10DDI_HRESOURCE, DXGI_FORMAT, UINT) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY VSSetShaderResources_Default(D3D10DDI_HDEVICE, UINT, UINT, const D3D10DDI_HSHADERRESOURCEVIEW*) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY VSSetShaderResources_Preamble(D3D10DDI_HDEVICE, UINT, UINT, const D3D10DDI_HSHADERRESOURCEVIEW*) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY VsSetConstantBuffers_Default(D3D10DDI_HDEVICE, UINT, UINT, const D3D10DDI_HRESOURCE*) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY DdiVsSetConstantBuffers11_1(D3D10DDI_HDEVICE, UINT, UINT, const D3D10DDI_HRESOURCE*, const UINT*, const UINT*);
    static void APIENTRY VsSetConstantBuffers_Preamble(D3D10DDI_HDEVICE, UINT, UINT, const D3D10DDI_HRESOURCE*) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY VsSetConstantBuffers11_1_Preamble(D3D10DDI_HDEVICE, UINT, UINT, const D3D10DDI_HRESOURCE*, const UINT*, const UINT*) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY GSSetShaderResources_Default(D3D10DDI_HDEVICE, UINT, UINT, const D3D10DDI_HSHADERRESOURCEVIEW*) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY GSSetShaderResources_Preamble(D3D10DDI_HDEVICE, UINT, UINT, const D3D10DDI_HSHADERRESOURCEVIEW*) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY GsSetConstantBuffers_Default(D3D10DDI_HDEVICE, UINT, UINT, const D3D10DDI_HRESOURCE*) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY GsSetConstantBuffers11_1_Default(D3D10DDI_HDEVICE, UINT, UINT, const D3D10DDI_HRESOURCE*, const UINT*, const UINT*) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY GsSetConstantBuffers_Preamble(D3D10DDI_HDEVICE, UINT, UINT, const D3D10DDI_HRESOURCE*) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY GsSetConstantBuffers11_1_Preamble(D3D10DDI_HDEVICE, UINT, UINT, const D3D10DDI_HRESOURCE*, const UINT*, const UINT*) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY SOSetTargets_Default(D3D10DDI_HDEVICE, UINT, UINT, const D3D10DDI_HRESOURCE*, const UINT*) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY SOSetTargets_Preamble(D3D10DDI_HDEVICE, UINT, UINT, const D3D10DDI_HRESOURCE*, const UINT*) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY DdiPSSetShaderResources(D3D10DDI_HDEVICE, UINT, UINT, const D3D10DDI_HSHADERRESOURCEVIEW*);
    static void APIENTRY PSSetShaderResources_Preamble(D3D10DDI_HDEVICE, UINT, UINT, const D3D10DDI_HSHADERRESOURCEVIEW*) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY PsSetConstantBuffers_Default(D3D10DDI_HDEVICE, UINT, UINT, const D3D10DDI_HRESOURCE*) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY DdiPsSetConstantBuffers11_1(D3D10DDI_HDEVICE, UINT, UINT, const D3D10DDI_HRESOURCE*, const UINT*, const UINT*);
    static void APIENTRY PsSetConstantBuffers_Preamble(D3D10DDI_HDEVICE, UINT, UINT, const D3D10DDI_HRESOURCE*) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY PsSetConstantBuffers11_1_Preamble(D3D10DDI_HDEVICE, UINT, UINT, const D3D10DDI_HRESOURCE*, const UINT*, const UINT*) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY DdiSetRasterizerState(D3D10DDI_HDEVICE, D3D10DDI_HRASTERIZERSTATE);
    static void APIENTRY DdiSetScissorRects(D3D10DDI_HDEVICE, UINT, UINT, const D3D10_DDI_RECT*);

    static void APIENTRY DdiSetViewports(D3D10DDI_HDEVICE, UINT, UINT, const D3D10_DDI_VIEWPORT*);
    static void APIENTRY DdiIaSetTopology(D3D10DDI_HDEVICE, D3D10_DDI_PRIMITIVE_TOPOLOGY);

    static void APIENTRY DdiSetPredication(D3D10DDI_HDEVICE, D3D10DDI_HQUERY, BOOL); 
    static void APIENTRY SetTextFilter_Default(D3D10DDI_HDEVICE, UINT, UINT) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY ClearUnorderedAccessViewUint_Default(D3D10DDI_HDEVICE, D3D11DDI_HUNORDEREDACCESSVIEW, const UINT[4]) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY ClearUnorderedAccessViewFloat_Default(D3D10DDI_HDEVICE, D3D11DDI_HUNORDEREDACCESSVIEW, const FLOAT[4]) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY DdiClearDepthStencilView(D3D10DDI_HDEVICE, D3D10DDI_HDEPTHSTENCILVIEW, UINT, FLOAT, UINT8);
    static void APIENTRY Flush_Default(D3D10DDI_HDEVICE) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static BOOL APIENTRY DdiFlush(D3D10DDI_HDEVICE, UINT);
    static void APIENTRY GenerateMips_Default(D3D10DDI_HDEVICE, D3D10DDI_HSHADERRESOURCEVIEW) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY SetResourceMinLOD_Default(D3D10DDI_HDEVICE, D3D10DDI_HRESOURCE, FLOAT) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }

    static void APIENTRY QueryBegin_Default(D3D10DDI_HDEVICE, D3D10DDI_HQUERY) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY QueryEnd_Default(D3D10DDI_HDEVICE, D3D10DDI_HQUERY) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY QueryGetData_Default(D3D10DDI_HDEVICE, D3D10DDI_HQUERY, void*, UINT, UINT) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }

    static void APIENTRY DdiDynamicIABufferMapNoOverwrite(D3D10DDI_HDEVICE, D3D10DDI_HRESOURCE, UINT, D3D10_DDI_MAP, UINT, D3D10DDI_MAPPED_SUBRESOURCE*);
    static void APIENTRY DdiDynamicIABufferMapDiscard(D3D10DDI_HDEVICE, D3D10DDI_HRESOURCE, UINT, D3D10_DDI_MAP, UINT, D3D10DDI_MAPPED_SUBRESOURCE*);
    static void APIENTRY DdiDynamicIABufferUnmap(D3D10DDI_HDEVICE, D3D10DDI_HRESOURCE, UINT);
    static void APIENTRY DdiDynamicConstantBufferMapDiscard(D3D10DDI_HDEVICE, D3D10DDI_HRESOURCE, UINT, D3D10_DDI_MAP, UINT, D3D10DDI_MAPPED_SUBRESOURCE*);
    static void APIENTRY DdiDynamicConstantBufferUnmap(D3D10DDI_HDEVICE, D3D10DDI_HRESOURCE, UINT);
    static void APIENTRY DynamicResourceMapDiscard_Default(D3D10DDI_HDEVICE, D3D10DDI_HRESOURCE, UINT, D3D10_DDI_MAP, UINT, D3D10DDI_MAPPED_SUBRESOURCE*) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY DynamicResourceUnmap_Default(D3D10DDI_HDEVICE, D3D10DDI_HRESOURCE, UINT) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY DdiStagingResourceMap(D3D10DDI_HDEVICE, D3D10DDI_HRESOURCE, UINT, D3D10_DDI_MAP, UINT, D3D10DDI_MAPPED_SUBRESOURCE*);
    static void APIENTRY DdiStagingResourceUnmap(D3D10DDI_HDEVICE, D3D10DDI_HRESOURCE, UINT);
    static void APIENTRY ResourceMap_Default(D3D10DDI_HDEVICE, D3D10DDI_HRESOURCE, UINT, D3D10_DDI_MAP, UINT, D3D10DDI_MAPPED_SUBRESOURCE*) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY ResourceUnmap_Default(D3D10DDI_HDEVICE, D3D10DDI_HRESOURCE, UINT) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    // ShaderResourceViewReadAfterWriteHazard - Issue #34
    static void APIENTRY ShaderResourceViewReadAfterWriteHazard_Default(D3D10DDI_HDEVICE, D3D10DDI_HSHADERRESOURCEVIEW, D3D10DDI_HRESOURCE) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY ResourceReadAfterWriteHazard_Default(D3D10DDI_HDEVICE, D3D10DDI_HRESOURCE) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY ResourceWriteAfterWriteHazard_Default(D3D10DDI_HDEVICE, D3D10DDI_HRESOURCE) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY DdiResourceCopyRegion(D3D10DDI_HDEVICE, D3D10DDI_HRESOURCE, UINT, UINT, UINT, UINT, D3D10DDI_HRESOURCE, UINT, const D3D10_DDI_BOX*);
    static void APIENTRY DdiResourceCopyRegion11_1(D3D10DDI_HDEVICE, D3D10DDI_HRESOURCE, UINT, UINT, UINT, UINT, D3D10DDI_HRESOURCE, UINT, const D3D10_DDI_BOX*, UINT);
    static void APIENTRY DdiResourceCopy(D3D10DDI_HDEVICE, D3D10DDI_HRESOURCE, D3D10DDI_HRESOURCE);
    static void APIENTRY ResourceResolveSubresource_Default(D3D10DDI_HDEVICE, D3D10DDI_HRESOURCE, UINT, D3D10DDI_HRESOURCE, UINT, DXGI_FORMAT) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY DefaultConstantBufferUpdateSubresourceUP_Default(D3D10DDI_HDEVICE, D3D10DDI_HRESOURCE, UINT, const D3D10_DDI_BOX*, const VOID*, UINT, UINT) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY DdiConstantBufferUpdateSubresourceUP11_1(D3D10DDI_HDEVICE, D3D10DDI_HRESOURCE, UINT, const D3D10_DDI_BOX*, const VOID*, UINT, UINT, UINT);
    static void APIENTRY ResourceUpdateSubresourceUP_Default(D3D10DDI_HDEVICE, D3D10DDI_HRESOURCE, UINT, const D3D10_DDI_BOX*, const VOID*, UINT, UINT) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    // ResourceUpdateSubresourceUP11_1 - Issue #31
    static void APIENTRY ResourceUpdateSubresourceUP11_1_Default(D3D10DDI_HDEVICE, D3D10DDI_HRESOURCE, UINT, const D3D10_DDI_BOX*, const VOID*, UINT, UINT, UINT) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY CopyStructureCount_Default(D3D10DDI_HDEVICE, D3D10DDI_HRESOURCE, UINT, D3D11DDI_HUNORDEREDACCESSVIEW) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY CommandListExecute_Default(D3D10DDI_HDEVICE, D3D11DDI_HCOMMANDLIST) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }

    static void APIENTRY RelocateDeviceFuncs_Default(D3D10DDI_HDEVICE, D3D10DDI_DEVICEFUNCS*) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY RelocateDeviceFuncs1_Default(D3D10DDI_HDEVICE, D3D10_1DDI_DEVICEFUNCS*) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY RelocateDeviceFuncs11_Default(D3D10DDI_HDEVICE, D3D11DDI_DEVICEFUNCS*) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    // RelocateDeviceFuncs11_1 - Issue #35
    static void APIENTRY RelocateDeviceFuncs11_1_Default(D3D10DDI_HDEVICE, D3DWDDM1_3DDI_DEVICEFUNCS*) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY RelocateDeviceFuncsWDDM1_3_Default(D3D10DDI_HDEVICE, D3DWDDM1_3DDI_DEVICEFUNCS*) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY RelocateDeviceFuncsWDDM2_0_Default(D3D10DDI_HDEVICE, D3DWDDM2_0DDI_DEVICEFUNCS*) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static BOOL APIENTRY ResourceIsStagingBusy_Default(D3D10DDI_HDEVICE, D3D10DDI_HRESOURCE) { RosUmdLogging::Call(__FUNCTION__); __debugbreak();  return 0; }
    static SIZE_T APIENTRY CalcPrivateResourceSize_Default(D3D10DDI_HDEVICE, const D3D10DDIARG_CREATERESOURCE*) { RosUmdLogging::Call(__FUNCTION__); return 0; }
    static SIZE_T APIENTRY DdiCalcPrivateResourceSize(D3D10DDI_HDEVICE, const D3D11DDIARG_CREATERESOURCE*);
    static SIZE_T APIENTRY DdiCalcPrivateOpenedResourceSize(D3D10DDI_HDEVICE, const D3D10DDIARG_OPENRESOURCE*);
    static void APIENTRY CreateResource_Default(D3D10DDI_HDEVICE, const D3D10DDIARG_CREATERESOURCE*, D3D10DDI_HRESOURCE, D3D10DDI_HRTRESOURCE) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY DdiCreateResource(D3D10DDI_HDEVICE, const D3D11DDIARG_CREATERESOURCE*, D3D10DDI_HRESOURCE, D3D10DDI_HRTRESOURCE);
    static void APIENTRY DdiOpenResource(D3D10DDI_HDEVICE, const D3D10DDIARG_OPENRESOURCE*, D3D10DDI_HRESOURCE, D3D10DDI_HRTRESOURCE);
    static void APIENTRY DdiDestroyResource(D3D10DDI_HDEVICE, D3D10DDI_HRESOURCE);
    static SIZE_T APIENTRY CalcPrivateShaderResourceViewSize_Default(D3D10DDI_HDEVICE, const D3D10DDIARG_CREATESHADERRESOURCEVIEW*) { RosUmdLogging::Call(__FUNCTION__); return 0; }
    static void APIENTRY CreateShaderResourceView_Default(D3D10DDI_HDEVICE, const D3D10DDIARG_CREATESHADERRESOURCEVIEW*, D3D10DDI_HSHADERRESOURCEVIEW, D3D10DDI_HRTSHADERRESOURCEVIEW) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static SIZE_T APIENTRY CalcPrivateShaderResourceViewSize1_Default(D3D10DDI_HDEVICE, const D3D10_1DDIARG_CREATESHADERRESOURCEVIEW*) { RosUmdLogging::Call(__FUNCTION__); return 0; }
    static void APIENTRY CreateShaderResourceView1_Default(D3D10DDI_HDEVICE, const D3D10_1DDIARG_CREATESHADERRESOURCEVIEW*, D3D10DDI_HSHADERRESOURCEVIEW, D3D10DDI_HRTSHADERRESOURCEVIEW) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static SIZE_T APIENTRY DdiCalcPrivateShaderResourceViewSize11(D3D10DDI_HDEVICE, const D3D11DDIARG_CREATESHADERRESOURCEVIEW*);
    static void APIENTRY DdiCreateShaderResourceView11(D3D10DDI_HDEVICE, const D3D11DDIARG_CREATESHADERRESOURCEVIEW*, D3D10DDI_HSHADERRESOURCEVIEW, D3D10DDI_HRTSHADERRESOURCEVIEW);
    static void APIENTRY DdiDestroyShaderResourceView(D3D10DDI_HDEVICE, D3D10DDI_HSHADERRESOURCEVIEW);

    static SIZE_T APIENTRY DdiCalcPrivateRenderTargetViewSize(D3D10DDI_HDEVICE, const D3D10DDIARG_CREATERENDERTARGETVIEW*);
    static void APIENTRY DdiCreateRenderTargetView(D3D10DDI_HDEVICE, const D3D10DDIARG_CREATERENDERTARGETVIEW*, D3D10DDI_HRENDERTARGETVIEW, D3D10DDI_HRTRENDERTARGETVIEW);
    static void APIENTRY DdiDestroyRenderTargetView(D3D10DDI_HDEVICE, D3D10DDI_HRENDERTARGETVIEW);
    static void APIENTRY DdiClearRenderTargetView(D3D10DDI_HDEVICE, D3D10DDI_HRENDERTARGETVIEW, FLOAT[4]);
    static void APIENTRY DdiSetRenderTargets(D3D10DDI_HDEVICE, const D3D10DDI_HRENDERTARGETVIEW*, UINT, UINT, D3D10DDI_HDEPTHSTENCILVIEW, const D3D11DDI_HUNORDEREDACCESSVIEW*, const UINT*, UINT, UINT, UINT, UINT);

//    static void APIENTRY SetRenderTargets_Default(D3D10DDI_HDEVICE, const D3D10DDI_HRENDERTARGETVIEW*, UINT, UINT, D3D10DDI_HDEPTHSTENCILVIEW) { RosUmdLogging::Call(__FUNCTION__); }
//    static void APIENTRY SetRenderTargets_Preamble(D3D10DDI_HDEVICE, const D3D10DDI_HRENDERTARGETVIEW*, UINT, UINT, D3D10DDI_HDEPTHSTENCILVIEW) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
//    static void APIENTRY SetRenderTargets11_Preamble(D3D10DDI_HDEVICE, const D3D10DDI_HRENDERTARGETVIEW*, UINT, UINT, D3D10DDI_HDEPTHSTENCILVIEW, const D3D11DDI_HUNORDEREDACCESSVIEW*, const UINT*, UINT, UINT, UINT, UINT) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    
    static SIZE_T APIENTRY CalcPrivateUnorderedAccessViewSize_Default(D3D10DDI_HDEVICE, const D3D11DDIARG_CREATEUNORDEREDACCESSVIEW*) { RosUmdLogging::Call(__FUNCTION__); return 0; }
    static void APIENTRY CreateUnorderedAccessView_Default(D3D10DDI_HDEVICE, const D3D11DDIARG_CREATEUNORDEREDACCESSVIEW*, D3D11DDI_HUNORDEREDACCESSVIEW, D3D11DDI_HRTUNORDEREDACCESSVIEW) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY DestroyUnorderedAccessView_Default(D3D10DDI_HDEVICE, D3D11DDI_HUNORDEREDACCESSVIEW) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static SIZE_T APIENTRY CalcPrivateDepthStencilViewSize_Default(D3D10DDI_HDEVICE, const D3D10DDIARG_CREATEDEPTHSTENCILVIEW*) { RosUmdLogging::Call(__FUNCTION__); return 0; }
    static void APIENTRY CreateDepthStencilView_Default(D3D10DDI_HDEVICE, const D3D10DDIARG_CREATEDEPTHSTENCILVIEW*, D3D10DDI_HDEPTHSTENCILVIEW, D3D10DDI_HRTDEPTHSTENCILVIEW) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static SIZE_T APIENTRY DdiCalcPrivateDepthStencilViewSize11(D3D10DDI_HDEVICE, const D3D11DDIARG_CREATEDEPTHSTENCILVIEW*);
    static void APIENTRY DdiCreateDepthStencilView11(D3D10DDI_HDEVICE, const D3D11DDIARG_CREATEDEPTHSTENCILVIEW*, D3D10DDI_HDEPTHSTENCILVIEW, D3D10DDI_HRTDEPTHSTENCILVIEW);
    static void APIENTRY DdiDestroyDepthStencilView(D3D10DDI_HDEVICE, D3D10DDI_HDEPTHSTENCILVIEW);

//    static SIZE_T APIENTRY CalcPrivateBlendStateSize_Default(D3D10DDI_HDEVICE, const D3D10_DDI_BLEND_DESC*) { RosUmdLogging::Call(__FUNCTION__); return 0; }
//    static void APIENTRY CreateBlendState_Default(D3D10DDI_HDEVICE, const D3D10_DDI_BLEND_DESC*, D3D10DDI_HBLENDSTATE, D3D10DDI_HRTBLENDSTATE) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
//    static SIZE_T APIENTRY CalcPrivateBlendStateSize1_Default(D3D10DDI_HDEVICE, const D3D10_1_DDI_BLEND_DESC*) { RosUmdLogging::Call(__FUNCTION__); return 0; }
//    static void APIENTRY CreateBlendState1_Default(D3D10DDI_HDEVICE, const D3D10_1_DDI_BLEND_DESC*, D3D10DDI_HBLENDSTATE, D3D10DDI_HRTBLENDSTATE) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static SIZE_T APIENTRY DdiCalcPrivateBlendStateSize(D3D10DDI_HDEVICE, const D3D11_1_DDI_BLEND_DESC*);
    static void APIENTRY DdiCreateBlendState(D3D10DDI_HDEVICE, const D3D11_1_DDI_BLEND_DESC*, D3D10DDI_HBLENDSTATE, D3D10DDI_HRTBLENDSTATE);
    static void APIENTRY DdiDestroyBlendState(D3D10DDI_HDEVICE, D3D10DDI_HBLENDSTATE);
    static void APIENTRY DdiSetBlendState(D3D10DDI_HDEVICE, D3D10DDI_HBLENDSTATE, const FLOAT[4], UINT);

    static void APIENTRY DdiIaSetInputLayout(D3D10DDI_HDEVICE, D3D10DDI_HELEMENTLAYOUT);
    static SIZE_T APIENTRY DdiCalcPrivateElementLayoutSize(D3D10DDI_HDEVICE, const D3D10DDIARG_CREATEELEMENTLAYOUT*);
    static void APIENTRY DdiCreateElementLayout(D3D10DDI_HDEVICE, const D3D10DDIARG_CREATEELEMENTLAYOUT*, D3D10DDI_HELEMENTLAYOUT, D3D10DDI_HRTELEMENTLAYOUT);
    static void APIENTRY DdiDestroyElementLayout(D3D10DDI_HDEVICE, D3D10DDI_HELEMENTLAYOUT);

    static SIZE_T APIENTRY DdiCalcPrivateDepthStencilStateSize(D3D10DDI_HDEVICE, const D3D10_DDI_DEPTH_STENCIL_DESC*);
    static void APIENTRY DdiCreateDepthStencilState(D3D10DDI_HDEVICE, const D3D10_DDI_DEPTH_STENCIL_DESC*, D3D10DDI_HDEPTHSTENCILSTATE, D3D10DDI_HRTDEPTHSTENCILSTATE);
    static void APIENTRY DdiDestroyDepthStencilState(D3D10DDI_HDEVICE, D3D10DDI_HDEPTHSTENCILSTATE);
    static void APIENTRY DdiSetDepthStencilState(D3D10DDI_HDEVICE, D3D10DDI_HDEPTHSTENCILSTATE, UINT);

    static SIZE_T APIENTRY DdiCalcRasterizerStateSize(D3D10DDI_HDEVICE, const D3D11_1_DDI_RASTERIZER_DESC*);
    static void APIENTRY DdiCreateRasterizerState(D3D10DDI_HDEVICE, const D3D11_1_DDI_RASTERIZER_DESC*, D3D10DDI_HRASTERIZERSTATE, D3D10DDI_HRTRASTERIZERSTATE);
    static void APIENTRY DdiDestroyRasterizerState(D3D10DDI_HDEVICE, D3D10DDI_HRASTERIZERSTATE);

    static SIZE_T APIENTRY DdiCalcPrivateShaderSize(D3D10DDI_HDEVICE, const UINT*, const D3D11_1DDIARG_STAGE_IO_SIGNATURES*);
    static SIZE_T APIENTRY DdiCalcPrivateTessellationShaderSize(D3D10DDI_HDEVICE, const UINT*, const D3D11_1DDIARG_TESSELLATION_IO_SIGNATURES*);
    static void APIENTRY DdiCreateVertexShader(D3D10DDI_HDEVICE, const UINT*, D3D10DDI_HSHADER, D3D10DDI_HRTSHADER, const D3D11_1DDIARG_STAGE_IO_SIGNATURES*);
    static void APIENTRY DdiCreateHullShader(D3D10DDI_HDEVICE, const UINT*, D3D10DDI_HSHADER, D3D10DDI_HRTSHADER, const D3D11_1DDIARG_TESSELLATION_IO_SIGNATURES*);
    static void APIENTRY DdiCreateDomainShader(D3D10DDI_HDEVICE, const UINT*, D3D10DDI_HSHADER, D3D10DDI_HRTSHADER, const D3D11_1DDIARG_TESSELLATION_IO_SIGNATURES*);
    static void APIENTRY DdiCreateGeometryShader(D3D10DDI_HDEVICE, const UINT*, D3D10DDI_HSHADER, D3D10DDI_HRTSHADER, const D3D11_1DDIARG_STAGE_IO_SIGNATURES*);
    static void APIENTRY DdiCreatePixelShader(D3D10DDI_HDEVICE, const UINT*, D3D10DDI_HSHADER, D3D10DDI_HRTSHADER, const D3D11_1DDIARG_STAGE_IO_SIGNATURES*);
    static void APIENTRY DdiCreateComputeShader(D3D10DDI_HDEVICE, const UINT*, D3D10DDI_HSHADER, D3D10DDI_HRTSHADER);
    static void APIENTRY DdiDestroyShader(D3D10DDI_HDEVICE, D3D10DDI_HSHADER);
    static void APIENTRY DdiPsSetShader(D3D10DDI_HDEVICE, D3D10DDI_HSHADER);
    static void APIENTRY DdiVsSetShader(D3D10DDI_HDEVICE, D3D10DDI_HSHADER);
    static void APIENTRY DdiGsSetShader(D3D10DDI_HDEVICE, D3D10DDI_HSHADER);
    static void APIENTRY DdiHsSetShader(D3D10DDI_HDEVICE, D3D10DDI_HSHADER);
    static void APIENTRY DdiDsSetShader(D3D10DDI_HDEVICE, D3D10DDI_HSHADER);
    static void APIENTRY DdiCsSetShader(D3D10DDI_HDEVICE, D3D10DDI_HSHADER);

    static SIZE_T APIENTRY CalcPrivateGeometryShaderWithStreamOutputSize_Default(D3D10DDI_HDEVICE, const D3D10DDIARG_CREATEGEOMETRYSHADERWITHSTREAMOUTPUT*, const D3D10DDIARG_STAGE_IO_SIGNATURES*) { RosUmdLogging::Call(__FUNCTION__); return 0; }
    static void APIENTRY CreateGeometryShaderWithStreamOutput_Default(D3D10DDI_HDEVICE, const D3D10DDIARG_CREATEGEOMETRYSHADERWITHSTREAMOUTPUT*, D3D10DDI_HSHADER, D3D10DDI_HRTSHADER, const D3D10DDIARG_STAGE_IO_SIGNATURES*) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static SIZE_T APIENTRY CalcPrivateGeometryShaderWithStreamOutputSize11_Default(D3D10DDI_HDEVICE, const D3D11DDIARG_CREATEGEOMETRYSHADERWITHSTREAMOUTPUT*, const D3D10DDIARG_STAGE_IO_SIGNATURES*) { RosUmdLogging::Call(__FUNCTION__); return 0; }
    static SIZE_T APIENTRY CalcPrivateGeometryShaderWithStreamOutputSize11_1_Default(D3D10DDI_HDEVICE, const D3D11DDIARG_CREATEGEOMETRYSHADERWITHSTREAMOUTPUT*, const D3D11_1DDIARG_STAGE_IO_SIGNATURES*) { RosUmdLogging::Call(__FUNCTION__); return 0; }
    static void APIENTRY CreateGeometryShaderWithStreamOutput11_Default(D3D10DDI_HDEVICE, const D3D11DDIARG_CREATEGEOMETRYSHADERWITHSTREAMOUTPUT*, D3D10DDI_HSHADER, D3D10DDI_HRTSHADER, const D3D10DDIARG_STAGE_IO_SIGNATURES*) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY CreateGeometryShaderWithStreamOutput11_1_Default(D3D10DDI_HDEVICE, const D3D11DDIARG_CREATEGEOMETRYSHADERWITHSTREAMOUTPUT*, D3D10DDI_HSHADER, D3D10DDI_HRTSHADER, const D3D11_1DDIARG_STAGE_IO_SIGNATURES*) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }

    static SIZE_T APIENTRY DdiCalcPrivateSamplerSize(D3D10DDI_HDEVICE, const D3D10_DDI_SAMPLER_DESC*);
    static void APIENTRY DdiCreateSampler(D3D10DDI_HDEVICE, const D3D10_DDI_SAMPLER_DESC*, D3D10DDI_HSAMPLER, D3D10DDI_HRTSAMPLER);
    static void APIENTRY DdiDestroySampler(D3D10DDI_HDEVICE, D3D10DDI_HSAMPLER);
    static void APIENTRY DdiPSSetSamplers(D3D10DDI_HDEVICE, UINT, UINT, const D3D10DDI_HSAMPLER*);
    static void APIENTRY DdiVSSetSamplers(D3D10DDI_HDEVICE, UINT, UINT, const D3D10DDI_HSAMPLER*);
    static void APIENTRY DdiGSSetSamplers(D3D10DDI_HDEVICE, UINT, UINT, const D3D10DDI_HSAMPLER*);
    static void APIENTRY DdiCSSetSamplers(D3D10DDI_HDEVICE, UINT, UINT, const D3D10DDI_HSAMPLER*);
    static void APIENTRY DdiHSSetSamplers(D3D10DDI_HDEVICE, UINT, UINT, const D3D10DDI_HSAMPLER*);
    static void APIENTRY DdiDSSetSamplers(D3D10DDI_HDEVICE, UINT, UINT, const D3D10DDI_HSAMPLER*);

    static SIZE_T APIENTRY CalcPrivateQuerySize_Default(D3D10DDI_HDEVICE, const D3D10DDIARG_CREATEQUERY*) { ::OutputDebugStringA(__FUNCTION__); return 0; }
    static void APIENTRY CreateQuery_Default(D3D10DDI_HDEVICE, const D3D10DDIARG_CREATEQUERY*, D3D10DDI_HQUERY, D3D10DDI_HRTQUERY) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY DestroyQuery_Default(D3D10DDI_HDEVICE, D3D10DDI_HQUERY) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static SIZE_T APIENTRY CalcPrivateCommandListSize_Default(D3D10DDI_HDEVICE, CONST D3D11DDIARG_CREATECOMMANDLIST*) { ::OutputDebugStringA(__FUNCTION__); return 0; }
    static void APIENTRY CreateCommandList_Default(D3D10DDI_HDEVICE, CONST D3D11DDIARG_CREATECOMMANDLIST*, D3D11DDI_HCOMMANDLIST, D3D11DDI_HRTCOMMANDLIST) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY DestroyCommandList_Default(D3D10DDI_HDEVICE, D3D11DDI_HCOMMANDLIST) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }

    static void APIENTRY DdiCheckFormatSupport(D3D10DDI_HDEVICE, DXGI_FORMAT, UINT*);
    static void APIENTRY DdiCheckMultisampleQualityLevels(D3D10DDI_HDEVICE, DXGI_FORMAT, UINT, UINT, UINT*);
    static void APIENTRY CheckMultisampleQualityLevelsWDDM1_3_Default(D3D10DDI_HDEVICE, DXGI_FORMAT, UINT, UINT, UINT*) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY DdiCheckCounterInfo(D3D10DDI_HDEVICE, D3D10DDI_COUNTER_INFO*);
    static void APIENTRY CheckCounter_Default(D3D10DDI_HDEVICE, D3D10DDI_QUERY, D3D10DDI_COUNTER_TYPE*, UINT*, LPSTR, UINT*, LPSTR, UINT*, LPSTR, UINT*) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY CheckDeferredContextHandleSizes_Default(D3D10DDI_HDEVICE, UINT*, D3D11DDI_HANDLESIZE*) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static SIZE_T APIENTRY CalcDeferredContextHandleSize_Default(D3D10DDI_HDEVICE, D3D11DDI_HANDLETYPE, VOID*) { RosUmdLogging::Call(__FUNCTION__); __debugbreak();  return 0; }

    static void APIENTRY DdiDestroyDevice(D3D10DDI_HDEVICE);

    //DXGI DDI table entry points:
    static HRESULT APIENTRY Present(DXGI_DDI_ARG_PRESENT*);
    static HRESULT APIENTRY RotateResourceIdentities(DXGI_DDI_ARG_ROTATE_RESOURCE_IDENTITIES*);
    static HRESULT APIENTRY GetGammaCaps(DXGI_DDI_ARG_GET_GAMMA_CONTROL_CAPS*) { RosUmdLogging::Call(__FUNCTION__); __debugbreak();  return S_OK; }
    static HRESULT APIENTRY SetDisplayMode(DXGI_DDI_ARG_SETDISPLAYMODE*);
    static HRESULT APIENTRY SetResourcePriority(DXGI_DDI_ARG_SETRESOURCEPRIORITY*) { RosUmdLogging::Call(__FUNCTION__); __debugbreak();  return S_OK; }
    static HRESULT APIENTRY QueryResourceResidency(DXGI_DDI_ARG_QUERYRESOURCERESIDENCY*) { RosUmdLogging::Call(__FUNCTION__); __debugbreak();  return S_OK; }
    static HRESULT APIENTRY Blt(DXGI_DDI_ARG_BLT*) { RosUmdLogging::Call(__FUNCTION__); __debugbreak();  return S_OK; }
    static HRESULT APIENTRY ResolveSharedResource(DXGI_DDI_ARG_RESOLVESHAREDRESOURCE*) { RosUmdLogging::Call(__FUNCTION__); __debugbreak();  return S_OK; }
    static HRESULT APIENTRY Blt1(DXGI_DDI_ARG_BLT1*) { RosUmdLogging::Call(__FUNCTION__); __debugbreak();  return S_OK; }
    static HRESULT APIENTRY Present1(DXGI_DDI_ARG_PRESENT1*);


    static void APIENTRY HSSetShaderResources_Default(D3D10DDI_HDEVICE, UINT, UINT, const D3D10DDI_HSHADERRESOURCEVIEW*) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY HSSetShaderResources_Preamble(D3D10DDI_HDEVICE, UINT, UINT, const D3D10DDI_HSHADERRESOURCEVIEW*) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY HsSetConstantBuffers_Default(D3D10DDI_HDEVICE, UINT, UINT, const D3D10DDI_HRESOURCE*) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY HsSetConstantBuffers11_1_Default(D3D10DDI_HDEVICE, UINT, UINT, const D3D10DDI_HRESOURCE*, const UINT*, const UINT*) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY HsSetConstantBuffers_Preamble(D3D10DDI_HDEVICE, UINT, UINT, const D3D10DDI_HRESOURCE*) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY HsSetConstantBuffers11_1_Preamble(D3D10DDI_HDEVICE, UINT, UINT, const D3D10DDI_HRESOURCE*, const UINT*, const UINT*) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY DSSetShaderResources_Default(D3D10DDI_HDEVICE, UINT, UINT, const D3D10DDI_HSHADERRESOURCEVIEW*) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY DSSetShaderResources_Preamble(D3D10DDI_HDEVICE, UINT, UINT, const D3D10DDI_HSHADERRESOURCEVIEW*) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY DsSetConstantBuffers_Default(D3D10DDI_HDEVICE, UINT, UINT, const D3D10DDI_HRESOURCE*) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY DsSetConstantBuffers11_1_Default(D3D10DDI_HDEVICE, UINT, UINT, const D3D10DDI_HRESOURCE*, const UINT*, const UINT*) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY DsSetConstantBuffers_Preamble(D3D10DDI_HDEVICE, UINT, UINT, const D3D10DDI_HRESOURCE*) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY DsSetConstantBuffers11_1_Preamble(D3D10DDI_HDEVICE, UINT, UINT, const D3D10DDI_HRESOURCE*, const UINT*, const UINT*) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY CSSetShaderResources_Default(D3D10DDI_HDEVICE, UINT, UINT, const D3D10DDI_HSHADERRESOURCEVIEW*) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY CSSetShaderResources11_1_Default(D3D10DDI_HDEVICE, UINT, UINT, const D3D10DDI_HSHADERRESOURCEVIEW*, const UINT*, const UINT*) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY CSSetShaderResources_Preamble(D3D10DDI_HDEVICE, UINT, UINT, const D3D10DDI_HSHADERRESOURCEVIEW*) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY CSSetShaderResources11_1_Preamble(D3D10DDI_HDEVICE, UINT, UINT, const D3D10DDI_HSHADERRESOURCEVIEW*, const UINT*, const UINT*) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY CSSetUnorderedAccessViews_Default(D3D10DDI_HDEVICE, UINT, UINT, const D3D11DDI_HUNORDEREDACCESSVIEW*, const UINT*) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY CSSetUnorderedAccessViews_Preamble(D3D10DDI_HDEVICE, UINT, UINT, const D3D11DDI_HUNORDEREDACCESSVIEW*, const UINT*) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY CsSetConstantBuffers_Default(D3D10DDI_HDEVICE, UINT, UINT, const D3D10DDI_HRESOURCE*) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY CsSetConstantBuffers11_1_Default(D3D10DDI_HDEVICE, UINT, UINT, const D3D10DDI_HRESOURCE*, const UINT*, const UINT*) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY CsSetConstantBuffers_Preamble(D3D10DDI_HDEVICE, UINT, UINT, const D3D10DDI_HRESOURCE*) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY CsSetConstantBuffers11_1_Preamble(D3D10DDI_HDEVICE, UINT, UINT, const D3D10DDI_HRESOURCE*, const UINT*, const UINT*) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }

    static void APIENTRY PsSetShaderWithInterfaces_Default(D3D10DDI_HDEVICE, D3D10DDI_HSHADER, UINT, const UINT*, const D3D11DDIARG_POINTERDATA*) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY VsSetShaderWithInterfaces_Default(D3D10DDI_HDEVICE, D3D10DDI_HSHADER, UINT, const UINT*, const D3D11DDIARG_POINTERDATA*) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY GsSetShaderWithInterfaces_Default(D3D10DDI_HDEVICE, D3D10DDI_HSHADER, UINT, const UINT*, const D3D11DDIARG_POINTERDATA*) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY HsSetShaderWithInterfaces_Default(D3D10DDI_HDEVICE, D3D10DDI_HSHADER, UINT, const UINT*, const D3D11DDIARG_POINTERDATA*) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY DsSetShaderWithInterfaces_Default(D3D10DDI_HDEVICE, D3D10DDI_HSHADER, UINT, const UINT*, const D3D11DDIARG_POINTERDATA*) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY CsSetShaderWithInterfaces_Default(D3D10DDI_HDEVICE, D3D10DDI_HSHADER, UINT, const UINT*, const D3D11DDIARG_POINTERDATA*) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }

    static void APIENTRY Discard_Default(D3D10DDI_HDEVICE, D3D11DDI_HANDLETYPE, VOID*, const D3D10_DDI_RECT*, UINT) { RosUmdLogging::Call(__FUNCTION__); /*__debugbreak();*/ }

    static void APIENTRY AssignDebugBinary_Default(D3D10DDI_HDEVICE, D3D10DDI_HSHADER, UINT, CONST VOID*) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY DynamicConstantBufferMapNoOverwrite_Default(D3D10DDI_HDEVICE, D3D10DDI_HRESOURCE, UINT, D3D10_DDI_MAP, UINT, D3D10DDI_MAPPED_SUBRESOURCE*) { RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }
    static void APIENTRY CheckDirectFlipSupport (
        D3D10DDI_HDEVICE hDevice,
        D3D10DDI_HRESOURCE hResource1,
        D3D10DDI_HRESOURCE hResource2,
        UINT CheckDirectFlipFlags,
        _Out_ BOOL *pSupported);

    // ClearView - Issue #33
    static void APIENTRY ClearView_Default(D3D10DDI_HDEVICE, D3D11DDI_HANDLETYPE, VOID*, const FLOAT Color[4], const D3D10_DDI_RECT*, UINT) { Color; RosUmdLogging::Call(__FUNCTION__); __debugbreak(); }

    static const D3DWDDM1_3DDI_DEVICEFUNCS s_deviceFuncsWDDM1_3;
    static const DXGI1_3_DDI_BASE_FUNCTIONS s_dxgiDeviceFuncs4;

};
