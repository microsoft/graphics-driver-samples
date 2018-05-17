///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Resource implementation
//
// Copyright (C) Microsoft Corporation
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "CosUmd.h"

#include "CosUmdLogging.h"
#include "CosUmdResource.tmh"

#include "CosUmdDevice.h"
#include "CosUmdResource.h"
#include "CosUmdDebug.h"

#include "CosContext.h"

#if VC4
#include "Vc4Hw.h"
#endif

CosUmdResource::CosUmdResource() :
    m_signature(_SIGNATURE::CONSTRUCTED),
    m_hKMAllocation(NULL)
{
    // do nothing
}

CosUmdResource::~CosUmdResource()
{
    assert(
        (m_signature == _SIGNATURE::CONSTRUCTED) ||
        (m_signature == _SIGNATURE::INITIALIZED));
    // do nothing
}

void
CosUmdResource::Standup(
    CosUmdDevice *pUmdDevice,
    const D3D11DDIARG_CREATERESOURCE* pCreateResource,
    D3D10DDI_HRTRESOURCE hRTResource)
{
    UNREFERENCED_PARAMETER(pUmdDevice);

    assert(m_signature == _SIGNATURE::CONSTRUCTED);

    m_resourceDimension = pCreateResource->ResourceDimension;
    m_mip0Info = *pCreateResource->pMipInfoList;
    m_usage = pCreateResource->Usage;
    m_bindFlags = pCreateResource->BindFlags;
    m_mapFlags = pCreateResource->MapFlags;
    m_miscFlags = pCreateResource->MiscFlags;
    m_format = pCreateResource->Format;
    m_sampleDesc = pCreateResource->SampleDesc;
    m_mipLevels = pCreateResource->MipLevels;
    m_arraySize = pCreateResource->ArraySize;

    if (pCreateResource->pPrimaryDesc)
    {
        assert(
            (pCreateResource->MiscFlags & D3DWDDM2_0DDI_RESOURCE_MISC_DISPLAYABLE_SURFACE) &&
            (pCreateResource->BindFlags & D3D10_DDI_BIND_PRESENT) &&
            (pCreateResource->pPrimaryDesc->ModeDesc.Width != 0));

        m_isPrimary = true;
        m_primaryDesc = *pCreateResource->pPrimaryDesc;
    }
    else
    {
        m_isPrimary = false;
        ZeroMemory(&m_primaryDesc, sizeof(m_primaryDesc));
    }

    CalculateMemoryLayout();

    m_hRTResource = hRTResource;

    // Zero out internal state
    m_hKMResource = 0;
    m_hKMAllocation = 0;

    // Mark that the resource is not referenced by a command buffer (.i.e. null fence value)
    m_mostRecentFence = CosUmdCommandBuffer::s_nullFence;

    m_allocationListIndex = 0;

    m_pData = nullptr;
    m_pSysMemCopy = nullptr;
    m_signature = _SIGNATURE::INITIALIZED;
}

void CosUmdResource::InitSharedResourceFromExistingAllocation (
    const CosAllocationExchange* ExistingAllocationPtr,
    D3D10DDI_HKMRESOURCE hKMResource,
    D3DKMT_HANDLE hKMAllocation,        // can this be a D3D10DDI_HKMALLOCATION?
    D3D10DDI_HRTRESOURCE hRTResource
    )
{
    assert(m_signature == _SIGNATURE::CONSTRUCTED);

    COS_LOG_TRACE(
        "Opening existing resource. "
        "(ExistingAllocationPtr->m_hwWidth/HeightPixels = %u,%u  "
        "ExistingAllocationPtr->m_hwSizeBytes = %u, "
        "ExistingAllocationPtr->m_isPrimary = %d, "
        "hRTResource = 0x%p, "
        "hKMResource= 0x%x, "
        "hKMAllocation = 0x%x)",
        ExistingAllocationPtr->m_hwWidthPixels,
        ExistingAllocationPtr->m_hwHeightPixels,
        ExistingAllocationPtr->m_hwSizeBytes,
        ExistingAllocationPtr->m_isPrimary,
        hRTResource.handle,
        hKMResource.handle,
        hKMAllocation);

    // copy members from the existing allocation into this object
    CosAllocationExchange* basePtr = this;
    *basePtr = *ExistingAllocationPtr;

    // HW specific information calculated based on the fields above
    CalculateMemoryLayout();

    NT_ASSERT(
        (m_hwLayout == ExistingAllocationPtr->m_hwLayout) &&
        (m_hwWidthPixels == ExistingAllocationPtr->m_hwWidthPixels) &&
        (m_hwHeightPixels == ExistingAllocationPtr->m_hwHeightPixels) &&
        (m_hwSizeBytes == ExistingAllocationPtr->m_hwSizeBytes));

    m_hRTResource = hRTResource;
    m_hKMResource = hKMResource.handle;
    m_hKMAllocation = hKMAllocation;

    m_mostRecentFence = CosUmdCommandBuffer::s_nullFence;
    m_allocationListIndex = 0;

    m_pData = nullptr;
    m_pSysMemCopy = nullptr;

    m_signature = _SIGNATURE::INITIALIZED;
}

void
CosUmdResource::Teardown(void)
{
    m_signature = _SIGNATURE::CONSTRUCTED;
    // TODO[indyz]: Implement
}

void
CosUmdResource::ConstantBufferUpdateSubresourceUP(
    UINT DstSubresource,
    _In_opt_ const D3D10_DDI_BOX *pDstBox,
    _In_ const VOID *pSysMemUP,
    UINT RowPitch,
    UINT DepthPitch,
    UINT CopyFlags)
{
    assert(DstSubresource == 0);
    assert(pSysMemUP);

    assert(m_bindFlags & D3D10_DDI_BIND_CONSTANT_BUFFER); // must be constant buffer
    assert(m_resourceDimension == D3D10DDIRESOURCE_BUFFER);

    BYTE *pSysMemCopy = m_pSysMemCopy;
    UINT BytesToCopy = RowPitch;
    if (pDstBox)
    {
        if (pDstBox->left < 0 ||
            pDstBox->left > (INT)m_hwSizeBytes ||
            pDstBox->left > pDstBox->right ||
            pDstBox->right > (INT)m_hwSizeBytes)
        {
            return; // box is outside of buffer size. Nothing to copy.
        }

        pSysMemCopy += pDstBox->left;
        BytesToCopy = (pDstBox->right - pDstBox->left);
    }
    else if (BytesToCopy == 0)
    {
        BytesToCopy = m_hwSizeBytes; // copy whole.
    }
    else
    {
        BytesToCopy = min(BytesToCopy, m_hwSizeBytes);
    }

    CopyMemory(pSysMemCopy, pSysMemUP, BytesToCopy);

    return;

    DepthPitch;
    CopyFlags;
}

void
CosUmdResource::Map(
    CosUmdDevice *pUmdDevice,
    UINT subResource,
    D3D10_DDI_MAP mapType,
    UINT mapFlags,
    D3D10DDI_MAPPED_SUBRESOURCE* pMappedSubRes)
{
    assert(m_mipLevels <= 1);
    assert(m_arraySize == 1);

    UNREFERENCED_PARAMETER(subResource);

    //
    // Constant data is copied into command buffer, so there is no need for flushing
    //

    if (m_bindFlags & D3D10_DDI_BIND_CONSTANT_BUFFER)
    {
        pMappedSubRes->pData = m_pSysMemCopy;

        pMappedSubRes->RowPitch = this->Pitch();
        pMappedSubRes->DepthPitch = (UINT)m_hwSizeBytes;

        return;
    }

    pUmdDevice->m_commandBuffer.FlushIfMatching(m_mostRecentFence);

    D3DDDICB_LOCK lock;
    memset(&lock, 0, sizeof(lock));

    lock.hAllocation = m_hKMAllocation;

    //
    // TODO[indyz]: Consider how to optimize D3D10_DDI_MAP_WRITE_NOOVERWRITE
    //
    //    D3DDDICB_LOCKFLAGS::IgnoreSync and IgnoreReadSync are used for
    //    D3D10_DDI_MAP_WRITE_NOOVERWRITE optimization and are only allowed
    //    for allocations that can resides in aperture segment.
    //
    //    Currently COS driver puts all allocations in local video memory.
    //

    SetLockFlags(mapType, mapFlags, &lock.Flags);

    pUmdDevice->Lock(&lock);

    if (lock.Flags.Discard)
    {
        assert(m_hKMAllocation != lock.hAllocation);

        m_hKMAllocation = lock.hAllocation;

        if (pUmdDevice->m_commandBuffer.IsResourceUsed(this))
        {
            //
            // Indicate that the new allocation instance of the resource
            // is not used in the current command batch.
            //

            m_mostRecentFence -= 1;
        }
    }

    pMappedSubRes->pData = lock.pData;
    m_pData = (BYTE*)lock.pData;

    pMappedSubRes->RowPitch = this->Pitch();
    pMappedSubRes->DepthPitch = (UINT)m_hwSizeBytes;
}

void
CosUmdResource::Unmap(
    CosUmdDevice *pUmdDevice,
    UINT subResource)
{
    UNREFERENCED_PARAMETER(subResource);

    if (m_bindFlags & D3D10_DDI_BIND_CONSTANT_BUFFER)
    {
        return;
    }

    m_pData = NULL;

    D3DDDICB_UNLOCK unlock;
    memset(&unlock, 0, sizeof(unlock));

    unlock.NumAllocations = 1;
    unlock.phAllocations = &m_hKMAllocation;

    pUmdDevice->Unlock(&unlock);
}

void
CosUmdResource::SetLockFlags(
    D3D10_DDI_MAP mapType,
    UINT mapFlags,
    D3DDDICB_LOCKFLAGS *pLockFlags)
{
    switch (mapType)
    {
    case D3D10_DDI_MAP_READ:
        pLockFlags->ReadOnly = 1;
        break;
    case D3D10_DDI_MAP_WRITE:
        pLockFlags->WriteOnly = 1;
        break;
    case D3D10_DDI_MAP_READWRITE:
        break;
    case D3D10_DDI_MAP_WRITE_DISCARD:
        pLockFlags->Discard = 1;
    case D3D10_DDI_MAP_WRITE_NOOVERWRITE:
        break;
    }

    if (mapFlags & D3D10_DDI_MAP_FLAG_DONOTWAIT)
    {
        pLockFlags->DonotWait = 1;
    }
}

void
CosUmdResource::CalculateMemoryLayout(
    void)
{
    switch (m_resourceDimension)
    {
    case D3D10DDIRESOURCE_BUFFER:
        {
            m_hwLayout = CosHwLayout::Linear;

            // TODO(bhouse) Need mapping code from resource DXGI format to hw format
            m_hwWidthPixels = m_mip0Info.TexelWidth;
            m_hwHeightPixels = m_mip0Info.TexelHeight;

            m_hwSizeBytes = m_mip0Info.TexelWidth * CPixel::BytesPerPixel(m_format);
            NT_ASSERT(this->Pitch() == m_hwSizeBytes);
        }
    break;
    case D3D10DDIRESOURCE_TEXTURE2D:
        {
            // get layout and alignment requirement from binding and format
            const auto reqs =
                Get2dTextureLayoutRequirements(m_bindFlags, m_format);

            const UINT unalignedPitch =
                m_mip0Info.TexelWidth * CPixel::BytesPerPixel(m_format);
            const UINT alignedPitch = 
                AlignValue(unalignedPitch, reqs.PitchAlign);

            const UINT unalignedHeight = m_mip0Info.TexelHeight;
            const UINT alignedHeight =
                AlignValue(unalignedHeight, reqs.HeightAlign);

            m_hwLayout = reqs.Layout;
            m_hwWidthPixels = alignedPitch / CPixel::BytesPerPixel(m_format);
            m_hwHeightPixels = alignedHeight;
            m_hwSizeBytes = alignedPitch * alignedHeight;
        }
        break;
    case D3D10DDIRESOURCE_TEXTURE1D:
    case D3D10DDIRESOURCE_TEXTURE3D:
    case D3D10DDIRESOURCE_TEXTURECUBE:
    default:
        throw CosUmdException(DXGI_DDI_ERR_UNSUPPORTED);
    }
}

bool CosUmdResource::CanRotateFrom(const CosUmdResource* Other) const
{
    // Make sure we're not rotating from ourself and that the resources
    // are compatible (e.g. size, flags, ...)

    return (this != Other) &&
           (!m_pData && !Other->m_pData) &&
           (!m_pSysMemCopy && !Other->m_pSysMemCopy) &&
           (m_hRTResource != Other->m_hRTResource) &&
           ((m_hKMAllocation != Other->m_hKMAllocation) || !m_hKMAllocation) &&
           ((m_hKMResource != Other->m_hKMResource) || !m_hKMResource) &&
           (m_resourceDimension == Other->m_resourceDimension) &&
           (m_mip0Info == Other->m_mip0Info) &&
           (m_usage == Other->m_usage) &&
           (m_bindFlags == Other->m_bindFlags) &&
           (m_bindFlags & D3D10_DDI_BIND_PRESENT) &&
           (m_mapFlags == Other->m_mapFlags) &&
           (m_miscFlags == Other->m_miscFlags) &&
           (m_format == Other->m_format) &&
           (m_sampleDesc == Other->m_sampleDesc) &&
           (m_mipLevels == Other->m_mipLevels) &&
           (m_arraySize == Other->m_arraySize) &&
           (m_isPrimary == Other->m_isPrimary) &&
           ((m_primaryDesc.Flags & ~DXGI_DDI_PRIMARY_OPTIONAL) ==
            (Other->m_primaryDesc.Flags & ~DXGI_DDI_PRIMARY_OPTIONAL)) &&
           (m_primaryDesc.VidPnSourceId == Other->m_primaryDesc.VidPnSourceId) &&
           (m_primaryDesc.ModeDesc == Other->m_primaryDesc.ModeDesc) &&
           (m_primaryDesc.DriverFlags == Other->m_primaryDesc.DriverFlags) &&
           (m_hwLayout == Other->m_hwLayout) &&
           (m_hwWidthPixels == Other->m_hwWidthPixels) &&
           (m_hwHeightPixels == Other->m_hwHeightPixels) &&
           (m_hwSizeBytes == Other->m_hwSizeBytes);
}

#if VC4
// Form 1k sub-tile block
BYTE *CosUmdResource::Form1kSubTileBlock(BYTE *pInputBuffer, BYTE *pOutBuffer, UINT rowStride)
{
    // 1k sub-tile block is formed from micro-tiles blocks
    for (UINT h = 0; h < VC4_1KB_SUB_TILE_HEIGHT; h += 4)
    {
        BYTE *currentBufferPos = pInputBuffer + h*rowStride;

        // Process row of 4 micro-tiles blocks
        for (UINT w = 0; w < VC4_1KB_SUB_TILE_WIDTH_BYTES; w+= VC4_MICRO_TILE_WIDTH_BYTES)
        {
            BYTE *microTileOffset = currentBufferPos + w;

            // Process micro-tile block (4x16 bytes)
            for (int t = 0; t < VC4_MICRO_TILE_HEIGHT; t++)
            {
                memcpy(pOutBuffer, microTileOffset, VC4_MICRO_TILE_WIDTH_BYTES);
                pOutBuffer += VC4_MICRO_TILE_WIDTH_BYTES;
                microTileOffset += rowStride;
            }
        }
    }
    return pOutBuffer;
}

// Form one 4k tile block from pInputBuffer and store in pOutBuffer
BYTE *CosUmdResource::Form4kTileBlock(BYTE *pInputBuffer, BYTE *pOutBuffer, UINT rowStride, BOOLEAN OddRow)
{
    BYTE *currentTileOffset = NULL;

    if (OddRow)
    {
        // For even rows, process sub-tile blocks in ABCD order, where
        // each sub-tile is stored in memory as follows:
        //
        //  [C  B]
        //  [D  A]
        //

        // Get A block
        currentTileOffset = pInputBuffer + rowStride * VC4_1KB_SUB_TILE_HEIGHT + VC4_1KB_SUB_TILE_WIDTH_BYTES;
        pOutBuffer = Form1kSubTileBlock(currentTileOffset, pOutBuffer, rowStride);

        // Get B block
        currentTileOffset = pInputBuffer + VC4_1KB_SUB_TILE_WIDTH_BYTES;

        pOutBuffer = Form1kSubTileBlock(currentTileOffset, pOutBuffer, rowStride);

        // Get C block
        pOutBuffer = Form1kSubTileBlock(pInputBuffer, pOutBuffer, rowStride);

        // Get D block
        currentTileOffset = pInputBuffer + rowStride * VC4_1KB_SUB_TILE_HEIGHT;
        pOutBuffer = Form1kSubTileBlock(currentTileOffset, pOutBuffer, rowStride);

        // return current position in out buffer
        return pOutBuffer;

    }
    else
    {
        // For even rows, process sub-tile blocks in ABCD order, where
        // each sub-tile is stored in memory as follows:
        //
        //  [A  D]
        //  [B  C]
        //

        // Get A block
        pOutBuffer = Form1kSubTileBlock(pInputBuffer, pOutBuffer, rowStride);

        /// Get B block
        currentTileOffset = pInputBuffer + rowStride * VC4_1KB_SUB_TILE_HEIGHT;
        pOutBuffer = Form1kSubTileBlock(currentTileOffset, pOutBuffer, rowStride);

        // Get C Block
        currentTileOffset = pInputBuffer + rowStride * VC4_1KB_SUB_TILE_HEIGHT + VC4_1KB_SUB_TILE_WIDTH_BYTES;
        pOutBuffer = Form1kSubTileBlock(currentTileOffset, pOutBuffer, rowStride);

        // Get D block
        currentTileOffset = pInputBuffer + VC4_1KB_SUB_TILE_WIDTH_BYTES;
        pOutBuffer = Form1kSubTileBlock(currentTileOffset, pOutBuffer, rowStride);

        // return current position in out buffer
        return pOutBuffer;
    }
}

// Form (CountX * CountY) tile blocks from InputBuffer and store them in OutBuffer
void CosUmdResource::ConvertBitmapTo4kTileBlocks(BYTE *InputBuffer, BYTE *OutBuffer, UINT rowStride)
{
    // [todo] Currently only 32bpp mode is supported
    UINT CountX = this->WidthInTiles();
    UINT CountY = this->HeightInTiles();

    for (UINT k = 0; k < CountY; k++)
    {
        BOOLEAN oddRow = k & 1;
        if (oddRow)
        {
            // Build 4k blocks from right to left for odd rows
            for (int i = CountX - 1; i >= 0; i--)
            {
                BYTE *blockStartOffset = InputBuffer + k * rowStride * VC4_4KB_TILE_HEIGHT + i * VC4_4KB_TILE_WIDTH_BYTES;
                OutBuffer = Form4kTileBlock(blockStartOffset, OutBuffer, rowStride, oddRow);
            }
        }
        else
        {
            // Build 4k blocks from left to right for even rows
            for (UINT i = 0; i < CountX; i++)
            {
                BYTE *blockStartOffset = InputBuffer + k * rowStride * VC4_4KB_TILE_HEIGHT + i * VC4_4KB_TILE_WIDTH_BYTES;
                OutBuffer = Form4kTileBlock(blockStartOffset, OutBuffer, rowStride, oddRow);
            }
        }
    }
}

// given an xy microtile

_Use_decl_annotations_
void CosUmdResource::CopyTFormatToLinear (
    const void* Source,
    UINT SourceWidthTiles,  // width in 4k tiles
    UINT SourceHeightTiles, // height in 4k tiles
    void* Dest,
    UINT DestPitch,
    UINT DestHeight
    )
{
    if (DestPitch % VC4_MICRO_TILE_WIDTH_BYTES) {
        COS_LOG_ERROR(
            "Destination pitch is not aligned to microtile width. "
            "(DestPitch = %d, VC4_MICRO_TILE_WIDTH_BYTES = %d)",
            DestPitch,
            VC4_MICRO_TILE_WIDTH_BYTES);
        throw CosUmdException(E_INVALIDARG);
    }

    const BYTE* const destEnd = static_cast<BYTE*>(Dest) + (DestPitch * DestHeight);
    const BYTE* const srcEnd = static_cast<const BYTE*>(Source) +
        (SourceWidthTiles * SourceHeightTiles * VC4_4KB_TILE_SIZE_BYTES);

    // build destination image in scanline order
    for (UINT y = 0; y < DestHeight; ++y) {
        const UINT offsetWithinMicrotile =
            (y % VC4_MICRO_TILE_HEIGHT) * VC4_MICRO_TILE_WIDTH_BYTES;

        // destRow points to beginning of row y
        BYTE* const destRow = static_cast<BYTE*>(Dest) + y * DestPitch;

        for (UINT x = 0; x < DestPitch; x += VC4_MICRO_TILE_WIDTH_BYTES) {
            TileCoord tileCoord(
                x / VC4_MICRO_TILE_WIDTH_BYTES,
                y / VC4_MICRO_TILE_HEIGHT,
                SourceWidthTiles);

            // Compute the start of the line within the micro tile
            const BYTE* src =
                static_cast<const BYTE*>(Source) +
                tileCoord.ByteOffset() +
                offsetWithinMicrotile;
            NT_ASSERT((src + VC4_MICRO_TILE_WIDTH_BYTES) <= srcEnd);
            UNREFERENCED_PARAMETER(srcEnd);

            BYTE* dest = destRow + x;
            NT_ASSERT((dest + VC4_MICRO_TILE_WIDTH_BYTES) <= destEnd);
            UNREFERENCED_PARAMETER(destEnd);

            memcpy(dest, src, VC4_MICRO_TILE_WIDTH_BYTES);
        }
    }
}
#endif

CosUmdResource::_LayoutRequirements CosUmdResource::Get2dTextureLayoutRequirements (
    UINT BindFlags,
    DXGI_FORMAT Format
    )
{
    CosHwLayout hwLayout;
    UINT pitchAlign, heightAlign;

    Format;

    switch (BindFlags)
    {
    case 0:
    case D3D10_DDI_BIND_RENDER_TARGET:
    case D3D10_DDI_BIND_RENDER_TARGET | D3D10_DDI_BIND_SHADER_RESOURCE:
    case D3D10_DDI_BIND_RENDER_TARGET | D3D10_DDI_BIND_PRESENT:
    case D3D10_DDI_BIND_RENDER_TARGET | D3D10_DDI_BIND_SHADER_RESOURCE | D3D10_DDI_BIND_PRESENT:
    case D3D10_DDI_BIND_SHADER_RESOURCE:
    case D3D10_DDI_BIND_DEPTH_STENCIL:
    case D3D10_DDI_BIND_PRESENT:
        hwLayout = CosHwLayout::Linear;
        pitchAlign = 1;
        heightAlign = 1;
        break;

    case D3D10_DDI_BIND_VERTEX_BUFFER:
    case D3D10_DDI_BIND_INDEX_BUFFER:
    case D3D10_DDI_BIND_CONSTANT_BUFFER:
        COS_LOG_ERROR(
            "Unsupported bind flag for 2D texture. (BindFlags = 0x%x)",
            BindFlags);
        throw CosUmdException(E_INVALIDARG);
    case D3D10_DDI_BIND_STREAM_OUTPUT:
    default:
        COS_LOG_ERROR("Unsupported or invalid bind flags: 0x%x", BindFlags);
        throw CosUmdException(E_INVALIDARG);
    }

    return _LayoutRequirements{hwLayout, pitchAlign, heightAlign};
}
