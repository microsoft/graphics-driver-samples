#pragma once

#include "CosUmd12.h"

const UINT COS_MAX_NUM_COMMAND_BUFFERS = 256;

class CosUmd12CommandList
{
public:
    explicit CosUmd12CommandList(CosUmd12Device* pDevice, const D3D12DDIARG_CREATE_COMMAND_LIST_0001* pArgs)
    {
        m_pDevice = pDevice;
        m_args = *pArgs;
        m_pPipelineState = NULL;
        m_numFilledCommandBuffers = 0;
    }

    ~CosUmd12CommandList()
    {
    }

    void Reset()
    {
        // do nothing
    }

    static int CalculateSize(const D3D12DDIARG_CREATE_COMMAND_LIST_0001 * pArgs)
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

    // Interface for Command Queue
    void Execute(CosUmd12CommandQueue * pCommandQueue);

    void ResourceCopy(D3D12DDI_HRESOURCE DstResource, D3D12DDI_HRESOURCE SrcResource);

private:

    CosUmd12Device * m_pDevice;
    D3D12DDIARG_CREATE_COMMAND_LIST_0001 m_args;
    CosUmd12PipelineState * m_pPipelineState;

    CosUmd12CommandAllocator * m_pCommandAllocator;

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
