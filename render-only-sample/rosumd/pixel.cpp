/*==========================================================================;
 *
 *  Copyright (C) 1999-2002 Microsoft Corporation.  All Rights Reserved.
 *
 *  File:       pixel.cpp
 *  Content:    Utility class for working with pixel formats
 *
 *
 ***************************************************************************/

#include "precomp.h"
#include "pixel.hpp"

#ifndef DXPIXELVER
#define DXPIXELVER 100
#endif

IHVFormatInfo *CPixel::m_pFormatList = 0;

#undef DPF_MODNAME
#define DPF_MODNAME "CPixel::Cleanup"
void CPixel::Cleanup()
{
    while(m_pFormatList != 0)
    {
        IHVFormatInfo *t = m_pFormatList->m_pNext;
        delete m_pFormatList;
        m_pFormatList = t;
    }
}

#undef DPF_MODNAME
#define DPF_MODNAME "CPixel::ComputePixelStride"

UINT CPixel::ComputePixelStride(DXGI_FORMAT Format)
{
    UINT BPP = BytesPerPixel(Format);
    if (BPP == 0)
    {
        for(IHVFormatInfo *p = m_pFormatList; p != 0; p = p->m_pNext)
        {
            if (p->m_Format == Format)
            {
                return p->m_BPP >> 3;
            }
        }
    }
    return BPP;
}; // ComputePixelStride

#undef DPF_MODNAME
#define DPF_MODNAME "CPixel::ComputeSurfaceStride"

// Figure out a stride for a particular surface based on format and width
UINT CPixel::ComputeSurfaceStride(UINT cpWidth, UINT cbPixel)
{
    UINT uStride = 0;
    ComputeSurfaceStrideChecked(cpWidth, cbPixel, &uStride);
    return uStride;
}; // ComputeSurfaceStride

#undef DPF_MODNAME
#define DPF_MODNAME "CPixel::ComputeSurfaceStrideChecked"

// Figure out a stride for a particular surface based on format and width
inline BOOL CPixel::ComputeSurfaceStrideChecked(UINT cpWidth, UINT cbPixel, UINT *pStride)
{
    // Figure out basic (linear) stride;
    UINT64 u64Stride = (UINT64)cpWidth * (UINT64)cbPixel;

    // Round up to multiple of 4 (for NT; but makes sense to maximize
    // cache hits and reduce unaligned accesses)
    //
    // NOTE:  Also necessary for UYVY and YUY2 formats that this round
    // up by at least a multiple of 4.
    u64Stride = (u64Stride + 3) & ~3;    //safe since cbPixel cannot be 4G

    if (u64Stride <= MAXALLOCSIZE)
    {
        *pStride = (UINT)u64Stride;
        return TRUE;
    }
    return FALSE;
}; // ComputeSurfaceStride



#undef DPF_MODNAME
#define DPF_MODNAME "CPixel::ComputeSurfaceSize"

UINT CPixel::ComputeSurfaceSize(UINT            cpWidth,
                                UINT            cpHeight,
                                UINT            cbPixel)
{
    UINT uSize;
    ComputeSurfaceSizeChecked(cpWidth, cpHeight, cbPixel, &uSize);
    return uSize;
} // ComputeSurfaceSize

#undef DPF_MODNAME
#define DPF_MODNAME "CPixel::ComputeSurfaceSizeChecked"

BOOL CPixel::ComputeSurfaceSizeChecked(UINT            cpWidth,
                                       UINT            cpHeight,
                                       UINT            cbPixel,
                                       UINT           *pSize)

{
    UINT uStride;
    UINT64 uSize64;
    if (ComputeSurfaceStrideChecked(cpWidth, cbPixel, &uStride))
    {
        uSize64 = (UINT64)cpHeight * (UINT64)uStride;
        if (uSize64 <= MAXALLOCSIZE)
        {
            *pSize = (UINT)uSize64;
            return TRUE;
        }
    }

    *pSize = 0U;
    return FALSE;
} // ComputeSurfaceSize


#undef DPF_MODNAME
#define DPF_MODNAME "CPixel::ComputeMipMapSize"

UINT CPixel::ComputeMipMapSize(UINT             cpWidth,
                               UINT             cpHeight,
                               UINT             cLevels,
                               DXGI_FORMAT      Format)
{
    UINT uSize = 0; // initialize to zero to keep prefix happy win7: 185027
    ComputeMipMapSizeChecked(cpWidth, cpHeight, cLevels, Format, &uSize);
    return uSize;
} // ComputeMipMapSize

#undef DPF_MODNAME
#define DPF_MODNAME "CPixel::ComputeMipMapSizeChecked"

BOOL CPixel::ComputeMipMapSizeChecked(UINT             cpWidth,
                                      UINT             cpHeight,
                                      UINT             cLevels,
                                      DXGI_FORMAT      Format,
                                      UINT            *pSize)
{
    UINT cbPixel = ComputePixelStride(Format);

    // Adjust pixel->block if necessary
    BOOL isDXT = IsDXT(cbPixel);
    if (isDXT)
        cbPixel *= (UINT)-1;

    UINT64 cbSize = 0;
    if (cpWidth > MAXALLOCSIZE-3 || cpHeight > MAXALLOCSIZE-3)
        return FALSE;

    for (UINT i = 0; i < cLevels; i++)
    {
        UINT cbMipSize;
        // Figure out the size for
        // each level of the mip-map
        if (isDXT)
        {
            if (!ComputeSurfaceSizeChecked((cpWidth+3)/4, (cpHeight+3)/4, cbPixel, &cbMipSize))
                return FALSE;
        }
        else
        {
            if (!ComputeSurfaceSizeChecked(cpWidth, cpHeight, cbPixel, &cbMipSize))
                return FALSE;
        }
        cbSize += cbMipSize;
        // Shrink width and height by half; clamp to 1 pixel
        if (cpWidth > 1)
            cpWidth >>= 1;
        if (cpHeight > 1)
            cpHeight >>= 1;
    }
    if (cbSize <= MAXALLOCSIZE)
    {
        *pSize = (UINT)cbSize;
        return TRUE;
    }
    return FALSE;

} // ComputeMipMapSize

#undef DPF_MODNAME
#define DPF_MODNAME "CPixel::ComputeMipVolumeSize"

UINT CPixel::ComputeMipVolumeSize(UINT          cpWidth,
                                  UINT          cpHeight,
                                  UINT          cpDepth,
                                  UINT          cLevels,
                                  DXGI_FORMAT     Format)
{
    UINT uSize = 0; // To keep prefix happy win7: 185029
    ComputeMipVolumeSizeChecked(cpWidth, cpHeight, cpDepth, cLevels, Format, &uSize);
    return uSize;
} // ComputeMipVolumeSize

#undef DPF_MODNAME
#define DPF_MODNAME "CPixel::ComputeMipVolumeSizeChecked"

BOOL CPixel::ComputeMipVolumeSizeChecked(UINT          cpWidth,
                                         UINT          cpHeight,
                                         UINT          cpDepth,
                                         UINT          cLevels,
                                         DXGI_FORMAT   Format,
                                         UINT         *pSize)
{
    UINT cbPixel = ComputePixelStride(Format);

    // Adjust pixel->block if necessary
    BOOL isDXT       = IsDXT(cbPixel);

    if (isDXT)
        cbPixel *= (UINT)-1;

    UINT64 cbSize = 0;

    if (cpWidth > MAXALLOCSIZE-3 || cpHeight > MAXALLOCSIZE-3 || cpDepth > MAXALLOCSIZE-3)
        return FALSE;

    for (UINT i = 0; i < cLevels; i++)
    {
        // Figure out the size for
        // each level of the mip-volume
        UINT    Depth = cpDepth;
        UINT64  cbLevelSize;
        UINT    cbSliceSize;
        if (isDXT)
        {
            if (!ComputeSurfaceSizeChecked((cpWidth+3)/4, (cpHeight+3)/4, cbPixel, &cbSliceSize))
                return FALSE;
        }
        else
        {
            if (!ComputeSurfaceSizeChecked(cpWidth, cpHeight, cbPixel, &cbSliceSize))
                return FALSE;
        }
        cbLevelSize = (UINT64)Depth * (UINT64)cbSliceSize;
        if (cbLevelSize > MAXALLOCSIZE)
            return FALSE;
        cbSize += cbLevelSize;

        // Shrink width and height by half; clamp to 1 pixel
        if (cpWidth > 1)
            cpWidth >>= 1;
        if (cpHeight > 1)
            cpHeight >>= 1;
        if (cpDepth > 1)
            cpDepth >>= 1;
    }

    if (cbSize <= MAXALLOCSIZE)
    {
        *pSize = (UINT)cbSize;
        return TRUE;
    }
    return FALSE;

} // ComputeMipVolumeSize

// Given a surface desc, a level, and pointer to
// bits (pBits in the LockedRectData) and a sub-rect,
// this will fill in the pLockedRectData structure
void CPixel::ComputeMipMapOffset(UINT                           cpWidth,
                                 UINT                           cpHeight,
                                 DXGI_FORMAT                    Format,
                                 UINT                           iLevel,
                                 BYTE                          *pBits,
                                 CONST RECT                    *pRect,
                                 D3D10DDI_MAPPED_SUBRESOURCE   *pLockedRectData)
{
    DXGASSERT(pBits != NULL);
    DXGASSERT(pLockedRectData != NULL);
    DXGASSERT(iLevel < 32);
    DXGASSERT(0 != ComputePixelStride(Format));
    DXGASSERT(cpWidth > 0);
    DXGASSERT(cpHeight > 0);

    // CONSIDER: This is slow; and we can do a much better
    // job for the non-compressed/wacky cases.
    UINT       cbOffset = 0;
    UINT       cbPixel  = ComputePixelStride(Format);

    // Adjust pixel->block if necessary
    BOOL isDXT = IsDXT(cbPixel);
    if (isDXT)
        cbPixel *= (UINT)-1;
    BOOL OneBitPerPixelFormat = FALSE;
#if (DXPIXELVER > 8)
    OneBitPerPixelFormat = Format == DXGI_FORMAT_R1_UNORM;
    if (OneBitPerPixelFormat)
    {
        cbPixel = 1;
    }
#endif
    for (UINT i = 0; i < iLevel; i++)
    {
        if (isDXT)
            cbOffset += ComputeSurfaceSize((cpWidth+3)/4, (cpHeight+3)/4, cbPixel);
        else
        if (OneBitPerPixelFormat)
            cbOffset += ComputeSurfaceSize((cpWidth + 7) >> 3, cpHeight, cbPixel);
        else
            cbOffset += ComputeSurfaceSize(cpWidth, cpHeight, cbPixel);

        // Shrink width and height by half; clamp to 1 pixel
        if (cpWidth > 1)
            cpWidth >>= 1;
        if (cpHeight > 1)
            cpHeight >>= 1;
    }
    if (isDXT)
        cpWidth = (cpWidth+3)/4;
    else
    if (OneBitPerPixelFormat)
        cpWidth = (cpWidth+7)/8;


    // For DXTs, the pitch is the number of bytes
    // for a single row of blocks; which is the same
    // thing as the normal routine
    pLockedRectData->RowPitch = ComputeSurfaceStride(cpWidth,
                                                  cbPixel);
    DXGASSERT(pLockedRectData->RowPitch != 0);

    // Don't adjust for Rect for DXT formats
    if (pRect)
    {
        if (isDXT)
        {
            DXGASSERT((pRect->top  & 3) == 0);
            DXGASSERT((pRect->left & 3) == 0);
            cbOffset += (pRect->top  / 4) * pLockedRectData->RowPitch +
                        (pRect->left / 4) * cbPixel;
        }
        else
        if (OneBitPerPixelFormat)
        {
            // Assume that left is always 8 pixels aligned
            cbOffset += pRect->top  * pLockedRectData->RowPitch +
                        (pRect->left >> 3);
        }
        else
        {
            cbOffset += pRect->top  * pLockedRectData->RowPitch +
                        pRect->left * cbPixel;
        }
    }

    pLockedRectData->pData = pBits + cbOffset;

} // ComputeMipMapOffset

#undef DPF_MODNAME
#define DPF_MODNAME "CPixel::ComputeMipVolumeOffset"

// MipVolume version of ComputeMipMapOffset
void CPixel::ComputeMipVolumeOffset(UINT                            cpWidth,
                                    UINT                            cpHeight,
                                    UINT                            cpDepth,
                                    DXGI_FORMAT                     Format,
                                    UINT                            iLevel,
                                    BYTE                           *pBits,
                                    CONST D3DBOX                   *pBox,
                                    D3D10DDI_MAPPED_SUBRESOURCE    *pLockedBoxData)
{
    DXGASSERT(pBits != NULL);
    DXGASSERT(pLockedBoxData != NULL);
    DXGASSERT(iLevel < 32);
    DXGASSERT(0 != ComputePixelStride(Format));
    DXGASSERT(cpWidth > 0);
    DXGASSERT(cpHeight > 0);
    DXGASSERT(cpDepth > 0);

    UINT       cbOffset = 0;
    UINT       cbPixel  = ComputePixelStride(Format);

    // Adjust pixel->block if necessary
    BOOL isDXT       = IsDXT(cbPixel);

    if (isDXT)
        cbPixel *= (UINT)-1;

    for (UINT i = 0; i < iLevel; i++)
    {
        UINT    Depth = cpDepth;
        if (isDXT)
            cbOffset += Depth * ComputeSurfaceSize((cpWidth+3)/4, (cpHeight+3)/4, cbPixel);
        else
            cbOffset += Depth * ComputeSurfaceSize(cpWidth, cpHeight, cbPixel);

        // Shrink width and height by half; clamp to 1 pixel
        if (cpWidth > 1)
            cpWidth >>= 1;
        if (cpHeight > 1)
            cpHeight >>= 1;
        if (cpDepth > 1)
            cpDepth >>= 1;
    }

    if (isDXT)
    {
        cpWidth = (cpWidth+3)/4;
        cpHeight = (cpHeight+3)/4;
    }

    // For DXTs, the row pitch is the number of bytes
    // for a single row of blocks; which is the same
    // thing as the normal routine
    pLockedBoxData->RowPitch = ComputeSurfaceStride(cpWidth,
                                                    cbPixel);
    DXGASSERT(pLockedBoxData->RowPitch != 0);

    // For DXVs the slice pitch is the number of bytes
    // for a single plane of blocks; which is the same thing
    // as the normal routine
    pLockedBoxData->DepthPitch = ComputeSurfaceSize(cpWidth,
                                                    cpHeight,
                                                    cbPixel);
    DXGASSERT(pLockedBoxData->DepthPitch != 0);

    // Adjust for Box
    if (pBox)
    {
        UINT iStride = pLockedBoxData->RowPitch;
        UINT iSlice  = pLockedBoxData->DepthPitch;
        if (isDXT)
        {
                cbOffset += (pBox->Front) * iSlice;

            DXGASSERT((pBox->Top  & 3) == 0);
            DXGASSERT((pBox->Left & 3) == 0);
            cbOffset += (pBox->Top  / 4) * iStride +
                        (pBox->Left / 4) * cbPixel;
        }
        else
        {
            cbOffset += pBox->Front * iSlice  +
                        pBox->Top   * iStride +
                        pBox->Left  * cbPixel;
        }
    }

    pLockedBoxData->pData = pBits + cbOffset;

} // ComputeMipVolumeOffset


#undef DPF_MODNAME
#define DPF_MODNAME "CPixel::Register"

HRESULT CPixel::Register(DXGI_FORMAT Format, DWORD BPP)
{
    DXGASSERT(BPP != 0);

    // Do not register duplicates
    IHVFormatInfo *p;
    for(p = m_pFormatList; p != 0; p = p->m_pNext)
    {
        if (p->m_Format == Format)
        {
            return S_OK;
        }
    }

    // Not found, add to registry.
    // This allocation will be leaked, but since
    // we don't expect to have a large number of
    // IHV formats, the leak is not a big deal.
    // Also, the leak will be immediately recovered
    // upon process exit.
    IHVFormatInfo *p2 = new IHVFormatInfo;
    if (p2 == 0)
    {
        return E_OUTOFMEMORY;
    }
    p2->m_Format = Format;
    p2->m_BPP = BPP;
    p2->m_pNext = m_pFormatList;
    m_pFormatList = p2;

    return S_OK;
}


extern "C" UINT CPixel__ComputeSurfaceSize(UINT            cpWidth,
                                           UINT            cpHeight,
                                           DXGI_FORMAT     Format)
{
    return CPixel::ComputeSurfaceSize(cpWidth, cpHeight, Format);
}

extern "C" UINT CPixel__ComputeVolumeSize(UINT             cpWidth,
                                          UINT             cpHeight,
                                          UINT             cpDepth,
                                          DXGI_FORMAT      Format)
{
    return CPixel::ComputeVolumeSize(cpWidth, cpHeight, cpDepth, Format);
}

extern "C" UINT CPixel__ComputeMipMapSize(UINT             cpWidth,
                                          UINT             cpHeight,
                                          UINT             cLevels,
                                          DXGI_FORMAT      Format)
{
    return CPixel::ComputeMipMapSize(cpWidth, cpHeight, cLevels, Format);
}

extern "C" UINT CPixel__ComputeMipVolumeSize(UINT          cpWidth,
                                             UINT          cpHeight,
                                             UINT          cpDepth,
                                             UINT          cLevels,
                                             DXGI_FORMAT   Format)
{
    return CPixel::ComputeMipVolumeSize(cpWidth, cpHeight, cpDepth, cLevels, Format);
}

extern "C" void CPixel__ComputeMipMapOffset(UINT                            cpWidth,
                                            UINT                            cpHeight,
                                            DXGI_FORMAT                     Format,
                                            UINT                            iLevel,
                                            BYTE                           *pBits,
                                            CONST RECT                     *pRect,
                                            D3D10DDI_MAPPED_SUBRESOURCE    *pLockedRectData)
{
    CPixel::ComputeMipMapOffset(cpWidth, cpHeight, Format, iLevel, pBits, pRect, pLockedRectData);
}

extern "C" void CPixel__ComputeMipVolumeOffset(UINT                         cpWidth,
                                               UINT                         cpHeight,
                                               UINT                         cpDepth,
                                               DXGI_FORMAT                  Format,
                                               UINT                         iLevel,
                                               BYTE                        *pBits,
                                               CONST D3DBOX                *pBox,
                                               D3D10DDI_MAPPED_SUBRESOURCE *pLockedBoxData)
{
    CPixel::ComputeMipVolumeOffset(cpWidth, cpHeight, cpDepth, Format, iLevel, pBits, pBox, pLockedBoxData);
}

extern "C" void CPixel__ComputeSurfaceOffset(UINT                           cpWidth,
                                             UINT                           cpHeight,
                                             DXGI_FORMAT                    Format,
                                             BYTE                          *pBits,
                                             CONST RECT                    *pRect,
                                             D3D10DDI_MAPPED_SUBRESOURCE   *pLockedRectData)
{
    CPixel::ComputeSurfaceOffset(cpWidth, cpHeight, Format, pBits, pRect, pLockedRectData);
}

extern "C" UINT CPixel__ComputePixelStride(DXGI_FORMAT Format)
{
    return CPixel::ComputePixelStride(Format);
}

extern "C" void CPixel__AdjustForDXT(UINT *pcpWidth,
                                     UINT *pcpHeight,
                                     UINT *pcbPixel)
{
    CPixel::AdjustForDXT(pcpWidth, pcpHeight, pcbPixel);
}

extern "C" BOOL CPixel__IsDXT(DXGI_FORMAT Format)
{
    return CPixel::IsDXT(Format);
}

extern "C" UINT CPixel__BytesPerPixel(DXGI_FORMAT Format)
{
    return CPixel::BytesPerPixel(Format);
}

extern "C" HRESULT CPixel__Register(DXGI_FORMAT Format, DWORD BPP)
{
    return CPixel::Register(Format, BPP);
}

extern "C" void CPixel__Cleanup()
{
    CPixel::Cleanup();
}

extern "C" UINT CPixel__ComputeSurfaceStride(UINT cpWidth, UINT cbPixel)
{
    return CPixel::ComputeSurfaceStride(cpWidth, cbPixel);
}


// End of file : pixel.cpp

