#include "CosUmd12.h"

void APIENTRY Ddi_CommandQueue_ExecuteCommandLists(
    D3D12DDI_HCOMMANDQUEUE CommandQueue,
    UINT Count,
    _In_reads_(Count) const D3D12DDI_HCOMMANDLIST* pCommandLists)
{
}

void APIENTRY Ddi_CommandQueue_UpdateTileMappings(
    D3D12DDI_HCOMMANDQUEUE CommandQueue,
    D3D12DDI_HRESOURCE Resource,
    UINT NumTiledResourceRegions,
    _In_reads_(NumTiledResourceRegions) const D3D12DDI_TILED_RESOURCE_COORDINATE* pResourceRegionStartCoords,
    _In_reads_opt_(NumTiledResourceRegions) const D3D12DDI_TILE_REGION_SIZE* pResourceRegionSizes,
    D3D12DDI_HHEAP Heap,
    UINT NumRanges,
    _In_reads_opt_(NumRanges) const D3D12DDI_TILE_RANGE_FLAGS* pTileRangeFlags,
    _In_reads_opt_(NumRanges) const UINT* pHeapStartOffsets,
    _In_reads_opt_(NumRanges) const UINT* pRangeTileCounts,
    D3D12DDI_TILE_MAPPING_FLAGS Flags)
{
}

void APIENTRY Ddi_CommandQueue_CopyTileMappings(
    D3D12DDI_HCOMMANDQUEUE CommandQueue,
    D3D12DDI_HRESOURCE DstResource,
    _In_ const D3D12DDI_TILED_RESOURCE_COORDINATE* pDstRegionStartCoord,
    D3D12DDI_HRESOURCE SrcResource,
    _In_ const D3D12DDI_TILED_RESOURCE_COORDINATE* pSrcRegionStartCoord,
    _In_ const D3D12DDI_TILE_REGION_SIZE* pRegionSize,
    D3D12DDI_TILE_MAPPING_FLAGS Flags)
{
}

void APIENTRY Ddi_CommandQueue_SignalFence(
    D3D12DDI_HCOMMANDQUEUE CommandQueue,
    D3D12DDIARG_FENCE_OPERATION* pOperation)
{
}

void APIENTRY Ddi_CommandQueue_WaitForFence(
    D3D12DDI_HCOMMANDQUEUE CommandQueue,
    D3D12DDIARG_FENCE_OPERATION* pOperation)
{
}

D3D12DDI_COMMAND_QUEUE_FUNCS_CORE_0001 g_CosUmd12CommandQueue_Ddi_0001 =
{
    Ddi_CommandQueue_ExecuteCommandLists, // pfnExecuteCommandLists
    nullptr,                              // pfnUnused
    nullptr,                              // pfnUnused2
    Ddi_CommandQueue_UpdateTileMappings,  // pfnUpdateTileMappings
    Ddi_CommandQueue_CopyTileMappings,    // pfnCopyTileMappings
    Ddi_CommandQueue_SignalFence,         // pfnSignalFence
    Ddi_CommandQueue_WaitForFence         // pfnWaitForFence
};
