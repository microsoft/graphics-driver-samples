#pragma once

#include "RosAllocation.h"
#include "Pixel.hpp"
#include "RosUmdDebug.h"
#include "Vc4Hw.h"

class RosUmdResource : public RosAllocationExchange
{    
    enum class _SIGNATURE {
        CONSTRUCTED = 'Ures',
        INITIALIZED = 'URES',
        DESTRUCTED = 'ures',
    } m_signature;
    
public:

    RosUmdResource();
    ~RosUmdResource();

    static RosUmdResource* CastFrom(D3D10DDI_HRESOURCE);
    static RosUmdResource* CastFrom(DXGI_DDI_HRESOURCE);
    D3D10DDI_HRESOURCE CastTo() const;

    UINT                    m_hwWidthTilePixels;
    UINT                    m_hwHeightTilePixels;
    UINT                    m_hwWidthTiles;
    UINT                    m_hwHeightTiles;

    D3D10DDI_HRTRESOURCE    m_hRTResource;
    D3DKMT_HANDLE           m_hKMResource;      // can this be a D3D10DDI_HKMRESOURCE?
    D3DKMT_HANDLE           m_hKMAllocation;    // can this be a D3D10DDI_HKMALLOCATION?

    ULONGLONG               m_mostRecentFence;
    UINT                    m_allocationListIndex;

    // CPU mapping of the allocation
    BYTE                   *m_pData;

    // Used by constant buffer
    BYTE                   *m_pSysMemCopy;

    // Tiled textures information
    VC4TileInfo m_TileInfo;

    void
    Standup(
        RosUmdDevice *pUmdDevice,
        const D3D11DDIARG_CREATERESOURCE* pCreateResource,
        D3D10DDI_HRTRESOURCE hRTResource);

    void InitSharedResourceFromExistingAllocation (
        const RosAllocationExchange* ExistingAllocationPtr,
        D3D10DDI_HKMRESOURCE hKMResource,
        D3DKMT_HANDLE hKMAllocation,        // can this be a D3D10DDI_HKMALLOCATION?
        D3D10DDI_HRTRESOURCE hRTResource
        );
        
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
    ConstantBufferUpdateSubresourceUP(
        UINT DstSubresource,
        _In_opt_ const D3D10_DDI_BOX *pDstBox,
        _In_ const VOID *pSysMemUP,
        UINT RowPitch,
        UINT DepthPitch,
        UINT CopyFlags);

    void
    SetLockFlags(
        D3D10_DDI_MAP mapType,
        UINT mapFlags,
        D3DDDICB_LOCKFLAGS *pLockFlags);

    void
    CalculateMemoryLayout(
        void);

    // Determines whether the supplied resource can be rotated into this one.
    // Resources must have equivalent dimensions and flags to rotate.
    bool CanRotateFrom(const RosUmdResource* Other) const;

    bool IsPrimary()
    {
        if (m_isPrimary)
        {
            assert(m_miscFlags & D3DWDDM2_0DDI_RESOURCE_MISC_DISPLAYABLE_SURFACE);
            assert(m_bindFlags & D3D10_DDI_BIND_PRESENT);
            assert(m_primaryDesc.ModeDesc.Width != 0);
        }
        return m_isPrimary;
    }

    bool IsMapped()
    {
        return nullptr != m_pData;
    }

    bool IsMultisampled()
    {
        return m_sampleDesc.Count > 1;
    }

    // Support for various texture formats
    void ConvertInitialTextureFormatToInternal(
        const BYTE *pSrc,
        BYTE *pDst,
        UINT rowStride);

private:

    void  ConvertBufferto32Bpp(
        const BYTE *pSrc,
        BYTE *pDst,
        UINT srcBpp,
        UINT swizzleMask,
        UINT pSrcStride,
        UINT pDstStride);

    // Tiled textures support
    void ConvertBitmapTo4kTileBlocks(
        const BYTE *InputBuffer,
        BYTE *OutBuffer,
        UINT rowStride);

    // Tiled textures support
    BYTE *Form1kSubTileBlock(
        const BYTE *pInputBuffer, 
        BYTE *pOutBuffer, 
        UINT rowStride);

    BYTE *Form4kTileBlock(
        const BYTE *pInputBuffer, 
        BYTE *pOutBuffer, 
        UINT rowStride, 
        BOOLEAN OddRow);

    static void MapDxgiFormatToInternalFormats(
        DXGI_FORMAT format,
        _Out_ UINT &bpp,
        _Out_ RosHwFormat &rosFormat);

    void CalculateTilesInfo();

    static VC4TileInfo FillTileInfo(UINT bpp);

};

inline RosUmdResource* RosUmdResource::CastFrom(D3D10DDI_HRESOURCE hResource)
{
    auto thisPtr = static_cast< RosUmdResource* >(hResource.pDrvPrivate);
    if (thisPtr) { assert(thisPtr->m_signature == _SIGNATURE::INITIALIZED); }
    return thisPtr;
}

inline RosUmdResource* RosUmdResource::CastFrom(DXGI_DDI_HRESOURCE hResource)
{
    static_assert(
        sizeof(hResource) == sizeof(RosUmdResource*),
        "Sanity check on cast compatibility");
    auto thisPtr = reinterpret_cast<RosUmdResource*>(hResource);
    if (thisPtr) { assert(thisPtr->m_signature == _SIGNATURE::INITIALIZED); }
    return thisPtr;
}

inline D3D10DDI_HRESOURCE RosUmdResource::CastTo() const
{
    return MAKE_D3D10DDI_HRESOURCE(const_cast< RosUmdResource* >(this));
}
