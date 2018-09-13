#include "CosUmd12.h"

void APIENTRY CosUmd12ComputeCommandList_Ddi_CloseCommandList(
    D3D12DDI_HCOMMANDLIST CommandList)
{
    TRACE_FUNCTION();

    CosUmd12CommandList * pCommandList = CosUmd12CommandList::CastFrom(CommandList);

    pCommandList->Close();
}

void APIENTRY CosUmd12ComputeCommandList_Ddi_ResetCommandList_0040(
    D3D12DDI_HCOMMANDLIST CommandList,
    _In_ const D3D12DDIARG_RESETCOMMANDLIST_0040* pReset)
{
    TRACE_FUNCTION();

    CosUmd12CommandList * pCommandList = CosUmd12CommandList::CastFrom(CommandList);

    pCommandList->Reset(pReset);
}

void APIENTRY CosUmd12ComputeCommandList_Ddi_DrawInstanced(
    D3D12DDI_HCOMMANDLIST CommandList,
    UINT VertexCountPerInstance,
    UINT InstanceCount,
    UINT StartVertexLocation,
    UINT StartInstanceLocation)
{
    UNEXPECTED_DDI();
}

void APIENTRY CosUmd12ComputeCommandList_Ddi_DrawIndexedInstanced(
    D3D12DDI_HCOMMANDLIST CommandList,
    UINT IndexCountPerInstance,
    UINT InstanceCount,
    UINT StartIndexLocation,
    INT BaseVertexLocation,
    UINT StartInstanceLocation)
{
    UNEXPECTED_DDI();
}

void APIENTRY CosUmd12ComputeCommandList_Ddi_Dispatch(
    D3D12DDI_HCOMMANDLIST CommandList,
    UINT ThreadGroupCountX,
    UINT ThreadGroupCountY,
    UINT ThreadGroupCountZ)
{
    TRACE_FUNCTION();

    CosUmd12CommandList * pCommandList = CosUmd12CommandList::CastFrom(CommandList);

    pCommandList->Dispatch(ThreadGroupCountX, ThreadGroupCountY, ThreadGroupCountZ);
}

void APIENTRY CosUmd12ComputeCommandList_Ddi_ClearUnorderedAccessViewUint_0003(
    D3D12DDI_HCOMMANDLIST CommandList,
    D3D12DDI_GPU_DESCRIPTOR_HANDLE ViewGPUHandleInCurrentHeap,
    D3D12DDI_CPU_DESCRIPTOR_HANDLE ViewCPUHandle,
    D3D12DDI_HRESOURCE Resource,
    const UINT Values[4],
    UINT NumRects,
    _In_reads_(NumRects) const D3D12DDI_RECT* pRects)
{
    TRACE_FUNCTION();
}

void APIENTRY CosUmd12ComputeCommandList_Ddi_ClearUnorderedAccessViewFloat_0003(
    D3D12DDI_HCOMMANDLIST CommandList,
    D3D12DDI_GPU_DESCRIPTOR_HANDLE ViewGPUHandleInCurrentHeap,
    D3D12DDI_CPU_DESCRIPTOR_HANDLE ViewCPUHandle,
    D3D12DDI_HRESOURCE Resource,
    const FLOAT Values[4],
    UINT NumRects,
    _In_reads_(NumRects) const D3D12DDI_RECT* pRects)
{
    TRACE_FUNCTION();
}

void APIENTRY CosUmd12ComputeCommandList_Ddi_ClearRenderTargetView_0003(
    D3D12DDI_HCOMMANDLIST CommandList,
    D3D12DDI_CPU_DESCRIPTOR_HANDLE ViewCPUHandle,
    const FLOAT ColorRGBA[4],
    UINT NumRects,
    _In_reads_(NumRects) const D3D12DDI_RECT* pRects)
{
    UNEXPECTED_DDI();
}

void APIENTRY CosUmd12ComputeCommandList_Ddi_ClearDepthStencilView_0003(
    D3D12DDI_HCOMMANDLIST CommandList,
    D3D12DDI_CPU_DESCRIPTOR_HANDLE ViewCPUHandle,
    UINT Flags,
    FLOAT Depth,
    UINT8 Stencil,
    UINT NumRects,
    _In_reads_(NumRects) const D3D12DDI_RECT* pRects)
{
    UNEXPECTED_DDI();
}

void APIENTRY CosUmd12ComputeCommandList_Ddi_DiscardResource_0003(
    D3D12DDI_HCOMMANDLIST CommandList,
    D3D12DDI_HRESOURCE Resource,
    _In_opt_ const D3D12DDIARG_DISCARD_RESOURCE_0003* pDiscard)
{
    TRACE_FUNCTION();
}

void APIENTRY CosUmd12ComputeCommandList_Ddi_CopyTextureRegion_0003(
    D3D12DDI_HCOMMANDLIST CommandList,
    _In_ const D3D12DDIARG_BUFFER_PLACEMENT* pBufferPlacementDst,
    D3D12DDIARG_PLACED_RESOURCE PlacedResourceDst,
    UINT DstX,
    UINT DstY,
    UINT DstZ,
    _In_ const D3D12DDIARG_BUFFER_PLACEMENT* pBufferPlacementSrc,
    D3D12DDIARG_PLACED_RESOURCE PlacedResourceSrc,
    _In_opt_ const D3D12DDI_BOX* pSrcBox)
{
    UNEXPECTED_DDI();
}

void APIENTRY CosUmd12ComputeCommandList_Ddi_ResourceCopy(
    D3D12DDI_HCOMMANDLIST CommandList,
    D3D12DDI_HRESOURCE DstResource,
    D3D12DDI_HRESOURCE SrcResource)
{
    TRACE_FUNCTION();

    CosUmd12CommandList * pCommandList = CosUmd12CommandList::CastFrom(CommandList);

    pCommandList->ResourceCopy(DstResource, SrcResource);
}

void APIENTRY CosUmd12ComputeCommandList_Ddi_CopyTiles(
    D3D12DDI_HCOMMANDLIST CommandList,
    D3D12DDI_HRESOURCE Resource,
    _In_ const D3D12DDI_TILED_RESOURCE_COORDINATE* pRegionStartCoord,
    _In_ const D3D12DDI_TILE_REGION_SIZE* pRegionSize,
    D3D12DDI_HRESOURCE Buffer, // buffer
    UINT64 BufferStartOffsetInBytes,
    D3D12DDI_TILE_COPY_FLAGS CopyFlags)
{
    UNEXPECTED_DDI();
}

void APIENTRY CosUmd12ComputeCommandList_Ddi_CopyBufferRegion_0003(
    D3D12DDI_HCOMMANDLIST CommandList,
    D3D12DDIARG_BUFFER_PLACEMENT Dst,
    D3D12DDIARG_BUFFER_PLACEMENT Src,
    UINT64 SrcBytes)
{
    TRACE_FUNCTION();

    CosUmd12CommandList * pCommandList = CosUmd12CommandList::CastFrom(CommandList);

    pCommandList->CopyBufferRegion(Dst, Src, SrcBytes);
}

void APIENTRY CosUmd12ComputeCommandList_Ddi_ResourceResolveSubresource(
    D3D12DDI_HCOMMANDLIST CommandList,
    D3D12DDI_HRESOURCE DstResource,
    UINT DstSubresource,
    D3D12DDI_HRESOURCE SrcResource,
    UINT SrcSubresource,
    DXGI_FORMAT Format)
{
    UNEXPECTED_DDI();
}

void APIENTRY CosUmd12ComputeCommandList_Ddi_ExecuteBundle(
    D3D12DDI_HCOMMANDLIST CommandList,
    D3D12DDI_HCOMMANDLIST Bundle)
{
    TRACE_FUNCTION();
}

void APIENTRY CosUmd12ComputeCommandList_Ddi_ExecuteIndirect(
    D3D12DDI_HCOMMANDLIST CommandList,
    D3D12DDI_HCOMMANDSIGNATURE CommandSignature,
    UINT MaxCommandCount,
    D3D12DDIARG_BUFFER_PLACEMENT ArgumentBuffer,
    D3D12DDIARG_BUFFER_PLACEMENT CountBuffer)
{
    UNEXPECTED_DDI();
}

void APIENTRY CosUmd12ComputeCommandList_Ddi_ResourceBarrier_0022(
    D3D12DDI_HCOMMANDLIST CommandList,
    UINT Count,
    _In_reads_(Count) const D3D12DDIARG_RESOURCE_BARRIER_0022* pResourceBarrierDesc)
{
    TRACE_FUNCTION();
}

void APIENTRY CosUmd12ComputeCommandList_Ddi_Blt(
    D3D12DDI_HCOMMANDLIST CommandList,
    _In_ const D3D12DDIARG_BLT*)
{
    UNEXPECTED_DDI();
}

void APIENTRY CosUmd12ComputeCommandList_Ddi_Present_0051(
    D3D12DDI_HCOMMANDLIST CommandList, 
    D3D12DDI_HCOMMANDQUEUE CommandQueue, 
    _In_ const D3D12DDIARG_PRESENT_0001* pDesc, 
    _Out_ D3D12DDI_PRESENT_0051* pPresent,
    _Out_opt_ D3D12DDI_PRESENT_CONTEXTS_0051*,
    _Out_opt_ D3D12DDI_PRESENT_HWQUEUES_0051*)
{
    UNEXPECTED_DDI();
}

void APIENTRY CosUmd12ComputeCommandList_Ddi_BeginQuery_0003(
    D3D12DDI_HCOMMANDLIST CommandList,
    D3D12DDI_HQUERYHEAP QueryHeap,
    D3D12DDI_QUERY_TYPE QueryType,
    UINT Index)
{
    TRACE_FUNCTION();
}

void APIENTRY CosUmd12ComputeCommandList_Ddi_EndQuery_0003(
    D3D12DDI_HCOMMANDLIST CommandList,
    D3D12DDI_HQUERYHEAP QueryHeap,
    D3D12DDI_QUERY_TYPE QueryType,
    UINT Index)
{
    TRACE_FUNCTION();
}

void APIENTRY CosUmd12ComputeCommandList_Ddi_ResolveQueryData(
    D3D12DDI_HCOMMANDLIST CommandList,
    D3D12DDI_HQUERYHEAP QueryHeap,
    D3D12DDI_QUERY_TYPE QueryType,
    UINT StartElement,
    UINT ElementCount,
    D3D12DDI_HRESOURCE DestinationBuffer,
    UINT64 DestinationOffset)
{
    TRACE_FUNCTION();
}

void APIENTRY CosUmd12ComputeCommandList_Ddi_SetPredication(
    D3D12DDI_HCOMMANDLIST CommandList,
    D3D12DDI_HRESOURCE Buffer,
    UINT64 AlignedBufferOffset,
    D3D12DDI_PREDICATION_OP Operation)
{
    if (Operation == D3D12DDI_PREDICATION_OP_EQUAL_ZERO) {
        ASSERT(Buffer.pDrvPrivate == NULL);
        ASSERT(AlignedBufferOffset == 0);
    } else {
        TRACE_FUNCTION();
    }
}

void APIENTRY CosUmd12ComputeCommandList_Ddi_IaSetTopology_0003(
    D3D12DDI_HCOMMANDLIST CommandList,
    D3D12DDI_PRIMITIVE_TOPOLOGY Topology)
{
    UNEXPECTED_DDI();
}

void APIENTRY CosUmd12ComputeCommandList_Ddi_RsSetViewports_0003(
    D3D12DDI_HCOMMANDLIST CommandList,
    UINT Count,
    _In_reads_(Count) const D3D12DDI_VIEWPORT* pViewports)
{
    UNEXPECTED_DDI();
}

void APIENTRY CosUmd12ComputeCommandList_Ddi_RsSetScissorRects_0003(
    D3D12DDI_HCOMMANDLIST CommandList,
    UINT Count,
    _In_reads_(Count) const D3D12DDI_RECT* pRects)
{
    UNEXPECTED_DDI();
}

void APIENTRY CosUmd12ComputeCommandList_Ddi_OmSetBlendFactor(
    D3D12DDI_HCOMMANDLIST CommandList,
    const FLOAT BlendFactor[4])
{
    UNEXPECTED_DDI();
}

void APIENTRY CosUmd12ComputeCommandList_Ddi_OmSetStencilRef(
    D3D12DDI_HCOMMANDLIST CommandList,
    UINT StencilRef)
{
    UNEXPECTED_DDI();
}

void APIENTRY CosUmd12ComputeCommandList_Ddi_SetPipelineState(
    D3D12DDI_HCOMMANDLIST CommandList,
    D3D12DDI_HPIPELINESTATE PipelineState)
{
    TRACE_FUNCTION();

    CosUmd12CommandList * pCommandList = CosUmd12CommandList::CastFrom(CommandList);

    if (!pCommandList->IsComputeType())
    {
        UNEXPECTED_DDI();
    }

    CosUmd12PipelineState * pPipelineState = CosUmd12PipelineState::CastFrom(PipelineState);

    pCommandList->SetPipelineState(pPipelineState);
}

void APIENTRY CosUmd12ComputeCommandList_Ddi_SetDescriptorHeaps_0003(
    D3D12DDI_HCOMMANDLIST CommandList,
    _In_ UINT NumDescriptorHeaps,
    _In_reads_(NumDescriptorHeaps) D3D12DDI_HDESCRIPTORHEAP* pDescriptorHeaps)
{
    TRACE_FUNCTION();

    if (NumDescriptorHeaps == 0) {
        // do nothing
    } else {
        TRACE_FUNCTION();

        CosUmd12CommandList * pCommandList = CosUmd12CommandList::CastFrom(CommandList);

        pCommandList->SetDescriptorHeaps(NumDescriptorHeaps, pDescriptorHeaps);
    }
}

void APIENTRY CosUmd12ComputeCommandList_Ddi_SetComputeRootSignature(
    D3D12DDI_HCOMMANDLIST CommandList,
    _In_ D3D12DDI_HROOTSIGNATURE RootSignature)
{
    TRACE_FUNCTION();

    CosUmd12CommandList * pCommandList = CosUmd12CommandList::CastFrom(CommandList);

    pCommandList->SetRootSignature(RootSignature);
}

void APIENTRY CosUmd12ComputeCommandList_Ddi_SetGraphicsRootSignature(
    D3D12DDI_HCOMMANDLIST CommandList,
    _In_ D3D12DDI_HROOTSIGNATURE RootSignature)
{
    UNEXPECTED_DDI();
}

void APIENTRY CosUmd12ComputeCommandList_Ddi_SetComputeRootDescriptorTable(
    D3D12DDI_HCOMMANDLIST CommandList,
    _In_ UINT RootParameterIndex,
    _In_ D3D12DDI_GPU_DESCRIPTOR_HANDLE BaseDescriptor)
{
    TRACE_FUNCTION();

    CosUmd12CommandList * pCommandList = CosUmd12CommandList::CastFrom(CommandList);

    pCommandList->SetRootDescriptorTable(RootParameterIndex, BaseDescriptor);
}

void APIENTRY CosUmd12ComputeCommandList_Ddi_SetGraphicsRootDescriptorTable(
    D3D12DDI_HCOMMANDLIST CommandList,
    _In_ UINT RootParameterIndex,
    _In_ D3D12DDI_GPU_DESCRIPTOR_HANDLE BaseDescriptor)
{
    UNEXPECTED_DDI();
}

void APIENTRY CosUmd12ComputeCommandList_Ddi_SetComputeRoot32BitConstant(
    D3D12DDI_HCOMMANDLIST CommandList,
    UINT RootParameterIndex,
    UINT SrcData,
    UINT DestOffsetIn32BitValues)
{
    TRACE_FUNCTION();

    CosUmd12CommandList * pCommandList = CosUmd12CommandList::CastFrom(CommandList);

    pCommandList->SetRoot32BitConstants(RootParameterIndex, 1, &SrcData, DestOffsetIn32BitValues);
}

void APIENTRY CosUmd12ComputeCommandList_Ddi_SetGraphicsRoot32BitConstant(
    D3D12DDI_HCOMMANDLIST CommandList,
    UINT RootParameterIndex,
    UINT SrcData,
    UINT DestOffsetIn32BitValues)
{
    UNEXPECTED_DDI();
}

void APIENTRY CosUmd12ComputeCommandList_Ddi_SetComputeRoot32BitConstants_0003(
    D3D12DDI_HCOMMANDLIST CommandList,
    UINT RootParameterIndex,
    UINT Num32BitValuesToSet,
    const void* pSrcData,
    UINT DestOffsetIn32BitValues)
{
    TRACE_FUNCTION();

    CosUmd12CommandList * pCommandList = CosUmd12CommandList::CastFrom(CommandList);

    pCommandList->SetRoot32BitConstants(RootParameterIndex, Num32BitValuesToSet, pSrcData, DestOffsetIn32BitValues);
}

void APIENTRY CosUmd12ComputeCommandList_Ddi_SetGraphicsRoot32BitConstants_0003(
    D3D12DDI_HCOMMANDLIST CommandList,
    UINT RootParameterIndex,
    UINT Num32BitValuesToSet,
    const void* pSrcData,
    UINT DestOffsetIn32BitValues)
{
    UNEXPECTED_DDI();
}

void APIENTRY CosUmd12ComputeCommandList_Ddi_SetComputeRootConstantBufferView(
    D3D12DDI_HCOMMANDLIST CommandList,
    UINT RootParameterIndex,
    _In_ D3D12DDI_GPU_VIRTUAL_ADDRESS BufferLocation)
{
    TRACE_FUNCTION();

    //
    // TODO : Test
    //

    CosUmd12CommandList * pCommandList = CosUmd12CommandList::CastFrom(CommandList);

    pCommandList->SetRootView(RootParameterIndex, BufferLocation);
}

void APIENTRY CosUmd12ComputeCommandList_Ddi_SetGraphicsRootConstantBufferView(
    D3D12DDI_HCOMMANDLIST CommandList,
    UINT RootParameterIndex,
    _In_ D3D12DDI_GPU_VIRTUAL_ADDRESS BufferLocation)
{
    UNEXPECTED_DDI();
}

void APIENTRY CosUmd12ComputeCommandList_Ddi_SetComputeRootShaderResourceView(
    D3D12DDI_HCOMMANDLIST CommandList,
    UINT RootParameterIndex,
    _In_ D3D12DDI_GPU_VIRTUAL_ADDRESS BufferLocation)
{
    TRACE_FUNCTION();

    //
    // TODO : Test
    //

    CosUmd12CommandList * pCommandList = CosUmd12CommandList::CastFrom(CommandList);

    pCommandList->SetRootView(RootParameterIndex, BufferLocation);
}

void APIENTRY CosUmd12ComputeCommandList_Ddi_SetGraphicsRootShaderResourceView(
    D3D12DDI_HCOMMANDLIST CommandList,
    UINT RootParameterIndex,
    _In_ D3D12DDI_GPU_VIRTUAL_ADDRESS BufferLocation)
{
    UNEXPECTED_DDI();
}

void APIENTRY CosUmd12ComputeCommandList_Ddi_SetComputeRootUnorderedAccessView(
    D3D12DDI_HCOMMANDLIST CommandList,
    UINT RootParameterIndex,
    _In_ D3D12DDI_GPU_VIRTUAL_ADDRESS BufferLocation)
{
    TRACE_FUNCTION();

    CosUmd12CommandList * pCommandList = CosUmd12CommandList::CastFrom(CommandList);

    pCommandList->SetRootView(RootParameterIndex, BufferLocation);
}

void APIENTRY CosUmd12ComputeCommandList_Ddi_SetGraphicsRootUnorderedAccessView(
    D3D12DDI_HCOMMANDLIST CommandList,
    UINT RootParameterIndex,
    _In_ D3D12DDI_GPU_VIRTUAL_ADDRESS BufferLocation)
{
    UNEXPECTED_DDI();
}

void APIENTRY CosUmd12ComputeCommandList_Ddi_IaSetIndexBuffer(
    D3D12DDI_HCOMMANDLIST CommandList,
    _In_ const D3D12DDI_INDEX_BUFFER_VIEW* pDesc)
{
    UNEXPECTED_DDI();
}

void APIENTRY CosUmd12ComputeCommandList_Ddi_IaSetVertexBuffers_0003(
    D3D12DDI_HCOMMANDLIST CommandList,
    _In_ UINT StartSlot,
    _In_ UINT NumViews,
    _In_reads_opt_(NumViews) const D3D12DDI_VERTEX_BUFFER_VIEW * pViews)
{
    UNEXPECTED_DDI();
}

void APIENTRY CosUmd12ComputeCommandList_Ddi_SoSetTargets_0003(
    D3D12DDI_HCOMMANDLIST CommandList,
    _In_ UINT StartSlot,
    _In_ UINT NumViews,
    _In_reads_opt_(NumViews) const D3D12DDI_STREAM_OUTPUT_BUFFER_VIEW* pViews)
{
    UNEXPECTED_DDI();
}

void APIENTRY CosUmd12ComputeCommandList_Ddi_OmSetRenderTargets_0003(
    D3D12DDI_HCOMMANDLIST CommandList,
    _In_ UINT NumRenderTargetDescriptors,
    _In_ const D3D12DDI_CPU_DESCRIPTOR_HANDLE* pRenderTargetDescriptors,
    _In_ BOOL RTsSingleHandleToDescriptorRange,
    _In_opt_ const D3D12DDI_CPU_DESCRIPTOR_HANDLE* pDepthStencilDescriptor)
{
    UNEXPECTED_DDI();
}

void APIENTRY CosUmd12ComputeCommandList_Ddi_SetMarker(
    D3D12DDI_HCOMMANDLIST CommandList, 
    UINT64 Marker)
{
    TRACE_FUNCTION();
}

void APIENTRY CosUmd12ComputeCommandList_Ddi_ClearRootArguments(
    D3D12DDI_HCOMMANDLIST CommandList)
{
    TRACE_FUNCTION();
}

void APIENTRY CosUmd12ComputeCommandList_Ddi_AtomicCopyBufferRegion_0003(
    D3D12DDI_HCOMMANDLIST CommandList,
    D3D12DDIARG_BUFFER_PLACEMENT Dst,
    D3D12DDIARG_BUFFER_PLACEMENT Src,
    UINT64 SrcBytes)
{
    UNEXPECTED_DDI();
}

void APIENTRY CosUmd12ComputeCommandList_Ddi_OmSetDepthBounds_0025(
    D3D12DDI_HCOMMANDLIST CommandList,
    FLOAT Min,
    FLOAT Max)
{
    UNEXPECTED_DDI();
}

void APIENTRY CosUmd12ComputeCommandList_Ddi_SetSamplePositions_0027(
    D3D12DDI_HCOMMANDLIST CommandList,
    UINT NumSamplesPerPixel,
    UINT NumPixels,
    __in_ecount(NumSamplesPerPixel*NumPixels) D3D12DDI_SAMPLE_POSITION* pSamplePositions)
{
    UNEXPECTED_DDI();
}

void APIENTRY CosUmd12ComputeCommandList_Ddi_ResourceResolveSubresourceRegion(
    D3D12DDI_HCOMMANDLIST CommandList,
    D3D12DDI_HRESOURCE DstResource,
    UINT DstSubresource,
    UINT DstX,
    UINT DstY,
    D3D12DDI_HRESOURCE SrcResource,
    UINT SrcSubresource,
    __in_opt D3D12DDI_RECT* pSrcRect,
    DXGI_FORMAT Format,
    D3D12DDI_RESOLVE_MODE ResolveMode)
{
    UNEXPECTED_DDI();
}

void APIENTRY CosUmd12ComputeCommandList_Ddi_SetProtectedResourceSession_0030(
    D3D12DDI_HCOMMANDLIST CommandList,
    D3D12DDI_HPROTECTEDRESOURCESESSION_0030 ProtectedSession)
{
    UNEXPECTED_DDI();
}

void APIENTRY CosUmd12ComputeCommandList_Ddi_WriteBufferImmediate_0032(
    D3D12DDI_HCOMMANDLIST CommandList,
    UINT Count,
    _In_reads_(Count) CONST D3D12DDI_WRITEBUFFERIMMEDIATE_PARAMETER_0032 *pParams,
    _In_reads_opt_(Count) CONST D3D12DDI_WRITEBUFFERIMMEDIATE_MODE_0032 *pModes)
{
    TRACE_FUNCTION();
}

void APIENTRY CosUmd12ComputeCommandList_Ddi_SetViewInstanceMask_0033(
    D3D12DDI_HCOMMANDLIST CommandList,
    UINT Mask)
{
    UNEXPECTED_DDI();
}

VOID APIENTRY CosUmd12ComputeCommandList_Ddi_InitializeMetaCommand(
    D3D12DDI_HCOMMANDLIST CommandList,
    D3D12DDI_HMETACOMMAND_0052 MetaCommand,
    _In_ CONST void *pvInitializeDesc,
    _In_ SIZE_T InitializeDescSize)
{
    TRACE_FUNCTION();

    CosUmd12CommandList * pCommandList = CosUmd12CommandList::CastFrom(CommandList);
    CosUmd12MetaCommand * pMetaCommand = CosUmd12MetaCommand::CastFrom(MetaCommand);

    pMetaCommand->Initialize(pvInitializeDesc, InitializeDescSize);
}

VOID APIENTRY CosUmd12ComputeCommandList_Ddi_ExecuteMetaCommand(
    D3D12DDI_HCOMMANDLIST CommandList,
    D3D12DDI_HMETACOMMAND_0052 MetaCommand,
    _In_ CONST void *pvExecuteDesc,
    _In_ SIZE_T ExecuteDescSize)
{
    TRACE_FUNCTION();

    CosUmd12CommandList * pCommandList = CosUmd12CommandList::CastFrom(CommandList);
    CosUmd12MetaCommand * pMetaCommand = CosUmd12MetaCommand::CastFrom(MetaCommand);

    pMetaCommand->Execute(pCommandList, pvExecuteDesc, ExecuteDescSize);
}

D3D12DDI_COMMAND_LIST_FUNCS_3D_0052 g_CosUmd12ComputeCommandList_Ddi_0052 =
{
    CosUmd12ComputeCommandList_Ddi_CloseCommandList,                        // pfnCloseCommandList
    CosUmd12ComputeCommandList_Ddi_ResetCommandList_0040,                   // pfnResetCommandList
    CosUmd12ComputeCommandList_Ddi_DrawInstanced,                               // pfnDrawInstanced
    CosUmd12ComputeCommandList_Ddi_DrawIndexedInstanced,                        // pfnDrawIndexedInstanced
    CosUmd12ComputeCommandList_Ddi_Dispatch,                                // pfnDispatch
    CosUmd12ComputeCommandList_Ddi_ClearUnorderedAccessViewUint_0003,       // pfnClearUnorderedAccessViewUint
    CosUmd12ComputeCommandList_Ddi_ClearUnorderedAccessViewFloat_0003,      // pfnClearUnorderedAccessViewFloat
    CosUmd12ComputeCommandList_Ddi_ClearRenderTargetView_0003,                  // pfnClearRenderTargetView
    CosUmd12ComputeCommandList_Ddi_ClearDepthStencilView_0003,                  // pfnClearDepthStencilView
    CosUmd12ComputeCommandList_Ddi_DiscardResource_0003,                    // pfnDiscardResource
    CosUmd12ComputeCommandList_Ddi_CopyTextureRegion_0003,                      // pfnCopyTextureRegion
    CosUmd12ComputeCommandList_Ddi_ResourceCopy,                            // pfnResourceCopy
    CosUmd12ComputeCommandList_Ddi_CopyTiles,                                   // pfnCopyTiles
    CosUmd12ComputeCommandList_Ddi_CopyBufferRegion_0003,                   // pfnCopyBufferRegion
    CosUmd12ComputeCommandList_Ddi_ResourceResolveSubresource,                  // pfnResourceResolveSubresource
    CosUmd12ComputeCommandList_Ddi_ExecuteBundle,                           // pfnExecuteBundle
    CosUmd12ComputeCommandList_Ddi_ExecuteIndirect,                             // pfnExecuteIndirect
    CosUmd12ComputeCommandList_Ddi_ResourceBarrier_0022,                    // pfnResourceBarrier
    CosUmd12ComputeCommandList_Ddi_Blt,                                         // pfnBlt
    CosUmd12ComputeCommandList_Ddi_Present_0051,                                // pfnPresent
    CosUmd12ComputeCommandList_Ddi_BeginQuery_0003,                         // pfnBeginQuery
    CosUmd12ComputeCommandList_Ddi_EndQuery_0003,                           // pfnEndQuery
    CosUmd12ComputeCommandList_Ddi_ResolveQueryData,                        // pfnResolveQueryData
    CosUmd12ComputeCommandList_Ddi_SetPredication,                          // pfnSetPredication
    CosUmd12ComputeCommandList_Ddi_IaSetTopology_0003,                          // pfnIaSetTopology    
    CosUmd12ComputeCommandList_Ddi_RsSetViewports_0003,                         // pfnRsSetViewports
    CosUmd12ComputeCommandList_Ddi_RsSetScissorRects_0003,                      // pfnRsSetScissorRects
    CosUmd12ComputeCommandList_Ddi_OmSetBlendFactor,                            // pfnOmSetBlendFactor
    CosUmd12ComputeCommandList_Ddi_OmSetStencilRef,                             // pfnOmSetStencilRef
    CosUmd12ComputeCommandList_Ddi_SetPipelineState,                        // pfnSetPipelineState
    CosUmd12ComputeCommandList_Ddi_SetDescriptorHeaps_0003,                 // pfnSetDescriptorHeaps
    CosUmd12ComputeCommandList_Ddi_SetComputeRootSignature,                 // pfnSetComputeRootSignature
    CosUmd12ComputeCommandList_Ddi_SetGraphicsRootSignature,                    // pfnSetGraphicsRootSignature
    CosUmd12ComputeCommandList_Ddi_SetComputeRootDescriptorTable,           // pfnSetComputeRootDescriptorTable
    CosUmd12ComputeCommandList_Ddi_SetGraphicsRootDescriptorTable,              // pfnSetGraphicsRootDescriptorTable
    CosUmd12ComputeCommandList_Ddi_SetComputeRoot32BitConstant,             // pfnSetComputeRoot32BitConstant
    CosUmd12ComputeCommandList_Ddi_SetGraphicsRoot32BitConstant,                // pfnSetGraphicsRoot32BitConstant
    CosUmd12ComputeCommandList_Ddi_SetComputeRoot32BitConstants_0003,       // pfnSetComputeRoot32BitConstants
    CosUmd12ComputeCommandList_Ddi_SetGraphicsRoot32BitConstants_0003,          // pfnSetGraphicsRoot32BitConstants
    CosUmd12ComputeCommandList_Ddi_SetComputeRootConstantBufferView,        // pfnSetComputeRootConstantBufferView
    CosUmd12ComputeCommandList_Ddi_SetGraphicsRootConstantBufferView,           // pfnSetGraphicsRootConstantBufferView
    CosUmd12ComputeCommandList_Ddi_SetComputeRootShaderResourceView,        // pfnSetComputeRootShaderResourceView
    CosUmd12ComputeCommandList_Ddi_SetGraphicsRootShaderResourceView,           // pfnSetGraphicsRootShaderResourceView
    CosUmd12ComputeCommandList_Ddi_SetComputeRootUnorderedAccessView,       // pfnSetComputeRootUnorderedAccessView
    CosUmd12ComputeCommandList_Ddi_SetGraphicsRootUnorderedAccessView,          // pfnSetGraphicsRootUnorderedAccessView
    CosUmd12ComputeCommandList_Ddi_IaSetIndexBuffer,                            // pfnIASetIndexBuffer
    CosUmd12ComputeCommandList_Ddi_IaSetVertexBuffers_0003,                     // pfnIASetVertexBuffers
    CosUmd12ComputeCommandList_Ddi_SoSetTargets_0003,                           // pfnSOSetTargets
    CosUmd12ComputeCommandList_Ddi_OmSetRenderTargets_0003,                     // pfnOMSetRenderTargets
    CosUmd12ComputeCommandList_Ddi_SetMarker,                               // pfnSetMarker
    CosUmd12ComputeCommandList_Ddi_ClearRootArguments,                      // pfnClearRootArguments
    CosUmd12ComputeCommandList_Ddi_AtomicCopyBufferRegion_0003,                 // pfnAtomicCopyBufferRegion
    CosUmd12ComputeCommandList_Ddi_OmSetDepthBounds_0025,                       // pfnOMSetDepthBounds
    CosUmd12ComputeCommandList_Ddi_SetSamplePositions_0027,                     // pfnSetSamplePositions
    CosUmd12ComputeCommandList_Ddi_ResourceResolveSubresourceRegion,            // pfnResourceResolveSubresourceRegion
    CosUmd12ComputeCommandList_Ddi_SetProtectedResourceSession_0030,            // pfnSetProtectedResourceSession
    CosUmd12ComputeCommandList_Ddi_WriteBufferImmediate_0032,               // pfnWriteBufferImmediate
    CosUmd12ComputeCommandList_Ddi_SetViewInstanceMask_0033,                    // pfnSetViewInstanceMask
    CosUmd12ComputeCommandList_Ddi_InitializeMetaCommand,                   // pfnInitializeMetaCommand
    CosUmd12ComputeCommandList_Ddi_ExecuteMetaCommand                       // pfnExecuteMetaCommand
};
