#include "CosUmd12.h"

void APIENTRY CosUmd12Device_Ddi_CheckFormatSupport(
    D3D12DDI_HDEVICE Device,
    DXGI_FORMAT Format,
    _Out_ UINT* pFlags)
{
    // TODO: Review format support

    *pFlags = D3D10_DDI_FORMAT_SUPPORT_NOT_SUPPORTED;

    switch( Format )
    {
    case DXGI_FORMAT_R32G32B32_TYPELESS:
    case DXGI_FORMAT_R32G32B32_FLOAT:
    case DXGI_FORMAT_R32G32B32_UINT:
    case DXGI_FORMAT_R32G32B32_SINT:
    case DXGI_FORMAT_Y210:
    case DXGI_FORMAT_Y216:
    case DXGI_FORMAT_AYUV:
    case DXGI_FORMAT_P016:
    case DXGI_FORMAT_NV11:
        return;
    }

    const UINT SupportEverything = D3D10_DDI_FORMAT_SUPPORT_SHADER_SAMPLE |
        D3D10_DDI_FORMAT_SUPPORT_RENDERTARGET |
        D3D10_DDI_FORMAT_SUPPORT_BLENDABLE |
        D3D10_DDI_FORMAT_SUPPORT_MULTISAMPLE_RENDERTARGET |
        D3D10_DDI_FORMAT_SUPPORT_MULTISAMPLE_LOAD;

    const UINT SupportNoMSAA = D3D10_DDI_FORMAT_SUPPORT_SHADER_SAMPLE |
        D3D10_DDI_FORMAT_SUPPORT_RENDERTARGET |
        D3D10_DDI_FORMAT_SUPPORT_BLENDABLE;

    const UINT SupportNoMSAANoRT = D3D10_DDI_FORMAT_SUPPORT_SHADER_SAMPLE |
        D3D10_DDI_FORMAT_SUPPORT_BLENDABLE;

    const UINT SupportMSAALoad = SupportNoMSAANoRT | D3D10_DDI_FORMAT_SUPPORT_MULTISAMPLE_LOAD;

    const UINT SupportSampleOnly = D3D10_DDI_FORMAT_SUPPORT_SHADER_SAMPLE;

    switch( Format )
    {
    case DXGI_FORMAT_R32G32B32A32_TYPELESS:
    case DXGI_FORMAT_R32G32B32_TYPELESS:
    case DXGI_FORMAT_R16G16B16A16_TYPELESS:
        break;

    case DXGI_FORMAT_R32G32B32_FLOAT:
        *pFlags = SupportSampleOnly;
        break;

    case DXGI_FORMAT_R32G32B32_UINT:
    case DXGI_FORMAT_R32G32B32_SINT:
        *pFlags = SupportNoMSAA;
        break;

    case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
    case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
        *pFlags =SupportNoMSAANoRT;
        break;

    case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
    case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
    case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
    case DXGI_FORMAT_BC1_TYPELESS:
    case DXGI_FORMAT_BC1_UNORM:
    case DXGI_FORMAT_BC1_UNORM_SRGB:
    case DXGI_FORMAT_BC2_TYPELESS:
    case DXGI_FORMAT_BC2_UNORM:
    case DXGI_FORMAT_BC2_UNORM_SRGB:
    case DXGI_FORMAT_BC3_TYPELESS:
    case DXGI_FORMAT_BC3_UNORM:
    case DXGI_FORMAT_BC3_UNORM_SRGB:
    case DXGI_FORMAT_BC4_TYPELESS:
    case DXGI_FORMAT_BC4_UNORM:
    case DXGI_FORMAT_BC4_SNORM:
    case DXGI_FORMAT_BC5_TYPELESS:
    case DXGI_FORMAT_BC5_UNORM:
    case DXGI_FORMAT_BC5_SNORM:
    case DXGI_FORMAT_BC6H_TYPELESS:
    case DXGI_FORMAT_BC6H_UF16:
    case DXGI_FORMAT_BC6H_SF16:
    case DXGI_FORMAT_BC7_TYPELESS:
    case DXGI_FORMAT_BC7_UNORM:
    case DXGI_FORMAT_BC7_UNORM_SRGB:
    case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
    case DXGI_FORMAT_R8G8_B8G8_UNORM:
    case DXGI_FORMAT_G8R8_G8B8_UNORM:
    case DXGI_FORMAT_Y410:
    case DXGI_FORMAT_Y416:
    case DXGI_FORMAT_420_OPAQUE:
    case DXGI_FORMAT_YUY2:
    case DXGI_FORMAT_AI44:
    case DXGI_FORMAT_IA44:
    case DXGI_FORMAT_P8:
    case DXGI_FORMAT_A8P8:
    case DXGI_FORMAT_NV12:
    case DXGI_FORMAT_R1_UNORM:
        *pFlags = SupportSampleOnly;
        break;

    default:
        *pFlags = SupportEverything;
        break;
    }
}

void APIENTRY CosUmd12Device_Ddi_CheckMultisampleQualityLevels(
    D3D12DDI_HDEVICE Device,
    DXGI_FORMAT Format,
    UINT SampleCount,
    D3D12DDI_MULTISAMPLE_QUALITY_LEVEL_FLAGS Flags,
    _Out_ UINT* pNumQualityLevels)
{
    UNEXPECTED_DDI();

    *pNumQualityLevels = 0;
}

void APIENTRY CosUmd12Device_Ddi_GetMipPacking(
    D3D12DDI_HDEVICE Device,
    D3D12DDI_HRESOURCE TiledResource,
    _Out_ UINT* pNumPackedMips,
    _Out_ UINT* pNumTilesForPackedMips)
{
    UNEXPECTED_DDI();
}

SIZE_T APIENTRY CosUmd12Device_Ddi_CalcPrivateElementLayoutSize_0010(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDIARG_CREATEELEMENTLAYOUT_0010* pDesc)
{
    UNEXPECTED_DDI();

    return 0;
}

void APIENTRY CosUmd12Device_Ddi_CreateElementLayout_0010(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDIARG_CREATEELEMENTLAYOUT_0010* pDesc,
    D3D12DDI_HELEMENTLAYOUT ElementLayout)
{
    UNEXPECTED_DDI();
}

void APIENTRY CosUmd12Device_Ddi_DestroyElementLayout(
    D3D12DDI_HDEVICE Device, 
    D3D12DDI_HELEMENTLAYOUT ElementLayout)
{
    UNEXPECTED_DDI();
}

SIZE_T APIENTRY CosUmd12Device_Ddi_CalcPrivateBlendStateSize_0010(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDI_BLEND_DESC_0010* pDesc)
{
    UNEXPECTED_DDI();

    return 0;
}

void APIENTRY CosUmd12Device_Ddi_CreateBlendState_0010(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDI_BLEND_DESC_0010* pDesc,
    D3D12DDI_HBLENDSTATE BlendState)
{
    UNEXPECTED_DDI();
}

void APIENTRY CosUmd12Device_Ddi_DestroyBlendState(
    D3D12DDI_HDEVICE Device, 
    D3D12DDI_HBLENDSTATE BlendState)
{
    UNEXPECTED_DDI();
}

SIZE_T APIENTRY CosUmd12Device_Ddi_CalcPrivateDepthStencilStateSize_0025(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDI_DEPTH_STENCIL_DESC_0025* pDesc)
{
    UNEXPECTED_DDI();

    return 0;
}

void APIENTRY CosUmd12Device_Ddi_CreateDepthStencilState_0025(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDI_DEPTH_STENCIL_DESC_0025* pDesc,
    D3D12DDI_HDEPTHSTENCILSTATE DepthStencilState)
{
    UNEXPECTED_DDI();
}

void APIENTRY CosUmd12Device_Ddi_DestroyDepthStencilState(
    D3D12DDI_HDEVICE Device,
    D3D12DDI_HDEPTHSTENCILSTATE DepthStencilState)
{
    UNEXPECTED_DDI();
}

SIZE_T APIENTRY CosUmd12Device_Ddi_CalcPrivateRasterizerStateSize_0010(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDI_RASTERIZER_DESC_0010* pDesc)
{
    UNEXPECTED_DDI();

    return 0;
}

void APIENTRY CosUmd12Device_Ddi_CreateRasterizerState_0010(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDI_RASTERIZER_DESC_0010* pDesc,
    D3D12DDI_HRASTERIZERSTATE RasterizerState)
{
    UNEXPECTED_DDI();
}

void APIENTRY CosUmd12Device_Ddi_DestroyRasterizerState(
    D3D12DDI_HDEVICE Device, 
    D3D12DDI_HRASTERIZERSTATE RasterizerState)
{
    UNEXPECTED_DDI();
}

SIZE_T APIENTRY CosUmd12Device_Ddi_CalcPrivateShaderSize_0026(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDIARG_CREATE_SHADER_0026* pDesc)
{
    TRACE_FUNCTION();

    return CosUmd12Shader::CalculateSize(pDesc);
}

void APIENTRY CosUmd12Device_Ddi_CreateVertexShader_0026(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDIARG_CREATE_SHADER_0026* pDesc,
    D3D12DDI_HSHADER Shader)
{
    UNEXPECTED_DDI();
}

void APIENTRY CosUmd12Device_Ddi_CreatePixelShader_0026(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDIARG_CREATE_SHADER_0026* pDesc,
    D3D12DDI_HSHADER Shader)
{
    UNEXPECTED_DDI();
}

void APIENTRY CosUmd12Device_Ddi_CreateGeometryShader_0026(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDIARG_CREATE_SHADER_0026* pDesc,
    D3D12DDI_HSHADER Shader)
{
    UNEXPECTED_DDI();
}

void APIENTRY CosUmd12Device_Ddi_CreateComputeShader_0026(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDIARG_CREATE_SHADER_0026* pDesc,
    D3D12DDI_HSHADER Shader)
{
    TRACE_FUNCTION();

    CosUmd12Device * pDevice = CosUmd12Device::CastFrom(Device);
    CosUmd12Shader * pShader = new (Shader.pDrvPrivate) CosUmd12Shader(pDevice, pDesc);
}

SIZE_T APIENTRY CosUmd12Device_Ddi_CalcPrivateGeometryShaderWithStreamOutput_0026(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDIARG_CREATE_GEOMETRY_SHADER_WITH_STREAM_OUTPUT_0026* pDesc)
{
    UNEXPECTED_DDI();

    return 0;
}

void APIENTRY CosUmd12Device_Ddi_CreateGeometryShaderWithStreamOutput_0026(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDIARG_CREATE_GEOMETRY_SHADER_WITH_STREAM_OUTPUT_0026* pDesc,
    D3D12DDI_HSHADER Shader)
{
    UNEXPECTED_DDI();
}

SIZE_T APIENTRY CosUmd12Device_Ddi_CalcPrivateTessellationShaderSize_0026(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDIARG_CREATE_SHADER_0026* pDesc)
{
    UNEXPECTED_DDI();

    return 0;
}

void APIENTRY CosUmd12Device_Ddi_CreateHullShader_0026(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDIARG_CREATE_SHADER_0026* pDesc,
    D3D12DDI_HSHADER Shader)
{
    UNEXPECTED_DDI();
}

void APIENTRY CosUmd12Device_Ddi_CreateDomainShader_0026(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDIARG_CREATE_SHADER_0026* pDesc,
    D3D12DDI_HSHADER Shader)
{
    UNEXPECTED_DDI();
}

void APIENTRY CosUmd12Device_Ddi_DestroyShader(
    D3D12DDI_HDEVICE Device, 
    D3D12DDI_HSHADER Shader)
{
    TRACE_FUNCTION();

    CosUmd12Device * pDevice = CosUmd12Device::CastFrom(Device);
    CosUmd12Shader * pShader = CosUmd12Shader::CastFrom(Shader);

    pShader->~CosUmd12Shader();
}

SIZE_T APIENTRY CosUmd12Device_Ddi_CalcPrivateCommandQueueSize_0050(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDIARG_CREATECOMMANDQUEUE_0050* pDesc)
{
    return CosUmd12CommandQueue::CalculateSize(pDesc);
}

HRESULT APIENTRY CosUmd12Device_Ddi_CreateCommandQueue_0050(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDIARG_CREATECOMMANDQUEUE_0050* pDesc,
    D3D12DDI_HCOMMANDQUEUE DrvCommandQueue,
    D3D12DDI_HRTCOMMANDQUEUE RTCommandQueue)
{
    TRACE_FUNCTION();

    CosUmd12Device * pDevice = CosUmd12Device::CastFrom(Device);

    CosUmd12CommandQueue * pCommandQueue = new (DrvCommandQueue.pDrvPrivate) CosUmd12CommandQueue(pDevice, RTCommandQueue, pDesc);

    return pCommandQueue->Standup();
}

void APIENTRY CosUmd12Device_Ddi_DestroyCommandQueue(
    D3D12DDI_HDEVICE Device,
    D3D12DDI_HCOMMANDQUEUE CommandQueue)
{
    TRACE_FUNCTION();

    CosUmd12Device * pDevice = CosUmd12Device::CastFrom(Device);
    CosUmd12CommandQueue * pCommandQueue = CosUmd12CommandQueue::CastFrom(CommandQueue);

    pCommandQueue->~CosUmd12CommandQueue();
}

SIZE_T APIENTRY CosUmd12Device_Ddi_CalcPrivateCommandPoolSize_0040(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDIARG_CREATE_COMMAND_POOL_0040* pDesc)
{
    TRACE_FUNCTION();

    return CosUmd12CommandPool::CalculateSize(pDesc);
}

HRESULT APIENTRY CosUmd12Device_Ddi_CreateCommandPool_0040(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDIARG_CREATE_COMMAND_POOL_0040* pDesc,
    D3D12DDI_HCOMMANDPOOL_0040 CommandPool)
{
    TRACE_FUNCTION();

    CosUmd12Device * pDevice = CosUmd12Device::CastFrom(Device);
    CosUmd12CommandPool * pCommandQueue = new (CommandPool.pDrvPrivate) CosUmd12CommandPool(pDevice, pDesc);

    return S_OK;
}

void APIENTRY CosUmd12Device_Ddi_DestroyCommandPool_0040(
    D3D12DDI_HDEVICE Device,
    D3D12DDI_HCOMMANDPOOL_0040 CommandPool)
{
    TRACE_FUNCTION();

    CosUmd12Device * pDevice = CosUmd12Device::CastFrom(Device);
    CosUmd12CommandPool * pCommandPool = CosUmd12CommandPool::CastFrom(CommandPool);

    pCommandPool->~CosUmd12CommandPool();
}

void APIENTRY CosUmd12Device_Ddi_ResetCommandPool_0040(
    D3D12DDI_HDEVICE Device,
    D3D12DDI_HCOMMANDPOOL_0040 CommandPool)
{
    TRACE_FUNCTION();

    CosUmd12CommandPool * pCommandPool = CosUmd12CommandPool::CastFrom(CommandPool);
    pCommandPool->Reset();
}

SIZE_T APIENTRY CosUmd12Device_Ddi_CalcPrivatePipelineStateSize_0033(
    D3D12DDI_HDEVICE Device, 
    _In_ const D3D12DDIARG_CREATE_PIPELINE_STATE_0033* pDesc)
{
    TRACE_FUNCTION();

    return CosUmd12PipelineState::CalculateSize(pDesc);
}

HRESULT APIENTRY CosUmd12Device_Ddi_CreatePipelineState_0033(
    D3D12DDI_HDEVICE Device, 
    _In_ const D3D12DDIARG_CREATE_PIPELINE_STATE_0033* pDesc, 
    D3D12DDI_HPIPELINESTATE PipelineState,
    D3D12DDI_HRTPIPELINESTATE RTPipelineState)
{
    TRACE_FUNCTION();

    CosUmd12Device * pDevice = CosUmd12Device::CastFrom(Device);
    CosUmd12PipelineState * pPipelineState = new (PipelineState.pDrvPrivate) CosUmd12PipelineState(pDevice, RTPipelineState, pDesc);

    return S_OK;
}

VOID APIENTRY CosUmd12Device_Ddi_DestroyPipelineState(
    D3D12DDI_HDEVICE Device, 
    D3D12DDI_HPIPELINESTATE PipelineState)
{
    TRACE_FUNCTION();

    CosUmd12Device * pDevice = CosUmd12Device::CastFrom(Device);
    CosUmd12PipelineState * pPipelineState = CosUmd12PipelineState::CastFrom(PipelineState);

    pPipelineState->~CosUmd12PipelineState();
}

SIZE_T APIENTRY CosUmd12Device_Ddi_CalcPrivateCommandListSize_0040(
    D3D12DDI_HDEVICE Device, 
    _In_ const D3D12DDIARG_CREATE_COMMAND_LIST_0040* pDesc)
{
    TRACE_FUNCTION();

    return CosUmd12CommandList::CalculateSize(pDesc);
}

HRESULT APIENTRY CosUmd12Device_Ddi_CreateCommandList_0040(
    D3D12DDI_HDEVICE Device, 
    _In_ const D3D12DDIARG_CREATE_COMMAND_LIST_0040* pDesc,
    D3D12DDI_HCOMMANDLIST CommandList,
    D3D12DDI_HRTCOMMANDLIST RtCommandList)
{
    TRACE_FUNCTION();

    CosUmd12Device * pDevice = CosUmd12Device::CastFrom(Device);

    CosUmd12CommandList * pCommandList = new (CommandList.pDrvPrivate) CosUmd12CommandList(pDevice, pDesc, RtCommandList);

    HRESULT hr = pCommandList->StandUp();

    if (hr != S_OK) pCommandList->~CosUmd12CommandList();

    return hr;
}

void APIENTRY CosUmd12Device_Ddi_DestroyCommandList(
    D3D12DDI_HDEVICE Device, 
    D3D12DDI_HCOMMANDLIST CommandList)
{
    TRACE_FUNCTION();

    CosUmd12Device * pDevice = CosUmd12Device::CastFrom(Device);
    CosUmd12CommandList * pCommandList = CosUmd12CommandList::CastFrom(CommandList);

    pCommandList->~CosUmd12CommandList();
}

SIZE_T APIENTRY CosUmd12Device_Ddi_CalcPrivateFenceSize(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDIARG_CREATE_FENCE* pDesc)
{
    TRACE_FUNCTION();

    return CosUmd12Fence::CalculateSize(pDesc);
}

HRESULT APIENTRY CosUmd12Device_Ddi_CreateFence(
    D3D12DDI_HDEVICE Device,
    D3D12DDI_HFENCE Fence,
    _In_ const D3D12DDIARG_CREATE_FENCE* pDesc)
{
    TRACE_FUNCTION();

    CosUmd12Device * pDevice = CosUmd12Device::CastFrom(Device);
    CosUmd12Fence * pFence = new (Fence.pDrvPrivate) CosUmd12Fence(pDevice, pDesc);

    return S_OK;
}

void APIENTRY CosUmd12Device_Ddi_DestroyFence(
    D3D12DDI_HDEVICE Device,
    D3D12DDI_HFENCE Fence)
{
    TRACE_FUNCTION();

    CosUmd12Device * pDevice = CosUmd12Device::CastFrom(Device);
    CosUmd12Fence * pFence = CosUmd12Fence::CastFrom(Fence);

    pFence->~CosUmd12Fence();
}

SIZE_T APIENTRY CosUmd12Device_Ddi_CalcPrivateDescriptorHeapSize_0001(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDIARG_CREATE_DESCRIPTOR_HEAP_0001* pDesc)
{
    TRACE_FUNCTION();

    return CosUmd12DescriptorHeap::CalculateSize(pDesc);
}

HRESULT APIENTRY CosUmd12Device_Ddi_CreateDescriptorHeap_0001(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDIARG_CREATE_DESCRIPTOR_HEAP_0001* pDesc,
    D3D12DDI_HDESCRIPTORHEAP DescriptorHeap)
{
    TRACE_FUNCTION();

    CosUmd12Device * pDevice = CosUmd12Device::CastFrom(Device);

    return CosUmd12DescriptorHeap::Create(pDevice, pDesc, DescriptorHeap);
}

void APIENTRY CosUmd12Device_Ddi_DestroyDescriptorHeap(
    D3D12DDI_HDEVICE Device,
    D3D12DDI_HDESCRIPTORHEAP DescriptorHeap)
{
    TRACE_FUNCTION();
}

UINT APIENTRY CosUmd12Device_Ddi_GetDescriptorSizeInBytes(
    D3D12DDI_HDEVICE Device,
    D3D12DDI_DESCRIPTOR_HEAP_TYPE HeapType)
{
    TRACE_FUNCTION();

    UINT size = 0;

    switch (HeapType) {
    case D3D12DDI_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
    case D3D12DDI_DESCRIPTOR_HEAP_TYPE_SAMPLER:
    case D3D12DDI_DESCRIPTOR_HEAP_TYPE_RTV:
    case D3D12DDI_DESCRIPTOR_HEAP_TYPE_DSV:
        return sizeof(CosUmd12Descriptor);

    default:
        ASSERT(0);
    }

    return size;
}

D3D12DDI_CPU_DESCRIPTOR_HANDLE APIENTRY CosUmd12Device_Ddi_GetCpuDescriptorHandleForHeapStart(
    D3D12DDI_HDEVICE Device, 
    D3D12DDI_HDESCRIPTORHEAP DescriptorHeap)
{
    TRACE_FUNCTION();

    CosUmd12DescriptorHeap * pDescriptorHeap = CosUmd12DescriptorHeap::CastFrom(DescriptorHeap);

    return { (SIZE_T)pDescriptorHeap->GetCpuAddress() };
}

D3D12DDI_GPU_DESCRIPTOR_HANDLE APIENTRY CosUmd12Device_Ddi_GetGpuDescriptorHandleForHeapStart(
    D3D12DDI_HDEVICE Device,
    D3D12DDI_HDESCRIPTORHEAP DescriptorHeap)
{
    TRACE_FUNCTION();

    CosUmd12DescriptorHeap * pDescriptorHeap = CosUmd12DescriptorHeap::CastFrom(DescriptorHeap);

    return { pDescriptorHeap->GetGpuAddress() };
}

void APIENTRY CosUmd12Device_Ddi_CreateShaderResourceView_0002(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDIARG_CREATE_SHADER_RESOURCE_VIEW_0002* pDesc,
    _In_ D3D12DDI_CPU_DESCRIPTOR_HANDLE DestDescriptor)
{
    UNEXPECTED_DDI();
}

void APIENTRY CosUmd12Device_Ddi_CreateConstantBufferView(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDI_CONSTANT_BUFFER_VIEW_DESC* pDesc,
    _In_ D3D12DDI_CPU_DESCRIPTOR_HANDLE DestDescriptor)
{
    TRACE_FUNCTION();

    //
    // TODO : Test
    //

    CosUmd12Descriptor *pUavDescriptor = new ((void *)DestDescriptor.ptr) CosUmd12Descriptor(pDesc);
}

void APIENTRY CosUmd12Device_Ddi_CreateSampler(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDIARG_CREATE_SAMPLER* pDesc,
    _In_ D3D12DDI_CPU_DESCRIPTOR_HANDLE DestDescriptor)
{
    UNEXPECTED_DDI();
}

void APIENTRY CosUmd12Device_Ddi_CreateUnorderedAccessView_0002(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDIARG_CREATE_UNORDERED_ACCESS_VIEW_0002* pDesc,
    _In_ D3D12DDI_CPU_DESCRIPTOR_HANDLE DestDescriptor)
{
    TRACE_FUNCTION();

    CosUmd12Descriptor *pUavDescriptor = new ((void *)DestDescriptor.ptr) CosUmd12Descriptor(pDesc);
}

void APIENTRY CosUmd12Device_Ddi_CreateRenderTargetView_0002(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDIARG_CREATE_RENDER_TARGET_VIEW_0002* pDesc,
    _In_ D3D12DDI_CPU_DESCRIPTOR_HANDLE DestDescriptor)
{
    UNEXPECTED_DDI();
}

void APIENTRY CosUmd12Device_Ddi_CreateDepthStencilView(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDIARG_CREATE_DEPTH_STENCIL_VIEW* pDesc,
    _In_ D3D12DDI_CPU_DESCRIPTOR_HANDLE DestDescriptor)
{
    UNEXPECTED_DDI();
}


SIZE_T APIENTRY CosUmd12Device_Ddi_CalcPrivateRootSignatureSize_0013(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDIARG_CREATE_ROOT_SIGNATURE_0013* pDesc)
{
    TRACE_FUNCTION();

    return CosUmd12RootSignature::CalculateSize(pDesc);
}

HRESULT APIENTRY CosUmd12Device_Ddi_CreateRootSignature_0013(
    D3D12DDI_HDEVICE hDevice,
    _In_ const D3D12DDIARG_CREATE_ROOT_SIGNATURE_0013* pDesc,
    D3D12DDI_HROOTSIGNATURE hRootSignature)
{
    TRACE_FUNCTION();

    CosUmd12Device * pDevice = CosUmd12Device::CastFrom(hDevice);
    CosUmd12RootSignature * pCosUmdRootSignature = new (hRootSignature.pDrvPrivate) CosUmd12RootSignature(pDevice, pDesc);

    return S_OK;
}

void APIENTRY CosUmd12Device_Ddi_DestroyRootSignature(
    D3D12DDI_HDEVICE Device,
    D3D12DDI_HROOTSIGNATURE RootSignature)
{
    TRACE_FUNCTION();
}

HRESULT APIENTRY CosUmd12Device_Ddi_MapHeap(
    D3D12DDI_HDEVICE Device,
    D3D12DDI_HHEAP Heap,
    _Out_ void** pHeapData)
{
    TRACE_FUNCTION();

    CosUmd12Heap * pHeap = (CosUmd12Heap *)Heap.pDrvPrivate;

    return pHeap->Map(pHeapData);
}

void APIENTRY CosUmd12Device_Ddi_UnmapHeap(
    D3D12DDI_HDEVICE Device,
    D3D12DDI_HHEAP Heap)
{
    TRACE_FUNCTION();

    CosUmd12Heap * pHeap = (CosUmd12Heap *)Heap.pDrvPrivate;

    pHeap->Unmap();
}

D3D12DDI_HEAP_AND_RESOURCE_SIZES APIENTRY CosUmd12Device_Ddi_CalcPrivateHeapAndResourceSizes_0030(
    D3D12DDI_HDEVICE Device,
    _In_opt_ const D3D12DDIARG_CREATEHEAP_0001* pHeapDesc,
    _In_opt_ const D3D12DDIARG_CREATERESOURCE_0003* pResourceDesc,
    D3D12DDI_HPROTECTEDRESOURCESESSION_0030 hProtectedResourceSession)
{
    TRACE_FUNCTION();

    D3D12DDI_HEAP_AND_RESOURCE_SIZES sizes;

    sizes.Heap = CosUmd12Heap::CalculateSize();
    sizes.Resource = CosUmd12Resource::CalculateSize();

    return sizes;
}

//
// There are 3 usage cases for CreateHeapAndResource:
//
// 1. Create both a heap and resource (from API CreateCommittedResource)
// 2. Create just a heap (from API CreateHeap)
// 3. Create a resouce in an existing heap (from API CreatePlacedResource)
//
// #1 is the common usage case by ML runtime
//

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
    CosUmd12Device * pDevice = CosUmd12Device::CastFrom(Device);

    //
    // Compute only driver supports only BUFFER
    //

    if (pResourceDesc->ResourceType != D3D12DDI_RT_BUFFER)
    {
        return DXGI_ERROR_UNSUPPORTED;
    }

    CosUmd12Heap * pHeap = NULL;
    
    if (pHeapDesc != NULL) {
        TRACE_FUNCTION();
        pHeap = new (Heap.pDrvPrivate) CosUmd12Heap(pDevice, RtHeap, pHeapDesc);

        pHeap->Standup();
    }
    else
    {
        // TODO : Test

        pHeap = CosUmd12Heap::CastFrom(Heap);
    }

    if (pResourceDesc != NULL) {
        TRACE_FUNCTION();
        CosUmd12Resource * pResource = new (Resource.pDrvPrivate) CosUmd12Resource(pDevice, pResourceDesc);

        pResource->Standup(pHeap);
    }

    return S_OK;
}

void APIENTRY CosUmd12Device_Ddi_DestroyHeapAndResource(
    D3D12DDI_HDEVICE Device,
    D3D12DDI_HHEAP Heap,
    D3D12DDI_HRESOURCE Resource)
{
    TRACE_FUNCTION();

    CosUmd12Device * pDevice = CosUmd12Device::CastFrom(Device);
    CosUmd12Heap * pHeap = CosUmd12Heap::CastFrom(Heap);
    CosUmd12Resource * pResource = CosUmd12Resource::CastFrom(Resource);

    if (pHeap != NULL)
    {
        pHeap->~CosUmd12Heap();
    }

    if (pResource != NULL)
    {
        pResource->~CosUmd12Resource();
    }
}

HRESULT APIENTRY CosUmd12Device_Ddi_MakeResident_0001(
    D3D12DDI_HDEVICE Device,
    D3D12DDIARG_MAKERESIDENT_0001* pDesc)
{
    TRACE_FUNCTION();

    CosUmd12Device * pDevice = CosUmd12Device::CastFrom(Device);

    ASSERT(pDesc->NumAdapters == 1);

    UINT numAllocations = pDesc->NumObjects;

    D3DKMT_HANDLE* pAllocations = (D3DKMT_HANDLE*)malloc(sizeof(D3DKMT_HANDLE) * numAllocations);
    if (pAllocations == NULL) return E_OUTOFMEMORY;

    D3DDDI_MAKERESIDENT makeResident = {};

    makeResident.hPagingQueue = 0;
    makeResident.Flags = pDesc->Flags;
    makeResident.AllocationList = pAllocations;
    makeResident.PriorityList = nullptr;

    HRESULT hr = S_OK;

    UINT makeResidentCount = 0;

    for (UINT i = 0; i < numAllocations; ++i)
    {
        switch (pDesc->pObjects[i].Type)
        {
            case D3D12DDI_HT_HEAP:
            {
                D3D12DDI_HHEAP hHeap;
                hHeap.pDrvPrivate = pDesc->pObjects[i].Handle;

                CosUmd12Heap* pHeap = CosUmd12Heap::CastFrom(hHeap);

                if (pHeap->GetAllocationHandle())
                {
                    pAllocations[makeResidentCount++] = pHeap->GetAllocationHandle();
                }
                break;
            }
            case D3D12DDI_HT_DESCRIPTOR_HEAP:
            {
                TRACE_FUNCTION();
#if 0
                D3D12DDI_HDESCRIPTORHEAP hDescriptorHeap;
                hDescriptorHeap.pDrvPrivate = pDesc->pObjects[i].Handle;
                CDescriptorHeap* pDescriptorHeap = Promote(hDescriptorHeap);
                pAllocations[makeResidentCount++] = pDescriptorHeap->m_DescriptorBuffer.GetHandle();
#endif
                break;
            }
            case D3D12DDI_HT_QUERY_HEAP:
            {
                TRACE_FUNCTION();
#if 0
                D3D12DDI_HQUERYHEAP hQueryHeap;
                hQueryHeap.pDrvPrivate = pDesc->pObjects[i].Handle;
                CQueryHeap* pQueryHeap = Promote(hQueryHeap);
                DBGASSERT(pQueryHeap->m_Allocation.m_CpuMapCount == 0);
                pAllocations[makeResidentCount++] = pQueryHeap->m_Allocation.GetHandle();
#endif
                break;
            }

            case D3D12DDI_HT_0012_RESOURCE:
            case D3D12DDI_HT_PIPELINE_STATE:
            case D3D12DDI_HT_COMMAND_ALLOCATOR:
            case D3D12DDI_HT_COMMAND_QUEUE:
            case D3D12DDI_HT_FENCE:
            case D3D12DDI_HT_COMMAND_SIGNATURE:
            default:
                ASSERT(0);
                hr = E_INVALIDARG;
                break;
        }
    }

    if (SUCCEEDED(hr))
    {
        if (makeResidentCount == 0)
        {
            *pDesc->pPagingFenceValue = 0;
            pDesc->WaitMask = 0;
        }
        else
        {
            makeResident.NumAllocations = makeResidentCount;

            hr = pDevice->m_pUMCallbacks->pfnMakeResidentCb(pDevice->m_hRTDevice, *pDesc->pRTPagingQueue, &makeResident);
            if ((hr == E_PENDING) || SUCCEEDED(hr))
            {
                *pDesc->pPagingFenceValue = makeResident.PagingFenceValue;
                pDesc->WaitMask = (hr == E_PENDING) ? 1 : 0;
            }
        }
    }

    return hr;
}

HRESULT APIENTRY CosUmd12Device_Ddi_Evict2(
    D3D12DDI_HDEVICE Device,
    const D3D12DDIARG_EVICT* pDesc)
{
    TRACE_FUNCTION();

    CosUmd12Device * pDevice = CosUmd12Device::CastFrom(Device);

    UINT numAllocations = pDesc->NumObjects;

    D3DKMT_HANDLE* pAllocations = (D3DKMT_HANDLE*)malloc(sizeof(D3DKMT_HANDLE) * numAllocations);
    if (pAllocations == NULL) return E_OUTOFMEMORY;

    D3DDDICB_EVICT evict = {};

    evict.Flags = pDesc->Flags;

#if 0

    //
    // For testing KMD paging operation, override EvictOnlyIfNecessary with 0
    //

    evict.Flags.EvictOnlyIfNecessary = 0;

#endif

    evict.AllocationList = pAllocations;

    HRESULT hr = S_OK;

    UINT evictCount = 0;

    for (UINT i = 0; i < numAllocations; ++i)
    {
        switch (pDesc->pObjects[i].Type)
        {
        case D3D12DDI_HT_HEAP:
        {
            D3D12DDI_HHEAP hHeap;
            hHeap.pDrvPrivate = pDesc->pObjects[i].Handle;

            CosUmd12Heap* pHeap = CosUmd12Heap::CastFrom(hHeap);

            if (pHeap->GetAllocationHandle())
            {
                pAllocations[evictCount++] = pHeap->GetAllocationHandle();
            }
            break;
        }
        case D3D12DDI_HT_DESCRIPTOR_HEAP:
        {
            TRACE_FUNCTION();
#if 0
            D3D12DDI_HDESCRIPTORHEAP hDescriptorHeap;
            hDescriptorHeap.pDrvPrivate = pDesc->pObjects[i].Handle;
            CDescriptorHeap* pDescriptorHeap = Promote(hDescriptorHeap);
            pAllocations[evictCount++] = pDescriptorHeap->m_DescriptorBuffer.GetHandle();
#endif
            break;
        }
        case D3D12DDI_HT_QUERY_HEAP:
        {
            TRACE_FUNCTION();
#if 0
            D3D12DDI_HQUERYHEAP hQueryHeap;
            hQueryHeap.pDrvPrivate = pDesc->pObjects[i].Handle;
            CQueryHeap* pQueryHeap = Promote(hQueryHeap);
            DBGASSERT(pQueryHeap->m_Allocation.m_CpuMapCount == 0);
            pAllocations[evictCount++] = pQueryHeap->m_Allocation.GetHandle();
#endif
            break;
        }

        case D3D12DDI_HT_0012_RESOURCE:
        case D3D12DDI_HT_PIPELINE_STATE:
        case D3D12DDI_HT_COMMAND_ALLOCATOR:
        case D3D12DDI_HT_COMMAND_QUEUE:
        case D3D12DDI_HT_FENCE:
        case D3D12DDI_HT_COMMAND_SIGNATURE:
        default:
            ASSERT(0);
            hr = E_INVALIDARG;
            break;
        }
    }

    if (SUCCEEDED(hr))
    {
        if (evictCount)
        {
            evict.NumAllocations = evictCount;

            hr = pDevice->m_pUMCallbacks->pfnEvictCb(pDevice->m_hRTDevice, &evict);
        }
    }

    return hr;
}

D3D12DDI_HEAP_AND_RESOURCE_SIZES APIENTRY CosUmd12Device_Ddi_CalcPrivateOpenedHeapAndResourceSizes_0043(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDIARG_OPENHEAP_0003* pDesc,
    D3D12DDI_HPROTECTEDRESOURCESESSION_0030 ProtectedResourceSession)
{
    TRACE_FUNCTION();

    D3D12DDI_HEAP_AND_RESOURCE_SIZES sizes;

    sizes.Heap = CosUmd12Heap::CalculateSize();
    sizes.Resource = CosUmd12Resource::CalculateSize();

    return sizes;
}

HRESULT APIENTRY CosUmd12Device_Ddi_OpenHeapAndResource_0043(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDIARG_OPENHEAP_0003* pDesc,
    D3D12DDI_HHEAP Heap,
    D3D12DDI_HRTRESOURCE RtResource,
    D3D12DDI_HPROTECTEDRESOURCESESSION_0030 ProtectedResourceSession,
    D3D12DDI_HRESOURCE Resource)
{
    TRACE_FUNCTION();

#if 1

    //
    // Return failure before Ddi_OpenHeapAndResouce is properly implemented
    //
    // E_OUTOFMEMORY is allowed and doesn't trigger device removal
    //

    return E_OUTOFMEMORY;

#else

    CosUmd12Device * pDevice = CosUmd12Device::CastFrom(Device);
    CosUmd12Heap * pHeap = new (Heap.pDrvPrivate) CosUmd12Heap(pDevice);
    CosUmd12Resource * pResource = new (Resource.pDrvPrivate) CosUmd12Resource(pDevice, RtResource);

    if (pDesc->NumAllocations == 1)
    {
        ASSERT(pDesc->pOpenAllocationInfo->PrivateDriverDataSize == sizeof(CosAllocationExchange));
        CosAllocationExchange * pAllocation = (CosAllocationExchange *)pDesc->pOpenAllocationInfo->pPrivateDriverData;
        ASSERT(pAllocation->m_magic == CosAllocationExchange::kMagic);

        pResource->Initialize(pAllocation);
    }
    else
    {
        TRACE_FUNCTION();
        return E_NOTIMPL;
    }

    return S_OK;

#endif
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
    TRACE_FUNCTION();
}

void APIENTRY CosUmd12Device_Ddi_CopyDescriptorsSimple_0003(
    D3D12DDI_HDEVICE,
    _In_ UINT NumDescriptors,
    _In_ D3D12DDI_CPU_DESCRIPTOR_HANDLE DestDescriptorRangeStart,
    _In_ D3D12DDI_CPU_DESCRIPTOR_HANDLE SrcDescriptorRangeStart,
    _In_ D3D12DDI_DESCRIPTOR_HEAP_TYPE DescriptorHeapsType)
{
    TRACE_FUNCTION();
}

SIZE_T APIENTRY CosUmd12Device_Ddi_CalcPrivateQueryHeapSize_0001(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDIARG_CREATE_QUERY_HEAP_0001* pDesc)
{
    TRACE_FUNCTION();

    return 0;
}

HRESULT APIENTRY CosUmd12Device_Ddi_CreateQueryHeap_0001(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDIARG_CREATE_QUERY_HEAP_0001* pDesc,
    D3D12DDI_HQUERYHEAP QueryHeap)
{
    TRACE_FUNCTION();

    return E_NOTIMPL;
}

void APIENTRY CosUmd12Device_Ddi_DestroyQueryHeap(
    D3D12DDI_HDEVICE Device,
    D3D12DDI_HQUERYHEAP QueryHeap)
{
    TRACE_FUNCTION();
}

SIZE_T APIENTRY CosUmd12Device_Ddi_CalcPrivateCommandSignatureSize_0001(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDIARG_CREATE_COMMAND_SIGNATURE_0001* pDesc)
{
    UNEXPECTED_DDI();

    return 0;
}

HRESULT APIENTRY CosUmd12Device_Ddi_CreateCommandSignature_0001(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDIARG_CREATE_COMMAND_SIGNATURE_0001* pDesc,
    D3D12DDI_HCOMMANDSIGNATURE CommandSignature)
{
    UNEXPECTED_DDI();

    return E_NOTIMPL;
}

void APIENTRY CosUmd12Device_Ddi_DestroyCommandSignature(
    D3D12DDI_HDEVICE Device, 
    D3D12DDI_HCOMMANDSIGNATURE CommandSignature)
{
    UNEXPECTED_DDI();
}


D3D12DDI_GPU_VIRTUAL_ADDRESS APIENTRY CosUmd12Device_Ddi_CheckResourceVirtualAddress(
    D3D12DDI_HDEVICE Device,
    D3D12DDI_HRESOURCE Resource)
{
    TRACE_FUNCTION();

    CosUmd12Resource * pResource = (CosUmd12Resource *)Resource.pDrvPrivate;

#if COS_GPUVA_SUPPORT

    return pResource->GetGpuVa();

#else

    return pResource->GetUniqueAddress();

#endif
}

void APIENTRY CosUmd12Device_Ddi_CheckResourceAllocationInfo_0022(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDIARG_CREATERESOURCE_0003* pDesc,
    D3D12DDI_RESOURCE_OPTIMIZATION_FLAGS Flags,
    UINT32 AlignmentRestriction,
    UINT VisibleNodeMask,
    _Out_ D3D12DDI_RESOURCE_ALLOCATION_INFO_0022* pInfo)
{
    TRACE_FUNCTION();

    memset(pInfo, 0, sizeof(D3D12DDI_RESOURCE_ALLOCATION_INFO_0022));

    if (pDesc->ResourceType == D3D12DDI_RT_BUFFER)
    {
        pInfo->ResourceDataSize = pDesc->Width;
        pInfo->ResourceDataAlignment = AlignmentRestriction;
        pInfo->AdditionalDataHeaderAlignment = 1;
        pInfo->AdditionalDataAlignment = 1;
        pInfo->Layout = (pDesc->Layout != D3D12DDI_TL_UNDEFINED) ? pDesc->Layout : D3D12DDI_TL_ROW_MAJOR;
    } else {
        UNEXPECTED_DDI();
    }
}

void APIENTRY CosUmd12Device_Ddi_CheckSubresourceInfo(
    D3D12DDI_HDEVICE Device,
    D3D12DDI_HRESOURCE Resource,
    UINT Subresource,
    _Out_ D3D12DDI_SUBRESOURCE_INFO* pInfo)
{
    TRACE_FUNCTION();

    CosUmd12Resource * pResource = (CosUmd12Resource *)Resource.pDrvPrivate;

    //
    // Assumption : Only BUFFER is supported
    //
    memset(pInfo, 0, sizeof(*pInfo));
}

void APIENTRY CosUmd12Device_Ddi_CheckExistingResourceAllocationInfo_0022(
    D3D12DDI_HDEVICE Device,
    D3D12DDI_HRESOURCE Resource,
    _Out_ D3D12DDI_RESOURCE_ALLOCATION_INFO_0022* pInfo)
{
    TRACE_FUNCTION();

    CosUmd12Device * pDevice = CosUmd12Device::CastFrom(Device);
    CosUmd12Resource * pResource = CosUmd12Resource::CastFrom(Resource);

    memset(pInfo, 0, sizeof(*pInfo));

    pInfo->Layout = pResource->GetTextureLayout();
    pInfo->ResourceDataSize = pResource->GetDataSize();
}

HRESULT APIENTRY CosUmd12Device_Ddi_OfferResources(
    D3D12DDI_HDEVICE Device,
    const D3D12DDIARG_OFFERRESOURCES* pDesc)
{
    TRACE_FUNCTION();

    return E_NOTIMPL;
}

HRESULT APIENTRY CosUmd12Device_Ddi_ReclaimResources_0001(
     D3D12DDI_HDEVICE Device, 
     D3D12DDIARG_RECLAIMRESOURCES_0001* pDesc)
{
    TRACE_FUNCTION();

    return E_NOTIMPL;
}
UINT APIENTRY CosUmd12Device_Ddi_GetImplicitPhysicalAdapterMask(
    D3D12DDI_HDEVICE Device)
{
    TRACE_FUNCTION();

    return 0;
}

UINT APIENTRY CosUmd12Device_Ddi_GetPresentPrivateDriverDataSize(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDIARG_PRESENT_0001* pDesc)
{
    UNEXPECTED_DDI();

    return 0;
}

void APIENTRY CosUmd12Device_Ddi_QueryNodeMap(
    D3D12DDI_HDEVICE Device,
    UINT NumPhysicalAdapters,
    _Out_writes_(NumPhysicalAdapters) UINT* pMap)
{
    TRACE_FUNCTION();

    for (UINT i = 0; i < NumPhysicalAdapters; ++i)
    {
        pMap[i] = i;
    }
}

HRESULT APIENTRY CosUmd12Device_Ddi_RetrieveShaderComment_0003(
    D3D12DDI_HDEVICE Device,
    D3D12DDI_HPIPELINESTATE PipelineState,
    _Out_writes_z_(*pCharacterCountIncludingNullTerminator) WCHAR* pBuffer,
    _Inout_ SIZE_T* pCharacterCountIncludingNullTerminator)
{
    TRACE_FUNCTION();

    return E_NOTIMPL;
}

D3DKMT_HANDLE APIENTRY CosUmd12Device_Ddi_CheckResourceAllocationHandle(
    D3D12DDI_HDEVICE Device,
    D3D10DDI_HRESOURCE Resource)
{
    TRACE_FUNCTION();

    return NULL;
}

SIZE_T APIENTRY CosUmd12Device_Ddi_CalcPrivatePipelineLibrarySize_0010(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDIARG_CREATE_PIPELINE_LIBRARY_0010* pDesc)
{
    UNEXPECTED_DDI();

    return 0;
}

HRESULT APIENTRY CosUmd12Device_Ddi_CreatePipelineLibrary_0010(
    D3D12DDI_HDEVICE Device,
    _In_ const D3D12DDIARG_CREATE_PIPELINE_LIBRARY_0010* pDesc,
    D3D12DDI_HPIPELINELIBRARY PipelineLibrary)
{
    UNEXPECTED_DDI();

    return E_NOTIMPL;
}

void APIENTRY CosUmd12Device_Ddi_DestroyPipelineLibrary_0010(
    D3D12DDI_HDEVICE Device, 
    D3D12DDI_HPIPELINELIBRARY PipelineLibrary)
{
    UNEXPECTED_DDI();
}

HRESULT APIENTRY CosUmd12Device_Ddi_AddPipelineStateToLibrary_0010(
    D3D12DDI_HDEVICE Device,
    D3D12DDI_HPIPELINELIBRARY Library,
    D3D12DDI_HPIPELINESTATE PipelineState,
    UINT PipelineIndex)
{
    UNEXPECTED_DDI();

    return E_NOTIMPL;
}

SIZE_T APIENTRY CosUmd12Device_Ddi_CalcSerializedLibrarySize_0010(
    D3D12DDI_HDEVICE Device,
    D3D12DDI_HPIPELINELIBRARY hLibrary)
{
    UNEXPECTED_DDI();

    return 0;
}

HRESULT APIENTRY CosUmd12Device_Ddi_SerializeLibrary_0010(
    D3D12DDI_HDEVICE Device,
    D3D12DDI_HPIPELINELIBRARY hLibrary,
    _Out_ void *pBlob)
{
    UNEXPECTED_DDI();

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
    TRACE_FUNCTION();
}

SIZE_T APIENTRY CosUmd12Device_Ddi_CalcPrivateCommandRecorderSize_0040(
    D3D12DDI_HDEVICE Device,
    _In_ CONST D3D12DDIARG_CREATE_COMMAND_RECORDER_0040* pDesc)
{
    TRACE_FUNCTION();

    return CosUmd12CommandRecorder::CalculateSize(pDesc);
}

HRESULT APIENTRY CosUmd12Device_Ddi_CreateCommandRecorder_0040(
    D3D12DDI_HDEVICE Device,
    _In_ CONST D3D12DDIARG_CREATE_COMMAND_RECORDER_0040* pDesc,
    D3D12DDI_HCOMMANDRECORDER_0040 CommandRecorder)
{
    TRACE_FUNCTION();

    CosUmd12Device * pDevice = CosUmd12Device::CastFrom(Device);

    new (CommandRecorder.pDrvPrivate) CosUmd12CommandRecorder(pDevice, pDesc);
    return S_OK;
}

VOID APIENTRY CosUmd12Device_Ddi_DestroyCommandRecorder_0040(
    D3D12DDI_HDEVICE Device,
    D3D12DDI_HCOMMANDRECORDER_0040 CommandRecorder)
{
    TRACE_FUNCTION();
}

VOID APIENTRY CosUmd12Device_Ddi_CommandRecorderSetCommandPoolAsTarget_0040(
    D3D12DDI_HDEVICE Device,
    D3D12DDI_HCOMMANDRECORDER_0040 CommandRecorder,
    D3D12DDI_HCOMMANDPOOL_0040 CommandPool)
{
    TRACE_FUNCTION();

    CosUmd12CommandRecorder * pCommandRecorder = CosUmd12CommandRecorder::CastFrom(CommandRecorder);
    CosUmd12CommandPool * pCommandPool = CosUmd12CommandPool::CastFrom(CommandPool);

    pCommandRecorder->SetCommandPoolAsTarget(pCommandPool);
}

SIZE_T APIENTRY CosUmd12Device_Ddi_CalcPrivateSchedulingGroupSize(
    D3D12DDI_HDEVICE,
    _In_ CONST D3D12DDIARG_CREATESCHEDULINGGROUP_0050*)
{
    TRACE_FUNCTION();

    return 0;
}

HRESULT APIENTRY CosUmd12Device_Ddi_CreateSchedulingGroup(
    D3D12DDI_HDEVICE Devoce,
    _In_ CONST D3D12DDIARG_CREATESCHEDULINGGROUP_0050* pArgs,
    D3D12DDI_HSCHEDULINGGROUP_0050 SchedulingGroup,
    D3D12DDI_HRTSCHEDULINGGROUP_0050 RtSchedulingGroup)
{
    TRACE_FUNCTION();

    return E_NOINTERFACE;
}

VOID APIENTRY CosUmd12Device_Ddi_DestroySchedulingGroup(
    D3D12DDI_HDEVICE Devuce,
    D3D12DDI_HSCHEDULINGGROUP_0050 SchedulingGroup)
{
    TRACE_FUNCTION();
}

D3D12DDIARG_META_COMMAND_DESC CosUmd12Device::m_supportedMetaCommandDescs[] =
{
    { GUID_IDENTITY,             L"Identity",      D3D12DDI_GRAPHICS_STATE_NONE, D3D12DDI_GRAPHICS_STATE_NONE },
#if COS_MLMC_RS5_SUPPORT
    { MetaCommand_Normalization, L"Normalization", D3D12DDI_GRAPHICS_STATE_NONE, D3D12DDI_GRAPHICS_STATE_NONE },
    { MetaCommand_Convolution,   L"Convolution",   D3D12DDI_GRAPHICS_STATE_NONE, D3D12DDI_GRAPHICS_STATE_NONE },
    { MetaCommand_GEMM,          L"GEMM",          D3D12DDI_GRAPHICS_STATE_NONE, D3D12DDI_GRAPHICS_STATE_NONE },
    { MetaCommand_GRU,           L"GRU",           D3D12DDI_GRAPHICS_STATE_NONE, D3D12DDI_GRAPHICS_STATE_NONE },
    { MetaCommand_LSTM,          L"LSTM",          D3D12DDI_GRAPHICS_STATE_NONE, D3D12DDI_GRAPHICS_STATE_NONE },
    { MetaCommand_MVN,           L"NVM",           D3D12DDI_GRAPHICS_STATE_NONE, D3D12DDI_GRAPHICS_STATE_NONE },
    { MetaCommand_Pooling,       L"Pooling",       D3D12DDI_GRAPHICS_STATE_NONE, D3D12DDI_GRAPHICS_STATE_NONE },
    { MetaCommand_Reduction,     L"Reduction",     D3D12DDI_GRAPHICS_STATE_NONE, D3D12DDI_GRAPHICS_STATE_NONE },
    { MetaCommand_RNN,           L"RNN",           D3D12DDI_GRAPHICS_STATE_NONE, D3D12DDI_GRAPHICS_STATE_NONE },
    { MetaCommand_RoiPooling,    L"RoiPooling",    D3D12DDI_GRAPHICS_STATE_NONE, D3D12DDI_GRAPHICS_STATE_NONE },
    { MetaCommand_CopyTensor,    L"CopyTensor",    D3D12DDI_GRAPHICS_STATE_NONE, D3D12DDI_GRAPHICS_STATE_NONE }
#endif
};

HRESULT APIENTRY CosUmd12Device_Ddi_EnumerateMetaCommands(
    D3D12DDI_HDEVICE Device,
    _Inout_ UINT* pNumMetaCommands,
    _Out_writes_opt_(*pNumMetaCommands) D3D12DDIARG_META_COMMAND_DESC* pDescs)
{
    TRACE_FUNCTION();

    *pNumMetaCommands = _countof(CosUmd12Device::m_supportedMetaCommandDescs);

    if (pDescs)
    {
        memcpy(pDescs, CosUmd12Device::m_supportedMetaCommandDescs, sizeof(CosUmd12Device::m_supportedMetaCommandDescs));
    }
    return S_OK;
}

HRESULT APIENTRY CosUmd12Device_Ddi_EnumerateMetaCommandParameters(
    D3D12DDI_HDEVICE Device,
    GUID CommandId,
    D3D12DDI_META_COMMAND_PARAMETER_STAGE Stage,
    UINT* pParameterCount,
    D3D12DDIARG_META_COMMAND_PARAMETER_DESC* pParameterDescs)
{
    TRACE_FUNCTION();

    if (IsEqualGUID(CommandId, GUID_IDENTITY))
    {
        return CosUmd12MetaCommandIdentity::EnumerateMetaCommandParameters(Stage, pParameterCount, pParameterDescs);
    }
#if COS_MLMC_RS5_SUPPORT
    else if (IsEqualGUID(CommandId, MetaCommand_Normalization))
    {
        return CosUmd12MetaCommandNormalization::EnumerateMetaCommandParameters(Stage, pParameterCount, pParameterDescs);
    }
    else if (IsEqualGUID(CommandId, MetaCommand_Convolution))
    {
        return CosUmd12MetaCommandConvolution::EnumerateMetaCommandParameters(Stage, pParameterCount, pParameterDescs);
    }
    else if (IsEqualGUID(CommandId, MetaCommand_GEMM))
    {
        return CosUmd12MetaCommandGEMM::EnumerateMetaCommandParameters(Stage, pParameterCount, pParameterDescs);
    }
    else if (IsEqualGUID(CommandId, MetaCommand_GRU))
    {
        return CosUmd12MetaCommandGRU::EnumerateMetaCommandParameters(Stage, pParameterCount, pParameterDescs);
    }
    else if (IsEqualGUID(CommandId, MetaCommand_LSTM))
    {
        return CosUmd12MetaCommandLSTM::EnumerateMetaCommandParameters(Stage, pParameterCount, pParameterDescs);
    }
    else if (IsEqualGUID(CommandId, MetaCommand_MVN))
    {
        return CosUmd12MetaCommandMVN::EnumerateMetaCommandParameters(Stage, pParameterCount, pParameterDescs);
    }
    else if (IsEqualGUID(CommandId, MetaCommand_Pooling))
    {
        return CosUmd12MetaCommandPooling::EnumerateMetaCommandParameters(Stage, pParameterCount, pParameterDescs);
    }
    else if (IsEqualGUID(CommandId, MetaCommand_Reduction))
    {
        return CosUmd12MetaCommandReduction::EnumerateMetaCommandParameters(Stage, pParameterCount, pParameterDescs);
    }
    else if (IsEqualGUID(CommandId, MetaCommand_RNN))
    {
        return CosUmd12MetaCommandRNN::EnumerateMetaCommandParameters(Stage, pParameterCount, pParameterDescs);
    }
    else if (IsEqualGUID(CommandId, MetaCommand_RoiPooling))
    {
        return CosUmd12MetaCommandRoiPooling::EnumerateMetaCommandParameters(Stage, pParameterCount, pParameterDescs);
    }
    else if (IsEqualGUID(CommandId, MetaCommand_CopyTensor))
    {
        return CosUmd12MetaCommandCopyTensor::EnumerateMetaCommandParameters(Stage, pParameterCount, pParameterDescs);
    }
#endif

    return E_INVALIDARG;
}

SIZE_T APIENTRY CosUmd12Device_Ddi_CalcPrivateMetaCommandSize(
    D3D12DDI_HDEVICE Device,
    GUID CommandId,
    UINT NodeMask,
    CONST void* pvCreateDesc,
    SIZE_T CreateDescSizeInBytes)
{
    TRACE_FUNCTION();

    if (IsEqualGUID(CommandId, GUID_IDENTITY))
    {
        return CosUmd12MetaCommandIdentity::CalculateSize(CommandId);
    }
#if COS_MLMC_RS5_SUPPORT
    else if (IsEqualGUID(CommandId, MetaCommand_Normalization))
    {
        return CosUmd12MetaCommandNormalization::CalculateSize(CommandId);
    }
    else if (IsEqualGUID(CommandId, MetaCommand_Convolution))
    {
        return CosUmd12MetaCommandConvolution::CalculateSize(CommandId);
    }
    else if (IsEqualGUID(CommandId, MetaCommand_GEMM))
    {
        return CosUmd12MetaCommandGEMM::CalculateSize(CommandId);
    }
    else if (IsEqualGUID(CommandId, MetaCommand_GRU))
    {
        return CosUmd12MetaCommandGRU::CalculateSize(CommandId);
    }
    else if (IsEqualGUID(CommandId, MetaCommand_LSTM))
    {
        return CosUmd12MetaCommandLSTM::CalculateSize(CommandId);
    }
    else if (IsEqualGUID(CommandId, MetaCommand_MVN))
    {
        return CosUmd12MetaCommandMVN::CalculateSize(CommandId);
    }
    else if (IsEqualGUID(CommandId, MetaCommand_Pooling))
    {
        return CosUmd12MetaCommandPooling::CalculateSize(CommandId);
    }
    else if (IsEqualGUID(CommandId, MetaCommand_Reduction))
    {
        return CosUmd12MetaCommandReduction::CalculateSize(CommandId);
    }
    else if (IsEqualGUID(CommandId, MetaCommand_RNN))
    {
        return CosUmd12MetaCommandRNN::CalculateSize(CommandId);
    }
    else if (IsEqualGUID(CommandId, MetaCommand_RoiPooling))
    {
        return CosUmd12MetaCommandRoiPooling::CalculateSize(CommandId);
    }
    else if (IsEqualGUID(CommandId, MetaCommand_CopyTensor))
    {
        return CosUmd12MetaCommandCopyTensor::CalculateSize(CommandId);
    }
#endif
    else
    {
        return 0;
    }
}

HRESULT APIENTRY CosUmd12Device_Ddi_CreateMetaCommand(
    D3D12DDI_HDEVICE Device,
    GUID CommandId,
    UINT NodeMask,
    CONST void* pvCreateDesc,
    SIZE_T CreateDescSizeInBytes,
    D3D12DDI_HMETACOMMAND_0052 MetaCommand,
    D3D12DDI_HRTMETACOMMAND_0052 RtMetaCommand)
{
    TRACE_FUNCTION();

    CosUmd12Device * pDevice = CosUmd12Device::CastFrom(Device);

    if (IsEqualGUID(CommandId, GUID_IDENTITY))
    {
        new (MetaCommand.pDrvPrivate) CosUmd12MetaCommandIdentity(
                                        pDevice,
                                        NodeMask,
                                        pvCreateDesc,
                                        CreateDescSizeInBytes,
                                        RtMetaCommand);
    }
#if COS_MLMC_RS5_SUPPORT
    else if (IsEqualGUID(CommandId, MetaCommand_Normalization))
    {
        new (MetaCommand.pDrvPrivate) CosUmd12MetaCommandNormalization(
                                        pDevice,
                                        NodeMask,
                                        pvCreateDesc,
                                        CreateDescSizeInBytes,
                                        RtMetaCommand);
    }
    else if (IsEqualGUID(CommandId, MetaCommand_Convolution))
    {
        new (MetaCommand.pDrvPrivate) CosUmd12MetaCommandConvolution(
                                        pDevice,
                                        NodeMask,
                                        pvCreateDesc,
                                        CreateDescSizeInBytes,
                                        RtMetaCommand);
    }
    else if (IsEqualGUID(CommandId, MetaCommand_GEMM))
    {
        new (MetaCommand.pDrvPrivate) CosUmd12MetaCommandGEMM(
                                        pDevice,
                                        NodeMask,
                                        pvCreateDesc,
                                        CreateDescSizeInBytes,
                                        RtMetaCommand);
    }
    else if (IsEqualGUID(CommandId, MetaCommand_GRU))
    {
        new (MetaCommand.pDrvPrivate) CosUmd12MetaCommandGRU(
                                        pDevice,
                                        NodeMask,
                                        pvCreateDesc,
                                        CreateDescSizeInBytes,
                                        RtMetaCommand);
    }
    else if (IsEqualGUID(CommandId, MetaCommand_LSTM))
    {
        new (MetaCommand.pDrvPrivate) CosUmd12MetaCommandLSTM(
                                        pDevice,
                                        NodeMask,
                                        pvCreateDesc,
                                        CreateDescSizeInBytes,
                                        RtMetaCommand);
    }
    else if (IsEqualGUID(CommandId, MetaCommand_MVN))
    {
        new (MetaCommand.pDrvPrivate) CosUmd12MetaCommandMVN(
                                        pDevice,
                                        NodeMask,
                                        pvCreateDesc,
                                        CreateDescSizeInBytes,
                                        RtMetaCommand);
    }
    else if (IsEqualGUID(CommandId, MetaCommand_Pooling))
    {
        new (MetaCommand.pDrvPrivate) CosUmd12MetaCommandPooling(
                                        pDevice,
                                        NodeMask,
                                        pvCreateDesc,
                                        CreateDescSizeInBytes,
                                        RtMetaCommand);
    }
    else if (IsEqualGUID(CommandId, MetaCommand_Reduction))
    {
        new (MetaCommand.pDrvPrivate) CosUmd12MetaCommandReduction(
                                        pDevice,
                                        NodeMask,
                                        pvCreateDesc,
                                        CreateDescSizeInBytes,
                                        RtMetaCommand);
    }
    else if (IsEqualGUID(CommandId, MetaCommand_RNN))
    {
        new (MetaCommand.pDrvPrivate) CosUmd12MetaCommandRNN(
                                        pDevice,
                                        NodeMask,
                                        pvCreateDesc,
                                        CreateDescSizeInBytes,
                                        RtMetaCommand);
    }
    else if (IsEqualGUID(CommandId, MetaCommand_RoiPooling))
    {
        new (MetaCommand.pDrvPrivate) CosUmd12MetaCommandRoiPooling(
                                        pDevice,
                                        NodeMask,
                                        pvCreateDesc,
                                        CreateDescSizeInBytes,
                                        RtMetaCommand);
    }
#if 0
    //
    // Implementation of CopyTensor Meta Command indicates driver's preferrence for
    // HW specific tensor layout (META_COMMAND_TENSOR_LAYOUT_UNKNOWN)
    //
    // Accordingly, GetRequiredParameterInfo() Ddi must calculate a tensor resource's
    // size based on its chosen HW layout.
    //
    // Enable only for Windows build newer than 18231
    // Since it is used to prepare data for other meta commands, only enable after solid testing
    //
    // ResourceCopy and CopyBufferRegion are used for META_COMMAND_TENSOR_LAYOUT_STANDARD
    //
    else if (IsEqualGUID(CommandId, MetaCommand_CopyTensor))
    {
        new (MetaCommand.pDrvPrivate) CosUmd12MetaCommandCopyTensor(
            pDevice,
            NodeMask,
            pvCreateDesc,
            CreateDescSizeInBytes,
            RtMetaCommand);
    }
#endif
#endif
    else
    {
        return DXGI_ERROR_UNSUPPORTED;
    }

    return S_OK;
}

VOID APIENTRY CosUmd12Device_Ddi_DestroyMetaCommand(
    D3D12DDI_HDEVICE Device,
    D3D12DDI_HMETACOMMAND_0052 MetaCommand)
{
    TRACE_FUNCTION();
}

VOID APIENTRY CosUmd12Device_Ddi_GetMetaCommandRequiredParameterInfo(
    D3D12DDI_HMETACOMMAND_0052 MetaCommand,
    D3D12DDI_META_COMMAND_PARAMETER_STAGE Stage,
    UINT ParameterIndex,
    _Out_ D3D12DDIARG_META_COMMAND_REQUIRED_PARAMETER_INFO* pInfo)
{
    TRACE_FUNCTION();

    CosUmd12MetaCommand * pMetaCommand = CosUmd12MetaCommand::CastFrom(MetaCommand);

    pMetaCommand->GetRequiredParameterInfo(Stage, ParameterIndex, pInfo);
}

D3D12DDI_DEVICE_FUNCS_CORE_0052 g_CosUmd12Device_Ddi_0052 =
{
    CosUmd12Device_Ddi_CheckFormatSupport,                              // pfnCheckFormatSupport
    CosUmd12Device_Ddi_CheckMultisampleQualityLevels,                       // pfnCheckMultisampleQualityLevels
    CosUmd12Device_Ddi_GetMipPacking,                                       // pfnGetMipPacking
    CosUmd12Device_Ddi_CalcPrivateElementLayoutSize_0010,                   // pfnCalcPrivateElementLayoutSize
    CosUmd12Device_Ddi_CreateElementLayout_0010,                            // pfnCreateElementLayout
    CosUmd12Device_Ddi_DestroyElementLayout,                                // pfnDestroyElementLayout
    CosUmd12Device_Ddi_CalcPrivateBlendStateSize_0010,                      // pfnCalcPrivateBlendStateSize
    CosUmd12Device_Ddi_CreateBlendState_0010,                               // pfnCreateBlendState
    CosUmd12Device_Ddi_DestroyBlendState,                                   // pfnDestroyBlendState
    CosUmd12Device_Ddi_CalcPrivateDepthStencilStateSize_0025,               // pfnCalcPrivateDepthStencilStateSize
    CosUmd12Device_Ddi_CreateDepthStencilState_0025,                        // pfnCreateDepthStencilState
    CosUmd12Device_Ddi_DestroyDepthStencilState,                            // pfnDestroyDepthStencilState
    CosUmd12Device_Ddi_CalcPrivateRasterizerStateSize_0010,                 // pfnCalcPrivateRasterizerStateSize
    CosUmd12Device_Ddi_CreateRasterizerState_0010,                          // pfnCreateRasterizerState
    CosUmd12Device_Ddi_DestroyRasterizerState,                              // pfnDestroyRasterizerState
    CosUmd12Device_Ddi_CalcPrivateShaderSize_0026,                      // pfnCalcPrivateShaderSize
    CosUmd12Device_Ddi_CreateVertexShader_0026,                             // pfnCreateVertexShader
    CosUmd12Device_Ddi_CreatePixelShader_0026,                              // pfnCreatePixelShader
    CosUmd12Device_Ddi_CreateGeometryShader_0026,                           // pfnCreateGeometryShader
    CosUmd12Device_Ddi_CreateComputeShader_0026,                        // pfnCreateComputeShader
    CosUmd12Device_Ddi_CalcPrivateGeometryShaderWithStreamOutput_0026,      // pfnCalcPrivateGeometryShaderWithStreamOutput
    CosUmd12Device_Ddi_CreateGeometryShaderWithStreamOutput_0026,           // pfnCreateGeometryShaderWithStreamOutput
    CosUmd12Device_Ddi_CalcPrivateTessellationShaderSize_0026,              // pfnCalcPrivateTessellationShaderSize
    CosUmd12Device_Ddi_CreateHullShader_0026,                               // pfnCreateHullShader
    CosUmd12Device_Ddi_CreateDomainShader_0026,                             // pfnCreateDomainShader
    CosUmd12Device_Ddi_DestroyShader,                                   // pfnDestroyShader
    CosUmd12Device_Ddi_CalcPrivateCommandQueueSize_0050,                // pfnCalcPrivateCommandQueueSize
    CosUmd12Device_Ddi_CreateCommandQueue_0050,                         // pfnCreateCommandQueue
    CosUmd12Device_Ddi_DestroyCommandQueue,                             // pfnDestroyCommandQueue
    CosUmd12Device_Ddi_CalcPrivateCommandPoolSize_0040,                 // pfnCalcPrivateCommandPoolSize
    CosUmd12Device_Ddi_CreateCommandPool_0040,                          // pfnCreateCommandPool
    CosUmd12Device_Ddi_DestroyCommandPool_0040,                         // pfnDestroyCommandPool
    CosUmd12Device_Ddi_ResetCommandPool_0040,                           // pfnResetCommandPool
    CosUmd12Device_Ddi_CalcPrivatePipelineStateSize_0033,               // pfnCalcPrivatePipelineStateSize
    CosUmd12Device_Ddi_CreatePipelineState_0033,                        // pfnCreatePipelineState
    CosUmd12Device_Ddi_DestroyPipelineState,                            // pfnDestroyPipelineState
    CosUmd12Device_Ddi_CalcPrivateCommandListSize_0040,                 // pfnCalcPrivateCommandListSize
    CosUmd12Device_Ddi_CreateCommandList_0040,                          // pfnCreateCommandList
    CosUmd12Device_Ddi_DestroyCommandList,                              // pfnDestroyCommandList
    CosUmd12Device_Ddi_CalcPrivateFenceSize,                            // pfnCalcPrivateFenceSize
    CosUmd12Device_Ddi_CreateFence,                                     // pfnCreateFence
    CosUmd12Device_Ddi_DestroyFence,                                    // pfnDestroyFence
    CosUmd12Device_Ddi_CalcPrivateDescriptorHeapSize_0001,              // pfnCalcPrivateDescriptorHeapSize
    CosUmd12Device_Ddi_CreateDescriptorHeap_0001,                       // pfnCreateDescriptorHeap
    CosUmd12Device_Ddi_DestroyDescriptorHeap,                           // pfnDestroyDescriptorHeap
    CosUmd12Device_Ddi_GetDescriptorSizeInBytes,                        // pfnGetDescriptorSizeInBytes
    CosUmd12Device_Ddi_GetCpuDescriptorHandleForHeapStart,              // pfnGetCPUDescriptorHandleForHeapStart
    CosUmd12Device_Ddi_GetGpuDescriptorHandleForHeapStart,              // pfnGetGPUDescriptorHandleForHeapStart
    CosUmd12Device_Ddi_CreateShaderResourceView_0002,                       // pfnCreateShaderResourceView
    CosUmd12Device_Ddi_CreateConstantBufferView,                        // pfnCreateConstantBufferView
    CosUmd12Device_Ddi_CreateSampler,                                       // pfnCreateSampler
    CosUmd12Device_Ddi_CreateUnorderedAccessView_0002,                  // pfnCreateUnorderedAccessView
    CosUmd12Device_Ddi_CreateRenderTargetView_0002,                         // pfnCreateRenderTargetView
    CosUmd12Device_Ddi_CreateDepthStencilView,                              // pfnCreateDepthStencilView
    CosUmd12Device_Ddi_CalcPrivateRootSignatureSize_0013,               // pfnCalcPrivateRootSignatureSize
    CosUmd12Device_Ddi_CreateRootSignature_0013,                        // pfnCreateRootSignature
    CosUmd12Device_Ddi_DestroyRootSignature,                            // pfnDestroyRootSignature
    CosUmd12Device_Ddi_MapHeap,                                         // pfnMapHeap
    CosUmd12Device_Ddi_UnmapHeap,                                       // pfnUnmapHeap
    CosUmd12Device_Ddi_CalcPrivateHeapAndResourceSizes_0030,            // pfnCalcPrivateHeapAndResourceSizes
    CosUmd12Device_Ddi_CreateHeapAndResource_0030,                      // pfnCreateHeapAndResource
    CosUmd12Device_Ddi_DestroyHeapAndResource,                          // pfnDestroyHeapAndResource
    CosUmd12Device_Ddi_MakeResident_0001,                               // pfnMakeResident
    CosUmd12Device_Ddi_Evict2,                                          // pfnEvict
    CosUmd12Device_Ddi_CalcPrivateOpenedHeapAndResourceSizes_0043,      // pfnCalcPrivateOpenedHeapAndResourceSizes
    CosUmd12Device_Ddi_OpenHeapAndResource_0043,                        // pfnOpenHeapAndResource
    CosUmd12Device_Ddi_CopyDescriptors_0003,                            // pfnCopyDescriptors
    CosUmd12Device_Ddi_CopyDescriptorsSimple_0003,                      // pfnCopyDescriptorsSimple
    CosUmd12Device_Ddi_CalcPrivateQueryHeapSize_0001,                   // pfnCalcPrivateQueryHeapSize
    CosUmd12Device_Ddi_CreateQueryHeap_0001,                            // pfnCreateQueryHeap
    CosUmd12Device_Ddi_DestroyQueryHeap,                                // pfnDestroyQueryHeap
    CosUmd12Device_Ddi_CalcPrivateCommandSignatureSize_0001,                // pfnCalcPrivateCommandSignatureSize
    CosUmd12Device_Ddi_CreateCommandSignature_0001,                         // pfnCreateCommandSignature
    CosUmd12Device_Ddi_DestroyCommandSignature,                             // pfnDestroyCommandSignature
    CosUmd12Device_Ddi_CheckResourceVirtualAddress,                     // pfnCheckResourceVirtualAddress
    CosUmd12Device_Ddi_CheckResourceAllocationInfo_0022,                // pfnCheckResourceAllocationInfo
    CosUmd12Device_Ddi_CheckSubresourceInfo,                            // pfnCheckSubresourceInfo
    CosUmd12Device_Ddi_CheckExistingResourceAllocationInfo_0022,        // pfnCheckExistingResourceAllocationInfo
    CosUmd12Device_Ddi_OfferResources,                                  // pfnOfferResources
    CosUmd12Device_Ddi_ReclaimResources_0001,                           // pfnReclaimResources
    CosUmd12Device_Ddi_GetImplicitPhysicalAdapterMask,                  // pfnGetImplicitPhysicalAdapterMask
    CosUmd12Device_Ddi_GetPresentPrivateDriverDataSize,                     // pfnGetPresentPrivateDriverDataSize
    CosUmd12Device_Ddi_QueryNodeMap,                                    // pfnQueryNodeMap
    CosUmd12Device_Ddi_RetrieveShaderComment_0003,                      // pfnRetrieveShaderComment
    CosUmd12Device_Ddi_CheckResourceAllocationHandle,                   // pfnCheckResourceAllocationHandle
    CosUmd12Device_Ddi_CalcPrivatePipelineLibrarySize_0010,                 // pfnCalcPrivatePipelineLibrarySize
    CosUmd12Device_Ddi_CreatePipelineLibrary_0010,                          // pfnCreatePipelineLibrary
    CosUmd12Device_Ddi_DestroyPipelineLibrary_0010,                         // pfnDestroyPipelineLibrary
    CosUmd12Device_Ddi_AddPipelineStateToLibrary_0010,                      // pfnAddPipelineStateToLibrary
    CosUmd12Device_Ddi_CalcSerializedLibrarySize_0010,                      // pfnCalcSerializedLibrarySize
    CosUmd12Device_Ddi_SerializeLibrary_0010,                               // pfnSerializeLibrary
    CosUmd12Device_Ddi_GetDebugAllocationInfo_0014,                     // pfnGetDebugAllocationInfo
    CosUmd12Device_Ddi_CalcPrivateCommandRecorderSize_0040,             // pfnCalcPrivateCommandRecorderSize
    CosUmd12Device_Ddi_CreateCommandRecorder_0040,                      // pfnCreateCommandRecorder
    CosUmd12Device_Ddi_DestroyCommandRecorder_0040,                     // pfnDestroyCommandRecorder
    CosUmd12Device_Ddi_CommandRecorderSetCommandPoolAsTarget_0040,      // pfnCommandRecorderSetCommandPoolAsTarget
    CosUmd12Device_Ddi_CalcPrivateSchedulingGroupSize,                  // pfnCalcPrivateSchedulingGroupSize
    CosUmd12Device_Ddi_CreateSchedulingGroup,                           // pfnCreateSchedulingGroup
    CosUmd12Device_Ddi_DestroySchedulingGroup,                          // pfnDestroySchedulingGroup
    CosUmd12Device_Ddi_EnumerateMetaCommands,                           // pfnEnumerateMetaCommands
    CosUmd12Device_Ddi_EnumerateMetaCommandParameters,                  // pfnEnumerateMetaCommandParameters
    CosUmd12Device_Ddi_CalcPrivateMetaCommandSize,                      // pfnCalcPrivateMetaCommandSize
    CosUmd12Device_Ddi_CreateMetaCommand,                               // pfnCreateMetaCommand
    CosUmd12Device_Ddi_DestroyMetaCommand,                              // pfnDestroyMetaCommand
    CosUmd12Device_Ddi_GetMetaCommandRequiredParameterInfo              // pfnGetMetaCommandRequiredParameterInfo
};
