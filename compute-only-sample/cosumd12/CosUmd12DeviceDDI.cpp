#include "CosUmd12.h"

void APIENTRY CosUmd12Device_Ddi_CheckFormatSupport(
    D3D12DDI_HDEVICE Device,
    DXGI_FORMAT Format,
    _Out_ UINT* pFlags)
{
    DebugBreak();
}

void APIENTRY CosUmd12Device_Ddi_CheckMultisampleQualityLevels(
    D3D12DDI_HDEVICE Device,
    DXGI_FORMAT Format,
    UINT SampleCount,
    D3D12DDI_MULTISAMPLE_QUALITY_LEVEL_FLAGS Flags,
    _Out_ UINT* pNumQualityLevels)
{
    DebugBreak();
}
void APIENTRY CosUmd12Device_Ddi_GetMipPacking(
    D3D12DDI_HDEVICE Device,
    D3D12DDI_HRESOURCE TiledResource,
    _Out_ UINT* pNumPackedMips,
    _Out_ UINT* pNumTilesForPackedMips)
{
    DebugBreak();
}

SIZE_T APIENTRY CosUmd12Device_Ddi_CalcPrivateElementLayoutSize_0010(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDIARG_CREATEELEMENTLAYOUT_0010* pDesc)
{
    DebugBreak();

    return 0;
}

void APIENTRY CosUmd12Device_Ddi_CreateElementLayout_0010(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDIARG_CREATEELEMENTLAYOUT_0010* pDesc,
    D3D12DDI_HELEMENTLAYOUT ElementLayout)
{
    DebugBreak();
}

void APIENTRY CosUmd12Device_Ddi_DestroyElementLayout(
    D3D12DDI_HDEVICE Device, 
    D3D12DDI_HELEMENTLAYOUT ElementLayout)
{
    DebugBreak();
}

SIZE_T APIENTRY CosUmd12Device_Ddi_CalcPrivateBlendStateSize_0010(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDI_BLEND_DESC_0010* pDesc)
{
    DebugBreak();

    return 0;
}

void APIENTRY CosUmd12Device_Ddi_CreateBlendState_0010(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDI_BLEND_DESC_0010* pDesc,
    D3D12DDI_HBLENDSTATE BlendState)
{
    DebugBreak();
}

void APIENTRY CosUmd12Device_Ddi_DestroyBlendState(
    D3D12DDI_HDEVICE Device, 
    D3D12DDI_HBLENDSTATE BlendState)
{
    DebugBreak();
}

SIZE_T APIENTRY CosUmd12Device_Ddi_CalcPrivateDepthStencilStateSize_0025(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDI_DEPTH_STENCIL_DESC_0025* pDesc)
{
    DebugBreak();

    return 0;
}

void APIENTRY CosUmd12Device_Ddi_CreateDepthStencilState_0025(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDI_DEPTH_STENCIL_DESC_0025* pDesc,
    D3D12DDI_HDEPTHSTENCILSTATE DepthStencilState)
{
    DebugBreak();
}

void APIENTRY CosUmd12Device_Ddi_DestroyDepthStencilState(
    D3D12DDI_HDEVICE Device,
    D3D12DDI_HDEPTHSTENCILSTATE DepthStencilState)
{
    DebugBreak();
}

SIZE_T APIENTRY CosUmd12Device_Ddi_CalcPrivateRasterizerStateSize_0010(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDI_RASTERIZER_DESC_0010* pDesc)
{
	DebugBreak();

    return 0;
}

void APIENTRY CosUmd12Device_Ddi_CreateRasterizerState_0010(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDI_RASTERIZER_DESC_0010* pDesc,
    D3D12DDI_HRASTERIZERSTATE RasterizerState)
{
    DebugBreak();
}

void APIENTRY CosUmd12Device_Ddi_DestroyRasterizerState(
    D3D12DDI_HDEVICE Device, 
    D3D12DDI_HRASTERIZERSTATE RasterizerState)
{
	DebugBreak();
}

SIZE_T APIENTRY CosUmd12Device_Ddi_CalcPrivateShaderSize_0026(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDIARG_CREATE_SHADER_0026* pDesc)
{
	DebugBreak();

    return 0;
}

void APIENTRY CosUmd12Device_Ddi_CreateVertexShader_0026(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDIARG_CREATE_SHADER_0026* pDesc,
    D3D12DDI_HSHADER Shader)
{
	DebugBreak();
}

void APIENTRY CosUmd12Device_Ddi_CreatePixelShader_0026(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDIARG_CREATE_SHADER_0026* pDesc,
    D3D12DDI_HSHADER Shader)
{
	DebugBreak();
}

void APIENTRY CosUmd12Device_Ddi_CreateGeometryShader_0026(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDIARG_CREATE_SHADER_0026* pDesc,
    D3D12DDI_HSHADER Shader)
{
	DebugBreak();
}

void APIENTRY CosUmd12Device_Ddi_CreateComputeShader_0026(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDIARG_CREATE_SHADER_0026* pDesc,
    D3D12DDI_HSHADER Shader)
{
	DebugBreak();
}

SIZE_T APIENTRY CosUmd12Device_Ddi_CalcPrivateGeometryShaderWithStreamOutput_0026(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDIARG_CREATE_GEOMETRY_SHADER_WITH_STREAM_OUTPUT_0026* pDesc)
{
	DebugBreak();

    return 0;
}

void APIENTRY CosUmd12Device_Ddi_CreateGeometryShaderWithStreamOutput_0026(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDIARG_CREATE_GEOMETRY_SHADER_WITH_STREAM_OUTPUT_0026* pDesc,
    D3D12DDI_HSHADER Shader)
{
	DebugBreak();
}

SIZE_T APIENTRY CosUmd12Device_Ddi_CalcPrivateTessellationShaderSize_0026(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDIARG_CREATE_SHADER_0026* pDesc)
{
	DebugBreak();

    return 0;
}

void APIENTRY CosUmd12Device_Ddi_CreateHullShader_0026(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDIARG_CREATE_SHADER_0026* pDesc,
    D3D12DDI_HSHADER Shader)
{
	DebugBreak();
}

void APIENTRY CosUmd12Device_Ddi_CreateDomainShader_0026(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDIARG_CREATE_SHADER_0026* pDesc,
    D3D12DDI_HSHADER Shader)
{
	DebugBreak();
}

void APIENTRY CosUmd12Device_Ddi_DestroyShader(
    D3D12DDI_HDEVICE Device, 
    D3D12DDI_HSHADER Shader)
{
	DebugBreak();
}

SIZE_T APIENTRY CosUmd12Device_Ddi_CalcPrivateCommandQueueSize_0023(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDIARG_CREATECOMMANDQUEUE_0023* pDesc)
{
	DebugBreak();

    return 0;
}

HRESULT APIENTRY CosUmd12Device_Ddi_CreateCommandQueue_0023(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDIARG_CREATECOMMANDQUEUE_0023* pDesc,
    D3D12DDI_HCOMMANDQUEUE DrvCommandQueue,
    D3D12DDI_HRTCOMMANDQUEUE RTCommandQueue)
{
	DebugBreak();

    return E_NOTIMPL;
}

void APIENTRY CosUmd12Device_Ddi_DestroyCommandQueue(
    D3D12DDI_HDEVICE Device,
    D3D12DDI_HCOMMANDQUEUE CommandQueue)
{
	DebugBreak();
}

SIZE_T APIENTRY CosUmd12Device_Ddi_CalcPrivateCommandAllocatorSize(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDIARG_CREATECOMMANDALLOCATOR* pDesc)
{
	DebugBreak();

    return 0;
}

HRESULT APIENTRY CosUmd12Device_Ddi_CreateCommandAllocator(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDIARG_CREATECOMMANDALLOCATOR* pDesc)
{
	DebugBreak();

    return E_NOTIMPL;
}

void APIENTRY CosUmd12Device_Ddi_DestroyCommandAllocator(
    D3D12DDI_HDEVICE Device,
    D3D12DDI_HCOMMANDALLOCATOR CommandAllocator)
{
	DebugBreak();
}

void APIENTRY CosUmd12Device_Ddi_ResetCommandAllocator(
    D3D12DDI_HCOMMANDALLOCATOR CommandAllocator)
{
	DebugBreak();
}

SIZE_T APIENTRY CosUmd12Device_Ddi_CalcPrivatePipelineStateSize_0033(
    D3D12DDI_HDEVICE Device, 
    _In_ const D3D12DDIARG_CREATE_PIPELINE_STATE_0033* pDesc)
{
	DebugBreak();

    return 0;
}

HRESULT APIENTRY CosUmd12Device_Ddi_CreatePipelineState_0033(
    D3D12DDI_HDEVICE Device, 
    _In_ const D3D12DDIARG_CREATE_PIPELINE_STATE_0033* pDesc, 
    D3D12DDI_HPIPELINESTATE PipelineState,
    D3D12DDI_HRTPIPELINESTATE RTPipelineState)
{
	DebugBreak();

    return E_NOTIMPL;
}

VOID APIENTRY CosUmd12Device_Ddi_DestroyPipelineState(
    D3D12DDI_HDEVICE Device, 
    D3D12DDI_HPIPELINESTATE PipelineState)
{
	DebugBreak();
}

SIZE_T APIENTRY CosUmd12Device_Ddi_CalcPrivateCommandListSize_0001(
    D3D12DDI_HDEVICE Device, 
    _In_ const D3D12DDIARG_CREATE_COMMAND_LIST_0001* pDesc)
{
	DebugBreak();

    return 0;
}

HRESULT APIENTRY CosUmd12Device_Ddi_CreateCommandList_0001(
    D3D12DDI_HDEVICE Device, 
    _In_ const D3D12DDIARG_CREATE_COMMAND_LIST_0001* pDesc)
{
	DebugBreak();

    return E_NOTIMPL;
}

void APIENTRY CosUmd12Device_Ddi_DestroyCommandList(
    D3D12DDI_HDEVICE Device, 
    D3D12DDI_HCOMMANDLIST CommandList)
{
	DebugBreak();
}

SIZE_T APIENTRY CosUmd12Device_Ddi_CalcPrivateFenceSize(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDIARG_CREATE_FENCE* pDesc)
{
	DebugBreak();

    return 0;
}

HRESULT APIENTRY CosUmd12Device_Ddi_CreateFence(
    D3D12DDI_HDEVICE Device,
    D3D12DDI_HFENCE Fence,
    _In_ const D3D12DDIARG_CREATE_FENCE* pDesc)
{
	DebugBreak();

    return E_NOTIMPL;
}

void APIENTRY CosUmd12Device_Ddi_DestroyFence(
    D3D12DDI_HDEVICE Device,
    D3D12DDI_HFENCE Fence)
{
	DebugBreak();
}

SIZE_T APIENTRY CosUmd12Device_Ddi_CalcPrivateDescriptorHeapSize_0001(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDIARG_CREATE_DESCRIPTOR_HEAP_0001* pDesc)
{
	DebugBreak();

    return 0;
}

HRESULT APIENTRY CosUmd12Device_Ddi_CreateDescriptorHeap_0001(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDIARG_CREATE_DESCRIPTOR_HEAP_0001* pDesc,
    D3D12DDI_HDESCRIPTORHEAP DescriptorHeap)
{
	DebugBreak();

    return E_NOTIMPL;
}

void APIENTRY CosUmd12Device_Ddi_DestroyDescriptorHeap(
    D3D12DDI_HDEVICE Device,
    D3D12DDI_HDESCRIPTORHEAP DescriptorHeap)
{
	DebugBreak();
}

UINT APIENTRY CosUmd12Device_Ddi_GetDescriptorSizeInBytes(
    D3D12DDI_HDEVICE Device,
    D3D12DDI_DESCRIPTOR_HEAP_TYPE HeapType)
{
	DebugBreak();

    return 0;
}

D3D12DDI_CPU_DESCRIPTOR_HANDLE APIENTRY CosUmd12Device_Ddi_GetCpuDescriptorHandleForHeapStart(
    D3D12DDI_HDEVICE Device, 
    D3D12DDI_HDESCRIPTORHEAP DescriptorHeap)
{
	DebugBreak();

    return { 0 };
}

D3D12DDI_GPU_DESCRIPTOR_HANDLE APIENTRY CosUmd12Device_Ddi_GetGpuDescriptorHandleForHeapStart(
    D3D12DDI_HDEVICE Device,
    D3D12DDI_HDESCRIPTORHEAP DescriptorHeap)
{
	DebugBreak();

    return { 0 };
}

void APIENTRY CosUmd12Device_Ddi_CreateShaderResourceView_0002(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDIARG_CREATE_SHADER_RESOURCE_VIEW_0002* pDesc,
    _In_ D3D12DDI_CPU_DESCRIPTOR_HANDLE DestDescriptor)
{
	DebugBreak();
}

void APIENTRY CosUmd12Device_Ddi_CreateConstantBufferView(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDI_CONSTANT_BUFFER_VIEW_DESC* pDesc,
    _In_ D3D12DDI_CPU_DESCRIPTOR_HANDLE DestDescriptor)
{
	DebugBreak();
}

void APIENTRY CosUmd12Device_Ddi_CreateSampler(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDIARG_CREATE_SAMPLER* pDesc,
    _In_ D3D12DDI_CPU_DESCRIPTOR_HANDLE DestDescriptor)
{
	DebugBreak();
}

void APIENTRY CosUmd12Device_Ddi_CreateUnorderedAccessView_0002(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDIARG_CREATE_UNORDERED_ACCESS_VIEW_0002* pDesc,
    _In_ D3D12DDI_CPU_DESCRIPTOR_HANDLE DestDescriptor)
{
	DebugBreak();
}

void APIENTRY CosUmd12Device_Ddi_CreateRenderTargetView_0002(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDIARG_CREATE_RENDER_TARGET_VIEW_0002* pDesc,
    _In_ D3D12DDI_CPU_DESCRIPTOR_HANDLE DestDescriptor)
{
	DebugBreak();
}

void APIENTRY CosUmd12Device_Ddi_CreateDepthStencilView(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDIARG_CREATE_DEPTH_STENCIL_VIEW* pDesc,
    _In_ D3D12DDI_CPU_DESCRIPTOR_HANDLE DestDescriptor)
{
	DebugBreak();
}


SIZE_T APIENTRY CosUmd12Device_Ddi_CalcPrivateRootSignatureSize_0013(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDIARG_CREATE_ROOT_SIGNATURE_0013* pDesc)
{
	DebugBreak();

    return 0;
}

HRESULT APIENTRY CosUmd12Device_Ddi_CreateRootSignature_0013(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDIARG_CREATE_ROOT_SIGNATURE_0013* pDesc,
    D3D12DDI_HROOTSIGNATURE RootSignature)
{
	DebugBreak();

    return E_NOTIMPL;
}

void APIENTRY CosUmd12Device_Ddi_DestroyRootSignature(
    D3D12DDI_HDEVICE Device,
    D3D12DDI_HROOTSIGNATURE RootSignature)
{
	DebugBreak();
}

HRESULT APIENTRY CosUmd12Device_Ddi_MapHeap(
    D3D12DDI_HDEVICE Device,
    D3D12DDI_HHEAP Heap,
    _Out_ void** pHeapData)
{
	DebugBreak();

    return E_NOTIMPL;
}

void APIENTRY CosUmd12Device_Ddi_UnmapHeap(
    D3D12DDI_HDEVICE Device,
    D3D12DDI_HHEAP Heap)
{
	DebugBreak();
}

D3D12DDI_HEAP_AND_RESOURCE_SIZES APIENTRY CosUmd12Device_Ddi_CalcPrivateHeapAndResourceSizes_0030(
    D3D12DDI_HDEVICE Device,
    _In_opt_ const D3D12DDIARG_CREATEHEAP_0001* pHeapDesc,
    _In_opt_ const D3D12DDIARG_CREATERESOURCE_0003* pResourceDesc,
    D3D12DDI_HPROTECTEDRESOURCESESSION_0030 hProtectedResourceSession)
{
	DebugBreak();

    return { 0 };
}

HRESULT APIENTRY CosUmd12Device_Ddi_CreateHeapAndResource_0030(
    D3D12DDI_HDEVICE Device,
    _In_opt_ const D3D12DDIARG_CREATEHEAP_0001* pHeapDesc,
    D3D12DDI_HHEAP Heap,
    D3D12DDI_HRTRESOURCE RtHeap,
    _In_opt_ const D3D12DDIARG_CREATERESOURCE_0003* pResourceDesc,
    _In_opt_ const D3D12DDI_CLEAR_VALUES* pClearValues,
    D3D12DDI_HPROTECTEDRESOURCESESSION_0030 hProtectedResourceSession, //TODO: DRM!
    D3D12DDI_HRESOURCE Resource)
{
	DebugBreak();

    return E_NOTIMPL;
}

void APIENTRY CosUmd12Device_Ddi_DestroyHeapAndResource(
    D3D12DDI_HDEVICE Device,
    D3D12DDI_HHEAP Heap,
    D3D12DDI_HRESOURCE Resource)
{
	DebugBreak();
}

HRESULT APIENTRY CosUmd12Device_Ddi_MakeResident_0001(
    D3D12DDI_HDEVICE Device,
    D3D12DDIARG_MAKERESIDENT_0001* pDesc)
{
	DebugBreak();

    return E_NOTIMPL;
}

HRESULT APIENTRY CosUmd12Device_Ddi_Evict2(
    D3D12DDI_HDEVICE Device,
    const D3D12DDIARG_EVICT* pDesc)
{
	DebugBreak();

    return E_NOTIMPL;
}

D3D12DDI_HEAP_AND_RESOURCE_SIZES APIENTRY CosUmd12Device_Ddi_CalcPrivateOpenedHeapAndResourceSizes_0003(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDIARG_OPENHEAP_0003* pDesc)
{
	DebugBreak();

    return { 0 };
}

HRESULT APIENTRY CosUmd12Device_Ddi_OpenHeapAndResource_0003(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDIARG_OPENHEAP_0003* pDesc,
    D3D12DDI_HHEAP Heap,
    D3D12DDI_HRTRESOURCE RtResource,
    D3D12DDI_HRESOURCE Resource)
{
	DebugBreak();

    return E_NOTIMPL;
}

void APIENTRY CosUmd12Device_Ddi_CopyDescriptors_0003(
    D3D12DDI_HDEVICE Device,
    _In_ UINT NumDestDescriptorRanges,
    _In_reads_(NumDestDescriptorRanges) const D3D12DDI_CPU_DESCRIPTOR_HANDLE* pDestDescriptorRangeStarts,
    _In_reads_opt_(NumDestDescriptorRanges) const UINT* pDestDescriptorRangeSizes, // NULL means all ranges 1
    _In_ UINT NumSrcDescriptorRanges,
    _In_reads_(NumSrcDescriptorRanges) const D3D12DDI_CPU_DESCRIPTOR_HANDLE* pSrcDescriptorRangeStarts,
    _In_reads_opt_(NumSrcDescriptorRanges) const UINT* pSrcDescriptorRangeSizes, // NULL means all ranges 1
    _In_ D3D12DDI_DESCRIPTOR_HEAP_TYPE DescriptorHeapsType)
{
	DebugBreak();
}

void APIENTRY CosUmd12Device_Ddi_CopyDescriptorsSimple_0003(
    D3D12DDI_HDEVICE,
    _In_ UINT NumDescriptors,
    _In_ D3D12DDI_CPU_DESCRIPTOR_HANDLE DestDescriptorRangeStart,
    _In_ D3D12DDI_CPU_DESCRIPTOR_HANDLE SrcDescriptorRangeStart,
    _In_ D3D12DDI_DESCRIPTOR_HEAP_TYPE DescriptorHeapsType)
{
	DebugBreak();
}

SIZE_T APIENTRY CosUmd12Device_Ddi_CalcPrivateQueryHeapSize_0001(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDIARG_CREATE_QUERY_HEAP_0001* pDesc)
{
	DebugBreak();

    return 0;
}

HRESULT APIENTRY CosUmd12Device_Ddi_CreateQueryHeap_0001(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDIARG_CREATE_QUERY_HEAP_0001* pDesc,
    D3D12DDI_HQUERYHEAP QueryHeap)
{
	DebugBreak();

    return E_NOTIMPL;
}

void APIENTRY CosUmd12Device_Ddi_DestroyQueryHeap(
    D3D12DDI_HDEVICE Device,
    D3D12DDI_HQUERYHEAP QueryHeap)
{
	DebugBreak();
}

SIZE_T APIENTRY CosUmd12Device_Ddi_CalcPrivateCommandSignatureSize_0001(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDIARG_CREATE_COMMAND_SIGNATURE_0001* pDesc)
{
	DebugBreak();

    return 0;
}

HRESULT APIENTRY CosUmd12Device_Ddi_CreateCommandSignature_0001(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDIARG_CREATE_COMMAND_SIGNATURE_0001* pDesc,
    D3D12DDI_HCOMMANDSIGNATURE CommandSignature)
{
	DebugBreak();

    return E_NOTIMPL;
}

void APIENTRY CosUmd12Device_Ddi_DestroyCommandSignature(
    D3D12DDI_HDEVICE Device, 
    D3D12DDI_HCOMMANDSIGNATURE CommandSignature)
{
	DebugBreak();
}


D3D12DDI_GPU_VIRTUAL_ADDRESS APIENTRY CosUmd12Device_Ddi_CheckResourceVirtualAddress(
    D3D12DDI_HDEVICE Device,
    D3D12DDI_HRESOURCE Resource)
{
	DebugBreak();

    return NULL;
}

void APIENTRY CosUmd12Device_Ddi_CheckResourceAllocationInfo_0022(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDIARG_CREATERESOURCE_0003* pDesc,
    D3D12DDI_RESOURCE_OPTIMIZATION_FLAGS Flags,
    UINT32 AlignmentRestriction,
    UINT VisibleNodeMask,
    _Out_ D3D12DDI_RESOURCE_ALLOCATION_INFO_0022* pInfo)
{
	DebugBreak();
}

void APIENTRY CosUmd12Device_Ddi_CheckSubresourceInfo(
    D3D12DDI_HDEVICE Device,
    D3D12DDI_HRESOURCE Resource,
    UINT Subresource,
    _Out_ D3D12DDI_SUBRESOURCE_INFO* pInfo)
{
	DebugBreak();
}

void APIENTRY CosUmd12Device_Ddi_CheckExistingResourceAllocationInfo_0022(
    D3D12DDI_HDEVICE Device,
    D3D12DDI_HRESOURCE Resource,
    _Out_ D3D12DDI_RESOURCE_ALLOCATION_INFO_0022* pInfo)
{
	DebugBreak();
}

HRESULT APIENTRY CosUmd12Device_Ddi_OfferResources(
    D3D12DDI_HDEVICE Device,
    const D3D12DDIARG_OFFERRESOURCES* pDesc)
{
	DebugBreak();

    return E_NOTIMPL;
}

HRESULT APIENTRY CosUmd12Device_Ddi_ReclaimResources_0001(
     D3D12DDI_HDEVICE Device, 
     D3D12DDIARG_RECLAIMRESOURCES_0001* pDesc)
{
	DebugBreak();

    return E_NOTIMPL;
}
UINT APIENTRY CosUmd12Device_Ddi_GetImplicitPhysicalAdapterMask(
    D3D12DDI_HDEVICE Device)
{
	DebugBreak();

    return 0;
}

UINT APIENTRY CosUmd12Device_Ddi_GetPresentPrivateDriverDataSize(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDIARG_PRESENT_0001* pDesc)
{
	DebugBreak();

    return 0;
}

void APIENTRY CosUmd12Device_Ddi_QueryNodeMap(
    D3D12DDI_HDEVICE Device,
    UINT NumPhysicalAdapters,
    _Out_writes_(NumPhysicalAdapters) UINT* pMap)
{
	DebugBreak();
}

HRESULT APIENTRY CosUmd12Device_Ddi_RetrieveShaderComment_0003(
    D3D12DDI_HDEVICE Device,
    D3D12DDI_HPIPELINESTATE PipelineState,
    _Out_writes_z_(*pCharacterCountIncludingNullTerminator) WCHAR* pBuffer,
    _Inout_ SIZE_T* pCharacterCountIncludingNullTerminator)
{
	DebugBreak();

    return E_NOTIMPL;
}

D3DKMT_HANDLE APIENTRY CosUmd12Device_Ddi_CheckResourceAllocationHandle(
    D3D12DDI_HDEVICE Device,
    D3D10DDI_HRESOURCE Resource)
{
	DebugBreak();

    return NULL;
}

SIZE_T APIENTRY CosUmd12Device_Ddi_CalcPrivatePipelineLibrarySize_0010(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDIARG_CREATE_PIPELINE_LIBRARY_0010* pDesc)
{
	DebugBreak();

    return 0;
}

HRESULT APIENTRY CosUmd12Device_Ddi_CreatePipelineLibrary_0010(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDIARG_CREATE_PIPELINE_LIBRARY_0010* pDesc,
    D3D12DDI_HPIPELINELIBRARY PipelineLibrary)
{
	DebugBreak();

    return E_NOTIMPL;
}

void APIENTRY CosUmd12Device_Ddi_DestroyPipelineLibrary_0010(
    D3D12DDI_HDEVICE Device, 
    D3D12DDI_HPIPELINELIBRARY PipelineLibrary)
{
	DebugBreak();
}

HRESULT APIENTRY CosUmd12Device_Ddi_AddPipelineStateToLibrary_0010(
    D3D12DDI_HDEVICE Device,
    D3D12DDI_HPIPELINELIBRARY Library,
    D3D12DDI_HPIPELINESTATE PipelineState,
    UINT PipelineIndex)
{
	DebugBreak();

    return E_NOTIMPL;
}

SIZE_T APIENTRY CosUmd12Device_Ddi_CalcSerializedLibrarySize_0010(
    D3D12DDI_HDEVICE Device,
    D3D12DDI_HPIPELINELIBRARY hLibrary)
{
	DebugBreak();

    return 0;
}

HRESULT APIENTRY CosUmd12Device_Ddi_SerializeLibrary_0010(
    D3D12DDI_HDEVICE Device,
    D3D12DDI_HPIPELINELIBRARY hLibrary,
    _Out_ void *pBlob)
{
	DebugBreak();

    return E_NOTIMPL;
}

void APIENTRY CosUmd12Device_Ddi_GetDebugAllocationInfo_0014(
    D3D12DDI_HDEVICE Device,
    D3D12DDI_HANDLE_AND_TYPE Object,
    _Inout_ UINT* pNumVirtualAddressInfos,
    _Out_writes_to_opt_(*pNumVirtualAddressInfos, *pNumVirtualAddressInfos) D3D12DDI_DEBUG_VIRTUAL_ADDRESS_ALLOCATION_INFO_0012* pVirtualAddressInfos,
    _Inout_ UINT* pNumKMTInfos,
    _Out_writes_to_opt_(*pNumKMTInfos, *pNumKMTInfos) D3D12DDI_DEBUG_KMT_ALLOCATION_INFO_0014* pKMTInfos)
{
	DebugBreak();
}

D3D12DDI_DEVICE_FUNCS_CORE_0033 g_CosUmd12Device_Ddi_0033 =
{
    CosUmd12Device_Ddi_CheckFormatSupport,                             // pfnCheckFormatSupport
    CosUmd12Device_Ddi_CheckMultisampleQualityLevels,                  // pfnCheckMultisampleQualityLevels
    CosUmd12Device_Ddi_GetMipPacking,                                  // pfnGetMipPacking
    CosUmd12Device_Ddi_CalcPrivateElementLayoutSize_0010,              // pfnCalcPrivateElementLayoutSize
    CosUmd12Device_Ddi_CreateElementLayout_0010,                       // pfnCreateElementLayout
    CosUmd12Device_Ddi_DestroyElementLayout,                           // pfnDestroyElementLayout
    CosUmd12Device_Ddi_CalcPrivateBlendStateSize_0010,                 // pfnCalcPrivateBlendStateSize
    CosUmd12Device_Ddi_CreateBlendState_0010,                          // pfnCreateBlendState
    CosUmd12Device_Ddi_DestroyBlendState,                              // pfnDestroyBlendState
    CosUmd12Device_Ddi_CalcPrivateDepthStencilStateSize_0025,          // pfnCalcPrivateDepthStencilStateSize
    CosUmd12Device_Ddi_CreateDepthStencilState_0025,                   // pfnCreateDepthStencilState
    CosUmd12Device_Ddi_DestroyDepthStencilState,                       // pfnDestroyDepthStencilState
    CosUmd12Device_Ddi_CalcPrivateRasterizerStateSize_0010,            // pfnCalcPrivateRasterizerStateSize
    CosUmd12Device_Ddi_CreateRasterizerState_0010,                     // pfnCreateRasterizerState
    CosUmd12Device_Ddi_DestroyRasterizerState,                         // pfnDestroyRasterizerState
    CosUmd12Device_Ddi_CalcPrivateShaderSize_0026,                     // pfnCalcPrivateShaderSize
    CosUmd12Device_Ddi_CreateVertexShader_0026,                        // pfnCreateVertexShader
    CosUmd12Device_Ddi_CreatePixelShader_0026,                         // pfnCreatePixelShader
    CosUmd12Device_Ddi_CreateGeometryShader_0026,                      // pfnCreateGeometryShader
    CosUmd12Device_Ddi_CreateComputeShader_0026,                       // pfnCreateComputeShader
    CosUmd12Device_Ddi_CalcPrivateGeometryShaderWithStreamOutput_0026, // pfnCalcPrivateGeometryShaderWithStreamOutput
    CosUmd12Device_Ddi_CreateGeometryShaderWithStreamOutput_0026,      // pfnCreateGeometryShaderWithStreamOutput
    CosUmd12Device_Ddi_CalcPrivateTessellationShaderSize_0026,         // pfnCalcPrivateTessellationShaderSize
    CosUmd12Device_Ddi_CreateHullShader_0026,                          // pfnCreateHullShader
    CosUmd12Device_Ddi_CreateDomainShader_0026,                        // pfnCreateDomainShader
    CosUmd12Device_Ddi_DestroyShader,                                  // pfnDestroyShader
    CosUmd12Device_Ddi_CalcPrivateCommandQueueSize_0023,               // pfnCalcPrivateCommandQueueSize
    CosUmd12Device_Ddi_CreateCommandQueue_0023,                        // pfnCreateCommandQueue
    CosUmd12Device_Ddi_DestroyCommandQueue,                            // pfnDestroyCommandQueue
    CosUmd12Device_Ddi_CalcPrivateCommandAllocatorSize,                // pfnCalcPrivateCommandAllocatorSize
    CosUmd12Device_Ddi_CreateCommandAllocator,                         // pfnCreateCommandAllocator
    CosUmd12Device_Ddi_DestroyCommandAllocator,                        // pfnDestroyCommandAllocator
    CosUmd12Device_Ddi_ResetCommandAllocator,                          // pfnResetCommandAllocator
    CosUmd12Device_Ddi_CalcPrivatePipelineStateSize_0033,              // pfnCalcPrivatePipelineStateSize
    CosUmd12Device_Ddi_CreatePipelineState_0033,                       // pfnCreatePipelineState
    CosUmd12Device_Ddi_DestroyPipelineState,                           // pfnDestroyPipelineState
    CosUmd12Device_Ddi_CalcPrivateCommandListSize_0001,                // pfnCalcPrivateCommandListSize
    CosUmd12Device_Ddi_CreateCommandList_0001,                         // pfnCreateCommandList
    CosUmd12Device_Ddi_DestroyCommandList,                             // pfnDestroyCommandList
    CosUmd12Device_Ddi_CalcPrivateFenceSize,                           // pfnCalcPrivateFenceSize
    CosUmd12Device_Ddi_CreateFence,                                    // pfnCreateFence
    CosUmd12Device_Ddi_DestroyFence,                                   // pfnDestroyFence
    CosUmd12Device_Ddi_CalcPrivateDescriptorHeapSize_0001,             // pfnCalcPrivateDescriptorHeapSize
    CosUmd12Device_Ddi_CreateDescriptorHeap_0001,                      // pfnCreateDescriptorHeap
    CosUmd12Device_Ddi_DestroyDescriptorHeap,                          // pfnDestroyDescriptorHeap
    CosUmd12Device_Ddi_GetDescriptorSizeInBytes,                       // pfnGetDescriptorSizeInBytes
    CosUmd12Device_Ddi_GetCpuDescriptorHandleForHeapStart,             // pfnGetCPUDescriptorHandleForHeapStart
    CosUmd12Device_Ddi_GetGpuDescriptorHandleForHeapStart,             // pfnGetGPUDescriptorHandleForHeapStart
    CosUmd12Device_Ddi_CreateShaderResourceView_0002,                  // pfnCreateShaderResourceView
    CosUmd12Device_Ddi_CreateConstantBufferView,                       // pfnCreateConstantBufferView
    CosUmd12Device_Ddi_CreateSampler,                                  // pfnCreateSampler
    CosUmd12Device_Ddi_CreateUnorderedAccessView_0002,                 // pfnCreateUnorderedAccessView
    CosUmd12Device_Ddi_CreateRenderTargetView_0002,                    // pfnCreateRenderTargetView
    CosUmd12Device_Ddi_CreateDepthStencilView,                         // pfnCreateDepthStencilView
    CosUmd12Device_Ddi_CalcPrivateRootSignatureSize_0013,              // pfnCalcPrivateRootSignatureSize
    CosUmd12Device_Ddi_CreateRootSignature_0013,                       // pfnCreateRootSignature
    CosUmd12Device_Ddi_DestroyRootSignature,                           // pfnDestroyRootSignature
    CosUmd12Device_Ddi_MapHeap,                                        // pfnMapHeap
    CosUmd12Device_Ddi_UnmapHeap,                                      // pfnUnmapHeap
    CosUmd12Device_Ddi_CalcPrivateHeapAndResourceSizes_0030,           // pfnCalcPrivateHeapAndResourceSizes
    CosUmd12Device_Ddi_CreateHeapAndResource_0030,                     // pfnCreateHeapAndResource
    CosUmd12Device_Ddi_DestroyHeapAndResource,                         // pfnDestroyHeapAndResource
    CosUmd12Device_Ddi_MakeResident_0001,                              // pfnMakeResident
    CosUmd12Device_Ddi_Evict2,                                         // pfnEvict
    CosUmd12Device_Ddi_CalcPrivateOpenedHeapAndResourceSizes_0003,     // pfnCalcPrivateOpenedHeapAndResourceSizes
    CosUmd12Device_Ddi_OpenHeapAndResource_0003,                       // pfnOpenHeapAndResource
    CosUmd12Device_Ddi_CopyDescriptors_0003,                           // pfnCopyDescriptors
    CosUmd12Device_Ddi_CopyDescriptorsSimple_0003,                     // pfnCopyDescriptorsSimple
    CosUmd12Device_Ddi_CalcPrivateQueryHeapSize_0001,                  // pfnCalcPrivateQueryHeapSize
    CosUmd12Device_Ddi_CreateQueryHeap_0001,                           // pfnCreateQueryHeap
    CosUmd12Device_Ddi_DestroyQueryHeap,                               // pfnDestroyQueryHeap
    CosUmd12Device_Ddi_CalcPrivateCommandSignatureSize_0001,           // pfnCalcPrivateCommandSignatureSize
    CosUmd12Device_Ddi_CreateCommandSignature_0001,                    // pfnCreateCommandSignature
    CosUmd12Device_Ddi_DestroyCommandSignature,                        // pfnDestroyCommandSignature
    CosUmd12Device_Ddi_CheckResourceVirtualAddress,                    // pfnCheckResourceVirtualAddress
    CosUmd12Device_Ddi_CheckResourceAllocationInfo_0022,               // pfnCheckResourceAllocationInfo
    CosUmd12Device_Ddi_CheckSubresourceInfo,                           // pfnCheckSubresourceInfo
    CosUmd12Device_Ddi_CheckExistingResourceAllocationInfo_0022,       // pfnCheckExistingResourceAllocationInfo
    CosUmd12Device_Ddi_OfferResources,                                 // pfnOfferResources
    CosUmd12Device_Ddi_ReclaimResources_0001,                          // pfnReclaimResources
    CosUmd12Device_Ddi_GetImplicitPhysicalAdapterMask,                 // pfnGetImplicitPhysicalAdapterMask
    CosUmd12Device_Ddi_GetPresentPrivateDriverDataSize,                // pfnGetPresentPrivateDriverDataSize
    CosUmd12Device_Ddi_QueryNodeMap,                                   // pfnQueryNodeMap
    CosUmd12Device_Ddi_RetrieveShaderComment_0003,                     // pfnRetrieveShaderComment
    CosUmd12Device_Ddi_CheckResourceAllocationHandle,                  // pfnCheckResourceAllocationHandle
    CosUmd12Device_Ddi_CalcPrivatePipelineLibrarySize_0010,            // pfnCalcPrivatePipelineLibrarySize
    CosUmd12Device_Ddi_CreatePipelineLibrary_0010,                     // pfnCreatePipelineLibrary
    CosUmd12Device_Ddi_DestroyPipelineLibrary_0010,                    // pfnDestroyPipelineLibrary
    CosUmd12Device_Ddi_AddPipelineStateToLibrary_0010,                 // pfnAddPipelineStateToLibrary
    CosUmd12Device_Ddi_CalcSerializedLibrarySize_0010,                 // pfnCalcSerializedLibrarySize
    CosUmd12Device_Ddi_SerializeLibrary_0010,                          // pfnSerializeLibrary
    CosUmd12Device_Ddi_GetDebugAllocationInfo_0014                     // pfnGetDebugAllocationInfo
};
