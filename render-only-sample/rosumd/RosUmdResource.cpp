///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Resource implementation
//
// Copyright (C) Microsoft Corporation
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "RosUmdDevice.h"
#include "RosUmdResource.h"
#include "RosUmdDebug.h"
#include "RosUmdLogging.h"

#include "RosContext.h"

#include "Vc4Hw.h"

RosUmdResource::RosUmdResource() :
    m_hKMAllocation(NULL)
{
    // do nothing
}

RosUmdResource::~RosUmdResource()
{
    // do nothing
}

void
RosUmdResource::Standup(
    RosUmdDevice *pUmdDevice,
    const D3D11DDIARG_CREATERESOURCE* pCreateResource,
    D3D10DDI_HRTRESOURCE hRTResource)
{
    UNREFERENCED_PARAMETER(pUmdDevice);

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
    m_mostRecentFence = RosUmdCommandBuffer::s_nullFence;

    m_allocationListIndex = 0;

    m_pSysMemCopy = NULL;
}

void
RosUmdResource::Teardown(void)
{
    // TODO[indyz]: Implement
}

void
RosUmdResource::ConstantBufferUpdateSubresourceUP(
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
RosUmdResource::Map(
    RosUmdDevice *pUmdDevice,
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

        pMappedSubRes->RowPitch = m_hwPitchBytes;
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
    //    Currently ROS driver puts all allocations in local video memory.
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

    pMappedSubRes->RowPitch = m_hwPitchBytes;
    pMappedSubRes->DepthPitch = (UINT)m_hwSizeBytes;
}

void
RosUmdResource::Unmap(
    RosUmdDevice *pUmdDevice,
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
RosUmdResource::SetLockFlags(
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
RosUmdResource::CalculateMemoryLayout(
    void)
{
    switch (m_resourceDimension)
    {
    case D3D10DDIRESOURCE_BUFFER:
        {
            m_hwLayout = RosHwLayout::Linear;

            // TODO(bhouse) Need mapping code from resource DXGI format to hw format
            m_hwFormat = RosHwFormat::X8;

            m_hwWidthPixels = m_mip0Info.TexelWidth;
            m_hwHeightPixels = m_mip0Info.TexelHeight;

            assert(m_hwFormat == RosHwFormat::X8);
            assert(m_hwHeightPixels == 1);
            m_hwPitchBytes = m_hwSizeBytes = m_hwWidthPixels;
        }
    break;
    case D3D10DDIRESOURCE_TEXTURE2D:
        {
            if (m_usage == D3D10_DDI_USAGE_DEFAULT)
            {
                m_hwLayout = RosHwLayout::Tiled;
            }
            else
            {
                m_hwLayout = RosHwLayout::Linear;
            }

#if VC4

            // TODO[indyz]: Enable tiled render target
            if ((m_bindFlags & D3D10_DDI_BIND_RENDER_TARGET) ||
                (m_bindFlags & D3D10_DDI_BIND_SHADER_RESOURCE))
            {
                m_hwLayout = RosHwLayout::Linear;
            }

#endif

            // TODO(bhouse) Need mapping code from resource DXGI format to hw format
            if (m_bindFlags & D3D10_DDI_BIND_DEPTH_STENCIL)
            {
                m_hwFormat = RosHwFormat::D24S8;
            }
            else
            {
                m_hwFormat = RosHwFormat::X8888;
            }

            // Using system memory linear MipMap as example
            m_hwWidthPixels = m_mip0Info.TexelWidth;
            m_hwHeightPixels = m_mip0Info.TexelHeight;

#if VC4
            // Align width and height to VC4_BINNING_TILE_PIXELS for binning
#endif

            m_hwWidthTilePixels = VC4_BINNING_TILE_PIXELS;
            m_hwHeightTilePixels = VC4_BINNING_TILE_PIXELS;
            m_hwWidthTiles = (m_hwWidthPixels + m_hwWidthTilePixels - 1) / m_hwWidthTilePixels;
            m_hwHeightTiles = (m_hwHeightPixels + m_hwHeightTilePixels - 1) / m_hwHeightTilePixels;
            m_hwWidthPixels = m_hwWidthTiles*m_hwWidthTilePixels;
            m_hwHeightPixels = m_hwHeightTiles*m_hwHeightTilePixels;

            if (m_hwLayout == RosHwLayout::Linear)
            {
                m_hwSizeBytes = CPixel::ComputeMipMapSize(
                    m_hwWidthPixels,
                    m_hwHeightPixels,
                    m_mipLevels,
                    m_format);

                m_hwPitchBytes = CPixel::ComputeSurfaceStride(
                    m_hwWidthPixels,
                    CPixel::BytesPerPixel(m_format));
            }
            else
            {
                assert(m_hwLayout == RosHwLayout::Tiled);
                assert(m_mipLevels == 1);

                assert((m_hwFormat == RosHwFormat::X8888) || (m_hwFormat == RosHwFormat::D24S8));
                UINT sizeTileBytes = 64 * 64 * 4;

                m_hwSizeBytes = m_hwWidthTiles * m_hwHeightTiles * sizeTileBytes;
                m_hwPitchBytes = 0;
            }
        }
        break;
    case D3D10DDIRESOURCE_TEXTURE1D:
    case D3D10DDIRESOURCE_TEXTURE3D:
    case D3D10DDIRESOURCE_TEXTURECUBE:
        {
            throw RosUmdException(DXGI_DDI_ERR_UNSUPPORTED);
        }
        break;
    }
}

void RosUmdResource::GetAllocationExchange(
    RosAllocationExchange * pOutAllocationExchange)
{
#if 0
    pOutAllocationExchange->m_resourceDimension = m_resourceDimension;
#endif
    pOutAllocationExchange->m_mip0Info = m_mip0Info;
#if 0
    pOutAllocationExchange->m_usage = m_usage;
    pOutAllocationExchange->m_mapFlags = m_mapFlags;
#endif

    pOutAllocationExchange->m_miscFlags = m_miscFlags;
    pOutAllocationExchange->m_bindFlags = m_bindFlags;
    pOutAllocationExchange->m_format = m_format;
    pOutAllocationExchange->m_sampleDesc = m_sampleDesc;
#if 0
    pOutAllocationExchange->m_mipLevels = m_mipLevels;
    pOutAllocationExchange->m_arraySize = m_arraySize;
#endif

    pOutAllocationExchange->m_primaryDesc = m_primaryDesc;
    pOutAllocationExchange->m_hwLayout = m_hwLayout;
    pOutAllocationExchange->m_hwWidthPixels = m_hwWidthPixels;
    pOutAllocationExchange->m_hwHeightPixels = m_hwHeightPixels;
    pOutAllocationExchange->m_hwFormat = m_hwFormat;
#if 0
    pOutAllocationExchange->m_hwPitch = m_hwPitch;
#endif

    pOutAllocationExchange->m_hwSizeBytes = m_hwSizeBytes;
}

bool RosUmdResource::CanRotateFrom(const RosUmdResource* Other) const
{
    // Make sure we're not rotating from ourself and that the resources
    // are compatible (e.g. size, flags, ...)

    return (this != Other) &&
           (!m_pData && !Other->m_pData) &&
           (!m_pSysMemCopy && !Other->m_pSysMemCopy) &&
           (m_hRTResource != Other->m_hRTResource) &&
           (m_hKMAllocation != Other->m_hKMAllocation) &&
           (m_hKMResource != Other->m_hKMResource) &&
           (m_resourceDimension == Other->m_resourceDimension) &&
           (m_mip0Info == Other->m_mip0Info) &&
           (m_usage == Other->m_usage) &&
           (m_bindFlags == Other->m_bindFlags) &&
           (m_bindFlags & D3D10_DDI_BIND_PRESENT) &&
           (m_mapFlags ==Other->m_mapFlags) &&
           (m_miscFlags == Other->m_miscFlags) &&
           (m_format == Other->m_format) &&
           (m_sampleDesc == Other->m_sampleDesc) &&
           (m_mipLevels == Other->m_mipLevels) &&
           (m_arraySize == Other->m_arraySize) &&
           (m_isPrimary == Other->m_isPrimary) &&
           (m_primaryDesc == Other->m_primaryDesc) &&
           (m_hwLayout == Other->m_hwLayout) &&
           (m_hwWidthPixels == Other->m_hwWidthPixels) &&
           (m_hwHeightPixels == Other->m_hwHeightPixels) &&
           (m_hwFormat == Other->m_hwFormat) &&
           (m_hwPitchBytes == Other->m_hwPitchBytes) &&
           (m_hwSizeBytes == Other->m_hwSizeBytes) &&
           (m_hwWidthTilePixels == Other->m_hwWidthTilePixels) &&
           (m_hwHeightTilePixels == Other->m_hwHeightTilePixels) &&
           (m_hwWidthTiles == Other->m_hwWidthTiles) &&
           (m_hwHeightTiles == Other->m_hwHeightTiles) &&
           (m_allocationListIndex == Other->m_allocationListIndex);
}

void RosUmdResource::RotateFrom(const RosUmdResource* Other)
{
    assert(this->CanRotateFrom(Other));

    // Replace our kernel mode allocation handles with the ones from
    // the supplied object
    m_hKMResource = Other->m_hKMResource;
    m_hKMAllocation = Other->m_hKMAllocation;
}
