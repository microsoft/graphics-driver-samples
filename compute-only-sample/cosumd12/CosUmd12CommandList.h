#pragma once

#include "CosUmd12.h"

class CosUmd12DescriptorHeap;

const UINT COS_MAX_NUM_COMMAND_BUFFERS = 256;

class CosUmd12CommandList
{
public:
    explicit CosUmd12CommandList(CosUmd12Device* pDevice, const D3D12DDIARG_CREATE_COMMAND_LIST_0040* pArgs, D3D12DDI_HRTCOMMANDLIST rtCommandList)
    {
        m_pDevice = pDevice;
        m_args = *pArgs;
        m_rtCommandList = rtCommandList;
        m_pPipelineState = NULL;
        memset(m_pDescriptorHeaps, 0, sizeof(m_pDescriptorHeaps));
        m_pCommandPool = NULL;
        m_numFilledCommandBuffers = 0;
    }

    ~CosUmd12CommandList()
    {
    }

    void Reset(const D3D12DDIARG_RESETCOMMANDLIST_0040 * pReset);

    static int CalculateSize(const D3D12DDIARG_CREATE_COMMAND_LIST_0040 * pArgs)
    {
        return sizeof(CosUmd12CommandList);
    }

    HRESULT StandUp();

    void SetPipelineState(CosUmd12PipelineState * pPipelineState)
    {
        m_pPipelineState = pPipelineState;
    }

    bool IsComputeType() { return (m_args.QueueFlags & D3D12DDI_COMMAND_QUEUE_FLAG_3D) == 0; }
    bool IsRenderType() { return (m_args.QueueFlags & D3D12DDI_COMMAND_QUEUE_FLAG_3D) != 0; }

    static CosUmd12CommandList* CastFrom(D3D12DDI_HCOMMANDLIST);
    D3D12DDI_HCOMMANDLIST CastTo() const;

    void Close();

    void SetRootSignature(
        D3D12DDI_HROOTSIGNATURE rootSignature);

    void SetDescriptorHeaps(
        UINT numDescriptorHeaps,
        D3D12DDI_HDESCRIPTORHEAP * pDescriptorHeaps);

    void SetRootDescriptorTable(
        UINT rootParameterIndex,
        D3D12DDI_GPU_DESCRIPTOR_HANDLE baseDescriptor);

    void SetRoot32BitConstants(
        UINT rootParameterIndex,
        UINT num32BitValuesToSet,
        const void* pSrcData,
        UINT destOffsetIn32BitValues);

    void SetRootView(
        UINT RootParameterIndex,
        D3D12DDI_GPU_VIRTUAL_ADDRESS BufferLocation);

    void Dispatch(
        UINT ThreadGroupCountX,
        UINT ThreadGroupCountY,
        UINT ThreadGroupCountZ);

    void ResourceCopy(D3D12DDI_HRESOURCE DstResource, D3D12DDI_HRESOURCE SrcResource);
    void CopyBufferRegion(D3D12DDIARG_BUFFER_PLACEMENT& dst, D3D12DDIARG_BUFFER_PLACEMENT& src, UINT64 bytesToCopy);
    void GpuMemoryCopy(D3D12_GPU_VIRTUAL_ADDRESS dstGpuVa, D3D12_GPU_VIRTUAL_ADDRESS srcGpuVa, UINT size);

    // Interface for Command Queue
    void Execute(CosUmd12CommandQueue * pCommandQueue);

private:

    CosUmd12Device * m_pDevice;
    D3D12DDIARG_CREATE_COMMAND_LIST_0040 m_args;
    D3D12DDI_HRTCOMMANDLIST m_rtCommandList;
    CosUmd12PipelineState * m_pPipelineState;
    CosUmd12DescriptorHeap * m_pDescriptorHeaps[D3D12DDI_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];

    //
    // Storage for current root values specified by Root Signature
    //

    BYTE m_rootValues[SIZE_ROOT_SIGNATURE];

    CosUmd12CommandPool * m_pCommandPool;

    CosUmd12CommandBuffer * m_pCurCommandBuffer;

    UINT m_numFilledCommandBuffers;
    CosUmd12CommandBuffer * m_filledCommandBuffers[COS_MAX_NUM_COMMAND_BUFFERS];

    void
    ReserveCommandBufferSpace(
        bool                        bSwCommand,
        UINT                        commandSize,
        BYTE **                     ppCommandBuffer,
        UINT                        allocationListSize = 0,
        UINT                        patchLocationSize = 0,
        UINT *                      pCurCommandOffset = NULL,
        D3DDDI_PATCHLOCATIONLIST ** ppPatchLocationList = NULL);
};

inline CosUmd12CommandList* CosUmd12CommandList::CastFrom(D3D12DDI_HCOMMANDLIST hRootSignature)
{
    return static_cast< CosUmd12CommandList* >(hRootSignature.pDrvPrivate);
}

inline D3D12DDI_HCOMMANDLIST CosUmd12CommandList::CastTo() const
{
    // TODO: Why does MAKE_D3D10DDI_HDEPTHSTENCILSTATE not exist?
    return MAKE_D3D12DDI_HCOMMANDLIST(const_cast< CosUmd12CommandList* >(this));
}
