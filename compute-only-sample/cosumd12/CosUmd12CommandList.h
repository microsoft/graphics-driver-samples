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

    template <typename THwMetaCommand, typename THwIoTable>
    void ExecuteMlMetaCommand(THwMetaCommand * pHwMetaCommand, THwIoTable * pHwIoTable, MetaCommandId metaCommandId)
    {
        UINT numPatchLocations = sizeof(THwIoTable)/sizeof(D3D12_GPU_DESCRIPTOR_HANDLE);
        UINT commandSize = sizeof(GpuHwMetaCommand) + sizeof(THwMetaCommand) + numPatchLocations*sizeof(GpuHWDescriptor);

        BYTE * pCommandBuf;
        UINT curCommandOffset;
        D3DDDI_PATCHLOCATIONLIST * pPatchLocationList;

        ReserveCommandBufferSpace(
            false,                          // HW command
            commandSize,
            (BYTE **)&pCommandBuf,
            numPatchLocations,
            numPatchLocations,
            &curCommandOffset,
            &pPatchLocationList);
        if (NULL == pCommandBuf)
        {
            return;
        }

        GpuHwMetaCommand *  pMetaCommand = (GpuHwMetaCommand *)pCommandBuf;

        pMetaCommand->m_commandId = MetaCommandExecute;
        pMetaCommand->m_commandSize = commandSize;

        pMetaCommand->m_metaCommandId = metaCommandId;

        // Copy the meta command META_COMMAND_CREATE_*_DESC
        memcpy(pMetaCommand + 1, pHwMetaCommand, sizeof(THwMetaCommand));

        // Clear the resource descriptors
        GpuHWDescriptor *   pHwDescriptors = (GpuHWDescriptor *)(pCommandBuf + sizeof(GpuHwMetaCommand) + sizeof(THwMetaCommand));
        memset(pHwDescriptors, 0, numPatchLocations*sizeof(GpuHWDescriptor));

        //
        // Copy the resource descriptors into the command buffer
        //
        // For ML Meta Command, all resources are referenced by their (UAV) descriptors'
        // D3D12_GPU_DESCRIPTOR_HANDLE(GPU VA) (within the descriptor heap) directly.
        //
        // For HW with GPU VA support, this should be a simple copy of META_COMMAND_EXECUTE_*_DESC
        //
        //
        
        D3D12_GPU_DESCRIPTOR_HANDLE *   pGpuDescriptorHandle = (D3D12_GPU_DESCRIPTOR_HANDLE *)pHwIoTable;
        CosUmd12DescriptorHeap *        pUavHeap = m_pDescriptorHeaps[D3D12DDI_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV];
        UINT                            hwDescriptorOffset = curCommandOffset + sizeof(GpuHwMetaCommand) + sizeof(THwMetaCommand);
        UINT                            i, numPatchLocationsUsed;

        for (i = 0, numPatchLocationsUsed = 0; i < numPatchLocations; i++, pGpuDescriptorHandle++)
        {
            if (pGpuDescriptorHandle->ptr)
            {
                UINT descriptorIndex = (UINT)((pGpuDescriptorHandle->ptr - 
                                               pUavHeap->GetGpuAddress()))/sizeof(CosUmd12Descriptor);

                CosUmd12Descriptor * pDescriptor = pUavHeap->GetCpuAddress() + descriptorIndex;

                pDescriptor->WriteHWDescriptor(
                                m_pCurCommandBuffer,
                                hwDescriptorOffset,
                                pPatchLocationList);

                numPatchLocationsUsed++;
            }

            hwDescriptorOffset += sizeof(GpuHWDescriptor);
        }

        // Commit the command into command buffer
        m_pCurCommandBuffer->CommitCommandBufferSpace(commandSize, numPatchLocationsUsed);
    }

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
