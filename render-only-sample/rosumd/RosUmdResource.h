#pragma once

#include "RosAllocation.h"
#include "Pixel.hpp"

class RosUmdResource
{
public:

    RosUmdResource();
    ~RosUmdResource();

    static RosUmdResource* CastFrom(D3D10DDI_HRESOURCE);
    D3D10DDI_HRESOURCE CastTo() const;

    // Input from UMD CreateResource DDI
    D3D10DDIRESOURCE_TYPE   m_resourceDimension;

    D3D10DDI_MIPINFO        m_mip0Info;
    UINT                    m_usage;        // D3D10_DDI_RESOURCE_USAGE
    UINT                    m_bindFlags;    // D3D10_DDI_RESOURCE_BIND_FLAG
    UINT                    m_mapFlags;     // D3D10_DDI_MAP
    UINT                    m_miscFlags;    // D3D10_DDI_RESOURCE_MISC_FLAG
    DXGI_FORMAT             m_format;
    DXGI_SAMPLE_DESC        m_sampleDesc;
    UINT                    m_mipLevels;
    UINT                    m_arraySize;

    DXGI_DDI_PRIMARY_DESC   m_primaryDesc;

    // HW specific information calculated based on the fields above
    RosHwLayout             m_hwLayout;

    UINT                    m_hwWidthPixels;
    UINT                    m_hwHeightPixels;
    RosHwFormat             m_hwFormat;
    UINT                    m_hwPitchBytes;
    size_t                  m_hwSizeBytes;

    UINT                    m_hwWidthTilePixels;
    UINT                    m_hwHeightTilePixels;
    UINT                    m_hwWidthTiles;
    UINT                    m_hwHeightTiles;

    D3D10DDI_HRTRESOURCE    m_hRTResource;
    D3DKMT_HANDLE           m_hKMResource;
    D3DKMT_HANDLE           m_hKMAllocation;

    ULONGLONG               m_mostRecentFence;
    UINT                    m_allocationListIndex;

    // Used by constant buffer
    BYTE                   *m_pSysMemCopy;

    void
    Standup(
        RosUmdDevice *pUmdDevice,
        const D3D11DDIARG_CREATERESOURCE* pCreateResource,
        D3D10DDI_HRTRESOURCE hRTResource);

    void Teardown(void);

    void
    Map(
        RosUmdDevice *pUmdDevice,
        UINT subResource,
        D3D10_DDI_MAP mapType,
        UINT mapFlags,
        D3D10DDI_MAPPED_SUBRESOURCE* mappedSubRes);

    void
    Unmap(
        RosUmdDevice *pUmdDevice,
        UINT subResource);

    void
    SetLockFlags(
        D3D10_DDI_MAP mapType,
        UINT mapFlags,
        D3DDDICB_LOCKFLAGS *pLockFlags);

    void
    CalculateMemoryLayout(
        void);

    void GetAllocationExchange(
        RosAllocationExchange * pOutAllocationExchange);
};

inline RosUmdResource* RosUmdResource::CastFrom(D3D10DDI_HRESOURCE hResource)
{
    return static_cast< RosUmdResource* >(hResource.pDrvPrivate);
}

inline D3D10DDI_HRESOURCE RosUmdResource::CastTo() const
{
    return MAKE_D3D10DDI_HRESOURCE(const_cast< RosUmdResource* >(this));
}
