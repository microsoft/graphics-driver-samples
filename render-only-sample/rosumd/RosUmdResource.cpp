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

RosUmdResource::RosUmdResource()
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
        m_primaryDesc = *pCreateResource->pPrimaryDesc;
    }
    else
    {
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
}

void
RosUmdResource::Teardown(void)
{
    // do nothing
}

void
RosUmdResource::Map(
    RosUmdDevice *pUmdDevice,
    UINT subResource,
    D3D10_DDI_MAP mapType,
    UINT mapFlags,
    D3D10DDI_MAPPED_SUBRESOURCE* pMappedSubRes)
{
    assert(m_mipLevels == 1);
    assert(m_arraySize == 1);

    UNREFERENCED_PARAMETER(subResource);

    pUmdDevice->m_commandBuffer.FlushIfMatching(m_mostRecentFence);

    D3DDDICB_LOCK lock;
    memset(&lock, 0, sizeof(lock));

    lock.hAllocation = m_hKMAllocation;
    SetLockFlags(mapType, mapFlags, &lock.Flags);

    pUmdDevice->Lock(&lock);

    pMappedSubRes->pData = lock.pData;

    pMappedSubRes->RowPitch = m_hwPitchBytes;
    pMappedSubRes->DepthPitch = (UINT)m_hwSizeBytes;

}

void
RosUmdResource::Unmap(
    RosUmdDevice *pUmdDevice,
    UINT subResource)
{
    UNREFERENCED_PARAMETER(subResource);

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
            m_hwSizeBytes = m_hwWidthPixels;
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

            // TODO(bhouse) Need mapping code from resource DXGI format to hw format
            m_hwFormat = RosHwFormat::X32;

            // Using system memory linear MipMap as example
            m_hwWidthPixels = m_mip0Info.TexelWidth;
            m_hwHeightPixels = m_mip0Info.TexelHeight;

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

                m_hwWidthTilePixels = 64;
                m_hwHeightTilePixels = 64;
                m_hwWidthTiles = (m_hwWidthPixels + m_hwWidthTilePixels - 1) / m_hwWidthTilePixels;
                m_hwHeightTiles = (m_hwHeightPixels + m_hwHeightTilePixels - 1) / m_hwHeightTilePixels;

                assert(m_hwFormat == RosHwFormat::X32);
                UINT sizeTileBytes = 64 * 64 * 4;

                m_hwSizeBytes = m_hwWidthTiles * m_hwWidthTiles * sizeTileBytes;
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
    pOutAllocationExchange->m_bindFlags = m_bindFlags;
    pOutAllocationExchange->m_mapFlags = m_mapFlags;
    pOutAllocationExchange->m_miscFlags = m_miscFlags;
#endif
    pOutAllocationExchange->m_format = m_format;
    pOutAllocationExchange->m_sampleDesc = m_sampleDesc;
#if 0
    pOutAllocationExchange->m_mipLevels = m_mipLevels;
    pOutAllocationExchange->m_arraySize = m_arraySize;
#endif
    pOutAllocationExchange->m_primaryDesc = m_primaryDesc;
    pOutAllocationExchange->m_hwLayout = m_hwLayout;
#if 0
    pOutAllocationExchange->m_hwWidth = m_hwWidth;
    pOutAllocationExchange->m_hwHeigh = m_hwHeigh;
    pOutAllocationExchange->m_hwFormat = m_hwFormat;
    pOutAllocationExchange->m_hwPitch = m_hwPitch;
#endif

    pOutAllocationExchange->m_hwSizeBytes = m_hwSizeBytes;
}
