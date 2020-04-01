#pragma once

#include "RosAllocation.h"
#include "RosUmdUtil.h"
#include "Pixel.hpp"
#include "RosUmdDebug.h"

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

    D3D10DDI_HRTRESOURCE    m_hRTResource;
    D3DKMT_HANDLE           m_hKMResource;      // can this be a D3D10DDI_HKMRESOURCE?
    D3DKMT_HANDLE           m_hKMAllocation;    // can this be a D3D10DDI_HKMALLOCATION?

    ULONGLONG               m_mostRecentFence;
    UINT                    m_allocationListIndex;

    // CPU mapping of the allocation
    BYTE                   *m_pData;

    // Used by constant buffer
    BYTE                   *m_pSysMemCopy;

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

    UINT Pitch () const
    {
        // Pitch is only valid for linear layouts
        assert(m_hwLayout == RosHwLayout::Linear);
        // linear formats are always packed
        return m_hwWidthPixels * CPixel::BytesPerPixel(m_format);
    }

    // Width in T-format 4k tiles
    UINT WidthInTiles () const
    {
        // Only valid in tiled mode
        assert(m_hwLayout == RosHwLayout::Tiled);
        return AlignValue(m_mip0Info.TexelWidth, VC4_4KB_TILE_WIDTH) / 
                VC4_4KB_TILE_WIDTH;
    }

    // Width in T-format 4k tiles
    UINT HeightInTiles () const
    {
        assert(m_hwLayout == RosHwLayout::Tiled);
        return AlignValue(m_mip0Info.TexelHeight, VC4_4KB_TILE_HEIGHT) / 
                VC4_4KB_TILE_HEIGHT;
    }

    UINT WidthInBinningTiles () const
    {
        assert((m_hwWidthPixels % VC4_BINNING_TILE_PIXELS) == 0);
        return m_hwWidthPixels / VC4_BINNING_TILE_PIXELS;
    }

    UINT HeightInBinningTiles () const
    {
        assert((m_hwHeightPixels % VC4_BINNING_TILE_PIXELS) == 0);
        return m_hwHeightPixels / VC4_BINNING_TILE_PIXELS;
    }

    VC4TextureDataType GetVc4TextureType () const
    {
        return Vc4TextureTypeFromDxgiFormat(m_hwLayout, m_format);
    }

    // Tiled textures support
    void ConvertBitmapTo4kTileBlocks(
        BYTE *InputBuffer,
        BYTE *OutBuffer,
        UINT rowStride);

    static void CopyTFormatToLinear (
        _In_reads_(SourceWidthTiles * SourceHeightTiles * VC4_4KB_TILE_SIZE_BYTES) const void* Source,
        UINT SourceWidthTiles,
        UINT SourceHeightTiles,
        _Out_writes_(DestPitch * DestHeight) void* Dest,
        UINT DestPitch,
        UINT DestHeight
        );

private:

    // Tiled textures support
    BYTE *Form1kSubTileBlock(
        BYTE *pInputBuffer,
        BYTE *pOutBuffer,
        UINT rowStride);

    BYTE *Form4kTileBlock(
        BYTE *pInputBuffer,
        BYTE *pOutBuffer,
        UINT rowStride,
        BOOLEAN OddRow);

    struct _LayoutRequirements {
        RosHwLayout Layout;
        UINT PitchAlign;
        UINT HeightAlign;
    };

    // Compute layout and alignment requirements from bind flags
    static _LayoutRequirements Get2dTextureLayoutRequirements (
        UINT BindFlags,
        DXGI_FORMAT Format
        );
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

//
// struct TileCoord
//
// Represents the coordinates of a micro-tile within a T-Format texture
// in terms of tile number (t), sub-tile number (s), and micro-tile number (m)
//
struct TileCoord {
    UINT t;     // 4k tile number
    UINT s;     // 1k sub-tile number
    UINT m;     // microtile number within 1k sub-tile

    //
    // Xm - x coordinate in micro-tiles (Xm = PixelX / 4)
    // Ym - y coordinate in micro-tiles (Ym = PixelY / 4)
    // W - width of T-format texture in 4k tiles
    //
    TileCoord (UINT Xm, UINT Ym, UINT W)
    {
        this->t = ComputeT(Xm, Ym, W);
        this->s = ComputeS(Xm, Ym);
        this->m = ComputeM(Xm, Ym);
    }

    //
    // Get the byte offset into the T-Format texture of the
    // micro-tile at (t, s, m)
    //
    UINT ByteOffset () const
    {
        return this->t * VC4_4KB_TILE_SIZE_BYTES +
            this->s * VC4_1KB_SUB_TILE_SIZE_BYTES +
            this->m * VC4_MICRO_TILE_SIZE_BYTES;
    }

    enum : UINT {
        _Mt = 8,    // width of 4k tile in micro tiles
        _Ms = 4,    // width of 1k sub tile in micro tiles
    };

    // Compute 4k tile number from linear (x, y) coordinates
    static UINT ComputeT (UINT Xm, UINT Ym, UINT W)
    {
        bool evenRow = ((Ym / _Mt) % 2) == 0;
        if (evenRow) {
            return (Xm / _Mt) + W * (Ym / _Mt);
        } else {
            return (W - 1 - (Xm / _Mt)) + W * (Ym / _Mt);
        }
    }

    // Compute 1k sub-tile number from linear (x, y) coordinates
    static UINT ComputeS (UINT Xm, UINT Ym)
    {
        const UINT xs = (Xm % _Mt) / _Ms;
        const UINT ys = (Ym % _Mt) / _Ms;

        bool evenRow = ((Ym / _Mt) % 2) == 0;
        if (evenRow) {
            static const UINT lut[][2] = {0, 1, 3, 2};
            return lut[xs][ys];
        } else {
            static const UINT lut[][2] = {2, 3, 1, 0};
            return lut[xs][ys];
        }
    }

    // Compute microtile number from linear (x, y) coordinates
    static UINT ComputeM (UINT Xm, UINT Ym)
    {
        return (Xm % _Ms) + (Ym % _Ms) * _Ms;
    }
};
