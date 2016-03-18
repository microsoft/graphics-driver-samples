/*==========================================================================;
 *
 *  Copyright (C) 1999-2002 Microsoft Corporation.  All Rights Reserved.
 *
 *  File:       pixel.hpp
 *  Content:    Utility class for working with pixel formats
 *
 *
 ***************************************************************************/

#ifdef __cplusplus

#ifndef __PIXEL_HPP_CPP__
#define __PIXEL_HPP_CPP__

#ifndef DXPIXELVER
#define DXPIXELVER 100
#endif

#ifndef DXGASSERT
#define DXGASSERT( x ) _Analysis_assume_( x );
#endif

struct IHVFormatInfo
{
    DXGI_FORMAT     m_Format;
    DWORD           m_BPP;
    IHVFormatInfo  *m_pNext;
};

//2GB max allocation size
#define MAXALLOCSIZE 0x80000000

// This is a utility class that implements useful helpers for
// allocating and accessing various pixel formats. All methods
// are static and hence should be accessed as follows:
//  e.g. CPixel::LockOffset(...)
//

class CPixel
{
public:
    // Allocate helpers

    // Determine the amount of memory that is needed to
    // allocate various things..
    static UINT ComputeSurfaceSize(UINT            cpWidth,
                                   UINT            cpHeight,
                                   DXGI_FORMAT     Format);

    static UINT ComputeVolumeSize(UINT             cpWidth,
                                  UINT             cpHeight,
                                  UINT             cpDepth,
                                  DXGI_FORMAT      Format);


    static UINT ComputeMipMapSize(UINT             cpWidth,
                                  UINT             cpHeight,
                                  UINT             cLevels,
                                  DXGI_FORMAT      Format);

    static UINT ComputeMipVolumeSize(UINT          cpWidth,
                                     UINT          cpHeight,
                                     UINT          cpDepth,
                                     UINT          cLevels,
                                     DXGI_FORMAT   Format);

    static BOOL ComputeSurfaceSizeChecked(UINT            cpWidth,
                                          UINT            cpHeight,
                                          DXGI_FORMAT    Format,
                                          UINT            *pSize);

    static BOOL ComputeVolumeSizeChecked(UINT             cpWidth,
                                         UINT             cpHeight,
                                         UINT             cpDepth,
                                         DXGI_FORMAT      Format,
                                         UINT             *pSize);


    static BOOL ComputeMipMapSizeChecked(UINT             cpWidth,
                                         UINT             cpHeight,
                                         UINT             cLevels,
                                         DXGI_FORMAT      Format,
                                         UINT             *pSize);

    static BOOL ComputeMipVolumeSizeChecked(UINT          cpWidth,
                                            UINT          cpHeight,
                                            UINT          cpDepth,
                                            UINT          cLevels,
                                            DXGI_FORMAT   Format,
                                            UINT          *pSize);


    // Lock helpers

    // Given a surface desc, a level, and pointer to
    // bits (pBits in the LockedRectData) and a sub-rect,
    // this will fill in the pLockedRectData structure
    static void ComputeMipMapOffset(UINT                            cpWidth,
                                    UINT                            cpHeight,
                                    DXGI_FORMAT                     Format,
                                    UINT                            iLevel,
                                    BYTE                           *pBits,
                                    CONST RECT                     *pRect,
                                    D3D10DDI_MAPPED_SUBRESOURCE    *pLockedRectData);

    // MipVolume version of ComputeMipMapOffset
    static void ComputeMipVolumeOffset(UINT                         cpWidth,
                                       UINT                         cpHeight,
                                       UINT                         cpDepth,
                                       DXGI_FORMAT                  Format,
                                       UINT                         iLevel,
                                       BYTE                        *pBits,
                                       CONST D3DBOX                *pBox,
                                       D3D10DDI_MAPPED_SUBRESOURCE *pLockedBoxData);

    // Surface version of ComputeMipMapOffset
    static void ComputeSurfaceOffset(UINT                           cpWidth,
                                     UINT                           cpHeight,
                                     DXGI_FORMAT                    Format,
                                     BYTE                          *pBits,
                                     CONST RECT                    *pRect,
                                     D3D10DDI_MAPPED_SUBRESOURCE   *pLockedRectData);

    // Pixel Stride will return negative for DXT formats
    // Call AdjustForDXT to work with things at the block level
    static UINT ComputePixelStride(DXGI_FORMAT Format);

    // This will adjust cbPixel
    // to pixels per block; and width and height will
    // be adjusted to pixels. Assumes the IsDXT(cbPixel).
    static void AdjustForDXT(UINT *pcpWidth,
                             UINT *pcpHeight,
                             UINT *pcbPixel);

    // returns TRUE if cbPixel is "negative" i.e. DXT/V group
    static BOOL IsDXT(UINT cbPixel);

    // returns TRUE if format is one of the DXT/V group
    static BOOL IsDXT(DXGI_FORMAT Format);

    static UINT BytesPerPixel(DXGI_FORMAT Format);

    // Register format for later lookup
    static HRESULT Register(DXGI_FORMAT Format, DWORD BPP);

    // Cleanup registry
    static void Cleanup();

    static UINT ComputeSurfaceStride(UINT cpWidth, UINT cbPixel);

    static BOOL ComputeSurfaceStrideChecked(UINT cpWidth, UINT cbPixel, UINT *pStride);


private:
    // Internal functions

    static UINT ComputeSurfaceSize(UINT            cpWidth,
                                   UINT            cpHeight,
                                   UINT            cbPixel);

    static BOOL ComputeSurfaceSizeChecked(UINT            cpWidth,
                                          UINT            cpHeight,
                                          UINT            cbPixel,
                                          UINT            *pSize);


    static IHVFormatInfo *m_pFormatList;

}; // CPixel


#undef DPF_MODNAME
#define DPF_MODNAME "CPixel::ComputeSurfaceOffset"

inline void CPixel::ComputeSurfaceOffset(UINT                           cpWidth,
                                         UINT                           cpHeight,
                                         DXGI_FORMAT                    Format,
                                         BYTE                          *pBits,
                                         CONST RECT                    *pRect,
                                         D3D10DDI_MAPPED_SUBRESOURCE   *pLockedRectData)
{
    ComputeMipMapOffset(cpWidth, cpHeight, Format, 0, pBits, pRect, pLockedRectData);
} // ComputeSurfaceOffset


#undef DPF_MODNAME
#define DPF_MODNAME "CPixel::ComputeSurfaceSize"

inline UINT CPixel::ComputeSurfaceSize(UINT            cpWidth,
                                       UINT            cpHeight,
                                       DXGI_FORMAT     Format)
{
    UINT uSize = 0; // initialize to zero to keep prefix happy win7: 185026
    ComputeSurfaceSizeChecked(cpWidth, cpHeight, Format, &uSize);
    return uSize;
} // ComputeSurfaceSize

#undef DPF_MODNAME
#define DPF_MODNAME "CPixel::ComputeSurfaceSizeChecked"

inline BOOL CPixel::ComputeSurfaceSizeChecked(UINT            cpWidth,
                                              UINT            cpHeight,
                                              DXGI_FORMAT     Format,
                                              UINT           *uSize)
{
    UINT cbPixel = ComputePixelStride(Format);

    // Adjust pixel->block if necessary
    BOOL isDXT = IsDXT(cbPixel);
    if (isDXT)
    {
        AdjustForDXT(&cpWidth, &cpHeight, &cbPixel);
    }
#if (DXPIXELVER > 8)
    else
    if (Format == DXGI_FORMAT_R1_UNORM)
    {
        cpWidth = (cpWidth + 7) >> 3;
        cbPixel = 1;
    }
#endif

    return ComputeSurfaceSizeChecked(cpWidth,
                                     cpHeight,
                                     cbPixel,
                                     uSize);
} // ComputeSurfaceSize


#undef DPF_MODNAME
#define DPF_MODNAME "CPixel::AdjustForDXT"
inline void CPixel::AdjustForDXT(UINT *pcpWidth,
                                 UINT *pcpHeight,
                                 UINT *pcbPixel)
{
    DXGASSERT(pcbPixel);
    DXGASSERT(pcpWidth);
    DXGASSERT(pcpHeight);
    DXGASSERT(IsDXT(*pcbPixel));

    // Adjust width and height for DXT formats to be in blocks
    // instead of pixels. Blocks are 4x4 pixels.
    *pcpWidth  = (*pcpWidth  + 3) / 4;
    *pcpHeight = (*pcpHeight + 3) / 4;

    // Negate the pcbPixel to determine bytes per block
    *pcbPixel *= (UINT)-1;

    // We only know of two DXT formats right now...
    DXGASSERT(*pcbPixel == 8 || *pcbPixel == 16);

} // CPixel::AdjustForDXT

#undef DPF_MODNAME
#define DPF_MODNAME "CPixel::ComputeVolumeSize"

inline UINT CPixel::ComputeVolumeSize(UINT             cpWidth,
                                      UINT             cpHeight,
                                      UINT             cpDepth,
                                      DXGI_FORMAT      Format)
{
    UINT cbPixel = ComputePixelStride(Format);

    if (IsDXT(cbPixel))
    {
            AdjustForDXT(&cpWidth, &cpHeight, &cbPixel);
    }

    return cpDepth * ComputeSurfaceSize(cpWidth, cpHeight, cbPixel);
} // ComputeVolumeSize


#undef DPF_MODNAME
#define DPF_MODNAME "CPixel::IsDXT(cbPixel)"

// returns TRUE if cbPixel is "negative"
inline BOOL CPixel::IsDXT(UINT cbPixel)
{
    if (((INT)cbPixel) < 0)
        return TRUE;
    else
        return FALSE;
} // IsDXT

#undef DPF_MODNAME
#define DPF_MODNAME "CPixel::IsDXT(format)"

// returns TRUE if this is a linear format
// i.e. DXT or DXV
inline BOOL CPixel::IsDXT(DXGI_FORMAT Format)
{
    // CONSIDER: This is a duplication of Requires4x4 function
    switch (Format)
    {
    // normal DXTs
    case DXGI_FORMAT_BC1_TYPELESS:
    case DXGI_FORMAT_BC1_UNORM:
    case DXGI_FORMAT_BC1_UNORM_SRGB:
    case DXGI_FORMAT_BC2_TYPELESS:
    case DXGI_FORMAT_BC2_UNORM:
    case DXGI_FORMAT_BC2_UNORM_SRGB:
    case DXGI_FORMAT_BC3_TYPELESS:
    case DXGI_FORMAT_BC3_UNORM:
    case DXGI_FORMAT_BC3_UNORM_SRGB:
    case DXGI_FORMAT_BC4_TYPELESS:
    case DXGI_FORMAT_BC4_UNORM:
    case DXGI_FORMAT_BC4_SNORM:
    case DXGI_FORMAT_BC5_TYPELESS:
    case DXGI_FORMAT_BC5_UNORM:
    case DXGI_FORMAT_BC5_SNORM:
    case DXGI_FORMAT_BC6H_TYPELESS:
    case DXGI_FORMAT_BC6H_UF16:
    case DXGI_FORMAT_BC6H_SF16:
    case DXGI_FORMAT_BC7_TYPELESS:
    case DXGI_FORMAT_BC7_UNORM:
    case DXGI_FORMAT_BC7_UNORM_SRGB:
        return TRUE;
    }

    return FALSE;
} // IsDXT


#endif // __PIXEL_HPP_CPP__

#else

#ifndef __PIXEL_HPP_C__
#define __PIXEL_HPP_C__

UINT CPixel__ComputeSurfaceSize(UINT            cpWidth,
                                UINT            cpHeight,
                                DXGI_FORMAT     Format);

UINT CPixel__ComputeVolumeSize(UINT             cpWidth,
                               UINT             cpHeight,
                               UINT             cpDepth,
                               DXGI_FORMAT      Format);

UINT CPixel__ComputeMipMapSize(UINT             cpWidth,
                               UINT             cpHeight,
                               UINT             cLevels,
                               DXGI_FORMAT      Format);

UINT CPixel__ComputeMipVolumeSize(UINT          cpWidth,
                                  UINT          cpHeight,
                                  UINT          cpDepth,
                                  UINT          cLevels,
                                  DXGI_FORMAT   Format);

void CPixel__ComputeMipMapOffset(UINT                           cpWidth,
                                 UINT                           cpHeight,
                                 DXGI_FORMAT                    Format,
                                 UINT                           iLevel,
                                 BYTE                          *pBits,
                                 CONST RECT                    *pRect,
                                 D3D10DDI_MAPPED_SUBRESOURCE   *pLockedRectData);

void CPixel__ComputeMipVolumeOffset(UINT                            cpWidth,
                                    UINT                            cpHeight,
                                    UINT                            cpDepth,
                                    DXGI_FORMAT                     Format,
                                    UINT                            iLevel,
                                    BYTE                           *pBits,
                                    CONST D3DBOX                   *pBox,
                                    D3D10DDI_MAPPED_SUBRESOURCE    *pLockedBoxData);

void CPixel__ComputeSurfaceOffset(UINT                          cpWidth,
                                  UINT                          cpHeight,
                                  DXGI_FORMAT                   Format,
                                  BYTE                         *pBits,
                                  CONST RECT                   *pRect,
                                  D3D10DDI_MAPPED_SUBRESOURCE  *pLockedRectData);

UINT CPixel__ComputePixelStride(DXGI_FORMAT Format);

void CPixel__AdjustForDXT(UINT *pcpWidth,
                          UINT *pcpHeight,
                          UINT *pcbPixel);

BOOL CPixel__IsDXT(DXGI_FORMAT Format);

UINT CPixel__BytesPerPixel(DXGI_FORMAT Format);

HRESULT CPixel__Register(DXGI_FORMAT Format, DWORD BPP);

void CPixel__Cleanup();

UINT CPixel__ComputeSurfaceStride(UINT cpWidth, UINT cbPixel);


#endif // __PIXEL_HPP_C__

#endif


