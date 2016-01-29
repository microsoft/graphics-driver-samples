#include <SDKDDKVer.h>
#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <dxgi1_2.h>
#include <stdio.h>
#include <tchar.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <math.h>
#include <directxmath.h>

#include "BitmapDecode.h"

#if !(WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP))
typedef struct tagBITMAPFILEHEADER {
    WORD    bfType;
    DWORD   bfSize;
    WORD    bfReserved1;
    WORD    bfReserved2;
    DWORD   bfOffBits;
} BITMAPFILEHEADER, FAR *LPBITMAPFILEHEADER, *PBITMAPFILEHEADER;
#endif


//--------------------------------------------------------------------------------------
// BitmapDecode.cpp
//
// Loads windows DIB images into a buffer.
//--------------------------------------------------------------------------------------
#define MyReadData(pDst,Size) CopyMemory((PVOID)(pDst), (PVOID)(pFile), (Size)); (pFile)+=(Size);

namespace BMP
{
// Bitmap is monochrome and the color table contains two entries. Each
// bit in the bitmap array represents a pixel. If the bit is clear, the pixel is
// displayed with the color of the first entry in the color table. If the bit is
// set, the pixel has the color of the second entry in the table.

HRESULT loadMono(PBYTE pFile, BITMAPINFOHEADER& bmpHeader, /*_Out_writes_bytes_(bmpHeader.biHeight*bmpHeader.biWidth*4)*/ BYTE* imageBuffer, RGBQUAD* pPalette)
{
    HRESULT hr      = S_OK;
    DWORD   height  = bmpHeader.biHeight;
    DWORD   width   = bmpHeader.biWidth;
    DWORD   count   = 0;
    DWORD   itter   = 0;
    BYTE    alpha   = 0;
    BYTE    byte    = 0; 
    
    for (INT y = height - 1; y >= 0; y--)
    {
        itter = (y * width * 4) ;
        for (UINT x = 0; x < width; x ++ , count += 4)
        {
            MyReadData(&byte, sizeof(byte));            
            imageBuffer [ itter ++] = pPalette [byte ? 1 : 0].rgbRed;
            imageBuffer [ itter ++] = pPalette [byte ? 1 : 0].rgbGreen;
            imageBuffer [ itter ++] = pPalette [byte ? 1 : 0].rgbBlue;
            imageBuffer [ itter ++] = alpha;        
        }
        // skip remaining bytes
        for (count = ( width + 1 ) / 2; count % 4; count++)
        {
            MyReadData(&byte, sizeof(byte));
        }
    }
    return hr;
}


// the bitmap has a maximum of 16 colors, and the palette contains up to 16 entries. each pixel in the bitmap 
// is represented by a 4-bit index into the palette. for example, if the first byte in the bitmap is 1fh, the 
// byte represents two pixels. the first pixel contains the color in the second palette entry, and the second 
// pixel contains the color in the sixteenth palette entry.

HRESULT loadRGB4(PBYTE pFile, BITMAPINFOHEADER& bmpHeader, /*_Out_writes_bytes_(bmpHeader.biHeight*bmpHeader.biWidth*4)*/ BYTE* imageBuffer, RGBQUAD* pPalette)
{
    HRESULT hr      = S_OK;
    
    DWORD   height  = bmpHeader.biHeight;
    DWORD   width   = bmpHeader.biWidth;
    DWORD   count   = 0;
    DWORD   itter   = 0;
    BYTE    alpha   = 0;
    BYTE    byte    = 0; 

    for (INT y = height - 1; y >= 0; y--)
    {
        itter = (y * width * 4);
        
        for (UINT x = 0; x < width; x += 2, count += 4)
        {
            MyReadData(&byte, sizeof(byte));
            imageBuffer [ itter ++] = pPalette [byte >> 4].rgbRed;
            imageBuffer [ itter ++] = pPalette [byte >> 4].rgbGreen;
            imageBuffer [ itter ++] = pPalette [byte >> 4].rgbBlue;
            imageBuffer [ itter ++] = alpha;
                    
            imageBuffer [ itter ++] = pPalette [byte & 0x0f].rgbRed;
            imageBuffer [ itter ++] = pPalette [byte & 0x0f].rgbGreen;
            imageBuffer [ itter ++] = pPalette [byte & 0x0f].rgbBlue;
            imageBuffer [ itter ++] = alpha;
        }
        
        // skip remaining bytes
        for (count = (width + 1) / 2; count % 4; count++)
        {
            MyReadData(&byte, sizeof(byte));
        }
    }
    return hr;
}

HRESULT loadRGB8(PBYTE pFile, BITMAPINFOHEADER& bmpHeader, /*_Out_writes_bytes_(bmpHeader.biHeight*bmpHeader.biWidth*4)*/ BYTE* imageBuffer, RGBQUAD* pPalette)
{
    HRESULT hr      = S_OK;
    DWORD   dwRead  = 0;
    
    DWORD   height  = bmpHeader.biHeight;
    DWORD   width   = bmpHeader.biWidth;
    DWORD   count   = 0;
    DWORD   itter   = 0;
    BYTE    alpha   = 0;
    BYTE    byte    = 0; 
    
    for ( INT y = height - 1; y >= 0; y-- )
    {
        itter = (y * width * 4) ;
        
        for ( UINT x = 0; x < width; x ++ , count += 4)
        {
            MyReadData(&byte, sizeof(byte));
            imageBuffer [ itter ++] = pPalette [byte].rgbRed;
            imageBuffer [ itter ++] = pPalette [byte].rgbGreen;
            imageBuffer [ itter ++] = pPalette [byte].rgbBlue;
            imageBuffer [ itter ++] = alpha;
        }
        
        // skip remaining bytes
        for ( ; count % 4; count++ )    // skip remaining bytes
        {
            MyReadData(&byte, sizeof(byte));
        }
    }
    
    return hr;
}

HRESULT loadRGB24(PBYTE pFile, BITMAPINFOHEADER& bmpHeader, /*_Out_writes_bytes_(bmpHeader.biHeight*bmpHeader.biWidth*4)*/ BYTE* imageBuffer)
{
    HRESULT hr      = S_OK;
    DWORD   dwRead  = 0;
    DWORD   height  = bmpHeader.biHeight;
    DWORD   width   = bmpHeader.biWidth;
    DWORD   count   = 0;
    DWORD   itter   = 0;
    
    BYTE    red     = 0; 
    BYTE    green   = 0; 
    BYTE    blue    = 0;
    BYTE    alpha   = 0;
    BYTE    trash   = 0;
    
    for ( INT y = height - 1; y >= 0; y-- )
    {
        // What is this voodoo magic?
        // BMP files are stored bottom to top,
        // so this code indexes into the proper
        // row in the buffer to display it right.
        itter = (y * width * 4 ) ;
        
        for ( UINT x = count = 0; x < width; x++, count += 4 )
        {
            MyReadData(&blue, sizeof(blue));
            MyReadData(&green, sizeof(green));
            MyReadData(&red, sizeof(red));
            
            // The layout of a BMP 24 is BGR not RGB...go figure.
            imageBuffer [itter ++] = red;
            imageBuffer [itter ++] = green;
            imageBuffer [itter ++] = blue;
            imageBuffer [itter ++] = alpha;
        }
        
        for ( ; count % 4; count++ )    // skip remaining bytes
        {
            MyReadData(&trash, sizeof(trash));
        }
    }

    return hr;
}

HRESULT loadRGB32(PBYTE pFile, BITMAPINFOHEADER& bmpHeader, /*_Out_writes_bytes_(bmpHeader.biHeight*bmpHeader.biWidth*4)*/ BYTE* imageBuffer)
{
    HRESULT hr      = S_OK;
    DWORD   dwRead  = 0;
    DWORD   height  = bmpHeader.biHeight;
    DWORD   width   = bmpHeader.biWidth;
    DWORD   count   = 0;
    DWORD   itter   = 0;
    
    BYTE    red     = 0; 
    BYTE    green   = 0; 
    BYTE    blue    = 0;
    BYTE    alpha   = 0;
    
    for ( INT y = height - 1; y >= 0; y-- )
    {
        // What is this voodoo magic?
        // BMP files are stored bottom to top,
        // so this code indexes into the proper
        // row in the buffer to display it right.
        itter = (y * width * 3) ;
        
        for ( UINT x = count = 0; x < width; x++, count += 3 )
        {
            MyReadData(&blue, sizeof(blue));
            MyReadData(&green, sizeof(green));
            MyReadData(&red, sizeof(red));
            MyReadData(&alpha, sizeof(alpha));

            // The layout of a BMP is BGR not RGB...go figure.
            imageBuffer [itter ++] = red;
            imageBuffer [itter ++] = green;
            imageBuffer [itter ++] = blue;
            imageBuffer [itter ++] = alpha;
        }
    }
    
    return hr;
}
} // namespace BMP

HRESULT LoadBMP(BYTE* pFile, ULONG *pRetWidth, ULONG *pRetHeight, PBYTE *pRetData)
{
    HRESULT             hr          = S_OK;
    BITMAPFILEHEADER    hdr         = { 0 };
    BITMAPINFOHEADER    infoHdr     = { 0 };
    RGBQUAD *           bmiColors   = NULL;
 
    *pRetWidth = 0;
    *pRetHeight = 0;
    *pRetData = NULL;
 
    /////////////////////////////////////////////////////
    //
    // Load header and do error checking
    //
    /////////////////////////////////////////////////////

    MyReadData(&hdr.bfType, sizeof(hdr.bfType));
    // Check that this is a bitmap file.
    if(hdr.bfType != 0x4D42)
    {
        return E_FAIL;
    }

    MyReadData(&hdr.bfSize, sizeof(hdr.bfSize));
    MyReadData(&hdr.bfReserved1, sizeof(hdr.bfReserved1));
    MyReadData(&hdr.bfReserved2, sizeof(hdr.bfReserved2));
    MyReadData(&hdr.bfOffBits, sizeof(hdr.bfOffBits));
    MyReadData(&infoHdr.biSize, sizeof(infoHdr.biSize));
    MyReadData(&infoHdr.biWidth, sizeof(infoHdr.biWidth));
    MyReadData(&infoHdr.biHeight, sizeof(infoHdr.biHeight));
    MyReadData(&infoHdr.biPlanes, sizeof(infoHdr.biPlanes));
    MyReadData(&infoHdr.biBitCount, sizeof(infoHdr.biBitCount));
    MyReadData(&infoHdr.biCompression, sizeof(infoHdr.biCompression));
    MyReadData(&infoHdr.biSizeImage, sizeof(infoHdr.biSizeImage));
    MyReadData(&infoHdr.biXPelsPerMeter, sizeof(infoHdr.biXPelsPerMeter));
    MyReadData(&infoHdr.biYPelsPerMeter, sizeof(infoHdr.biYPelsPerMeter));
    MyReadData(&infoHdr.biClrUsed, sizeof(infoHdr.biClrUsed));
    MyReadData(&infoHdr.biClrImportant, sizeof(infoHdr.biClrImportant));

    if ((infoHdr.biSize == 40 && infoHdr.biPlanes == 1 &&
        (infoHdr.biBitCount == 1 || infoHdr.biBitCount == 2 ||
         infoHdr.biBitCount == 4 || infoHdr.biBitCount == 8 || 
         infoHdr.biBitCount == 24 || infoHdr.biBitCount == 32) &&
        (infoHdr.biCompression == BI_RGB) == FALSE))
    {
        return E_FAIL;
    }
    
    PBYTE pData = (PBYTE) malloc(infoHdr.biWidth * infoHdr.biHeight * 4);
    if(pData == NULL)
    {
        return E_FAIL;
    }
    ZeroMemory(pData,infoHdr.biWidth * infoHdr.biHeight * 4);
    
    // Load the palette if this is a pallete format.
    if (infoHdr.biBitCount <= 8)
    {
        // This determines the number of colors that  
        // a format can have: 1 bit = 1, 4 bit = 16,
        // 8 bit = 256.  This is used to determine
        // how big the palette should be.
        int numColors = 1 << infoHdr.biBitCount;
    
        bmiColors = (RGBQUAD*) malloc(numColors * sizeof(RGBQUAD));
        if (bmiColors == NULL)
        {
            hr = E_FAIL;
            goto CLEAN_UP;
        }
        ZeroMemory(bmiColors, numColors * sizeof(RGBQUAD));

        for (int x = 0; x < numColors; x++)
        {
            char r, g, b, res;

            MyReadData(&b, sizeof(b));
            MyReadData(&g, sizeof(g));
            MyReadData(&r, sizeof(r));
            MyReadData(&res, sizeof(res));

            bmiColors[x].rgbBlue = b;
            bmiColors[x].rgbGreen = g;
            bmiColors[x].rgbRed = r;
            bmiColors[x].rgbReserved = res;
        }
    }
    
    if (infoHdr.biCompression == BI_RGB)
    {
        if (infoHdr.biBitCount == 1)      // mono
        {
            hr = BMP::loadMono(pFile, infoHdr, pData, bmiColors);
        }
        else if (infoHdr.biBitCount == 4)      // 16-colors uncompressed
        {
            hr = BMP::loadRGB4(pFile, infoHdr, pData, bmiColors);
        }
        else if (infoHdr.biBitCount == 8)      // 256-colors uncompressed
        {
            hr = BMP::loadRGB8(pFile, infoHdr, pData, bmiColors);
        }
        else if (infoHdr.biBitCount == 24) // True-Color bitmap
        {
            hr = BMP::loadRGB24(pFile, infoHdr, pData);
        }
        else if (infoHdr.biBitCount == 32) // true-color bitmap with alpha-channel
        {
            hr = BMP::loadRGB32(pFile, infoHdr, pData);
        }
    }
    
CLEAN_UP:
    if(bmiColors)
    {
        free(bmiColors);
    }

    if (FAILED(hr))
    {
        if (pData)
        {
            free(pData);
        }
    }
    else
    {
       *pRetWidth = infoHdr.biWidth;
       *pRetHeight = infoHdr.biHeight;
       *pRetData = pData;
    }

    return hr;
}

/*
0  -  No image data included.
1  -  Uncompressed, color-mapped images.
2  -  Uncompressed, RGB images.
3  -  Uncompressed, black and white images.
9  -  Runlength encoded color-mapped images.
10  -  Runlength encoded RGB images.
11  -  Compressed, black and white images.
32  -  Compressed color-mapped data, using Huffman, Delta, and
runlength encoding.
33  -  Compressed color-mapped data, using Huffman, Delta, and
runlength encoding.  4-pass quadtree-type process.
*/

enum TARGA_IMAGE_TYPE {
    NO_IMAGE                        = 0,
    UNCOMPRESSED_PALLETIZED         = 1,
    UNCOMPRESSED_RGB                = 2,
    UNCOMPRESSED_MONOCHROME         = 3,
    RUNLENGTH_ENCODED_PALLETIZED    = 9,
    RUNLENGTH_ENCODED_RGB           = 10,
    COMPRESSED_MONOCHROME           = 11,
    COMPRESSED_PALLETIZED           = 32,
    COMPRESSED_PALLETIZED_4_PASS    = 33,
};

// TGA specific constants and structs

typedef struct _TargaHeader {
    BYTE        id_length;
    BYTE        colormap_type; 
    BYTE        image_type;
    USHORT      colormap_index;
    USHORT      colormap_length;
    BYTE        colormap_size;
    USHORT      x_origin;
    USHORT      y_origin; 
    USHORT      width; 
    USHORT      height;
    BYTE        pixel_size;
    BYTE        attributes;
} TargaHeader;

typedef struct _Targa_Palette {
    BYTE        b;
    BYTE        g;
    BYTE        r;
    BYTE        a;
} Targa_Palette;

namespace TGA
{
HRESULT loadRGB8(PBYTE pFile, TargaHeader& Header, /*_Out_writes_bytes_(Header.height*Header.width*4)*/ BYTE* imageBuffer, Targa_Palette* pPalette)
{
    HRESULT     hr      = S_OK;

    UINT        height  = Header.height;
    UINT        width   = Header.width;
    UINT        count   = 0;
    UINT        itter   = 0;
    DWORD       dwRead  = 0;
    
    BYTE        byte    = 0; 
    BYTE        trash   = 0;
    
    for ( INT y = height - 1; y >= 0; y-- )
    {
        itter = (y * width * 4) ;
        
        for ( UINT x = 0; x < width; x ++ , count += 3)
        {
            MyReadData(&byte, sizeof(byte));
            imageBuffer [ itter ++] = pPalette [byte].r;
            imageBuffer [ itter ++] = pPalette [byte].g;
            imageBuffer [ itter ++] = pPalette [byte].b;
            imageBuffer [ itter ++] = pPalette [byte].a;
        }
        
        // skip remaining bytes
        for ( ; count % 4; count++ )    // skip remaining bytes
        {
            MyReadData(&trash, sizeof(trash));
        }
    }
        
    return hr;
}

HRESULT loadRGB24(PBYTE pFile, TargaHeader& Header, /*_Out_writes_bytes_(Header.height*Header.width*4)*/ BYTE* imageBuffer)
{
    HRESULT     hr      = S_OK;
    
    UINT        height  = Header.height;
    UINT        width   = Header.width;
    UINT        itter   = 0;
    DWORD       dwRead  = 0;
    
    BYTE        red     = 0; 
    BYTE        green   = 0; 
    BYTE        blue    = 0;
    
    for ( INT y = height - 1; y >= 0; y-- )
    {
        // What is this voodoo magic?
        // BMP files are stored bottom to top,
        // so this code indexes into the proper
        // row in the buffer to display it right.
        itter = (y * width * 4) ;
        
        for ( UINT x = 0; x < width; x++ )
        {
            MyReadData(&blue, sizeof(blue));            
            MyReadData(&green, sizeof(green));
            MyReadData(&red, sizeof(red));            
            // The layout of a TGA 24 is BGR not RGB...go figure.
            imageBuffer [itter ++] = red;
            imageBuffer [itter ++] = green;
            imageBuffer [itter ++] = blue;
            imageBuffer [itter ++] = 0;
        }
    }

    return hr;
}

HRESULT loadRGB32(PBYTE pFile, TargaHeader& Header, /*_Out_writes_bytes_(Header.height*Header.width*4)*/ BYTE* imageBuffer)
{
    HRESULT     hr      = S_OK;
    
    UINT        height  = Header.height;
    UINT        width   = Header.width;
    UINT        itter   = 0;
    DWORD       dwRead  = 0;
    
    BYTE        red     = 0; 
    BYTE        green   = 0; 
    BYTE        blue    = 0;
    BYTE        alpha   = 0;
    
    for ( INT y = height - 1; y >= 0; y-- )
    {
        // What is this voodoo magic?
        // TGA files are stored bottom to top,
        // so this code indexes into the proper
        // row in the buffer to display it right.
        itter = (y * width * 4) ;
        
        for ( UINT x = 0; x < width; x++ )
        {
            MyReadData(&blue, sizeof(blue));
            MyReadData(&green, sizeof(green));
            MyReadData(&red, sizeof(red));
            MyReadData(&alpha, sizeof(alpha));
            // The layout of a TGA 32 is BGR not RGB...go figure.
            imageBuffer [itter ++] = red;
            imageBuffer [itter ++] = green;
            imageBuffer [itter ++] = blue;
            imageBuffer [itter ++] = alpha;
        }
    }
    
    return hr;
}
} // namespace TGA

HRESULT LoadTGA(PBYTE pFile, ULONG *pRetWidth, ULONG *pRetHeight, PBYTE *pRetData)
{
    HRESULT         hr          = S_OK;
    Targa_Palette * pPalette    = NULL;
    TargaHeader     hdr         = { 0 };
    
    /////////////////////////////////////////////////////
    //
    // Load header and do error checking
    //
    /////////////////////////////////////////////////////
    
    MyReadData(&hdr.id_length, sizeof(hdr.id_length));
    MyReadData(&hdr.colormap_type, sizeof(hdr.colormap_type));
    MyReadData(&hdr.image_type, sizeof(hdr.image_type));
    MyReadData(&hdr.colormap_index, sizeof(hdr.colormap_index));
    MyReadData(&hdr.colormap_length, sizeof(hdr.colormap_length));
    MyReadData(&hdr.colormap_size, sizeof(hdr.colormap_size));
    MyReadData(&hdr.x_origin, sizeof(hdr.x_origin));
    MyReadData(&hdr.y_origin, sizeof(hdr.y_origin));
    MyReadData(&hdr.width, sizeof(hdr.width));
    MyReadData(&hdr.height, sizeof(hdr.height));
    MyReadData(&hdr.pixel_size, sizeof(hdr.pixel_size));
    MyReadData(&hdr.attributes, sizeof(hdr.attributes));

    if (hdr.image_type != NO_IMAGE && 
        hdr.image_type != UNCOMPRESSED_PALLETIZED && 
        hdr.image_type != UNCOMPRESSED_RGB &&
        hdr.image_type != UNCOMPRESSED_MONOCHROME && 
        hdr.image_type != RUNLENGTH_ENCODED_PALLETIZED && 
        hdr.image_type != RUNLENGTH_ENCODED_RGB &&
        hdr.image_type != COMPRESSED_MONOCHROME )               
    {
        return E_FAIL;
    }

    if ( hdr.image_type >= 9 && hdr.image_type <= 11 )      // RLE image  not supported yet
    {
        return E_FAIL;
    }
   
    if ( hdr.pixel_size != 32 && hdr.pixel_size != 24 && hdr.pixel_size != 8 )
    {
        return E_FAIL;
    }

    // Allocate memory for the bitmap
    PBYTE pData = (PBYTE) malloc(hdr.width * hdr.height * 4);
    if(!pData)
    {
        return E_FAIL;
    }
    memset(pData, 0, hdr.width * hdr.height * 4);
    
    pPalette = (Targa_Palette *) malloc(hdr.colormap_size * sizeof(Targa_Palette));
    if(!pPalette)
    {
        hr = E_FAIL;
        goto CLEAN_UP;;
    }
    memset(pPalette, 0, hdr.colormap_size);  
    
    // Load the pallet for palletized formats.
    if ( hdr.image_type == UNCOMPRESSED_PALLETIZED || hdr.image_type == RUNLENGTH_ENCODED_PALLETIZED )       
    {
        if ( hdr.colormap_size == 15 || hdr.colormap_size == 16 )
        {        
            UINT     a, b;
            
            for ( int i = 0; i < hdr.colormap_length; i++ )
            {
                MyReadData(&a , sizeof(a));
                MyReadData(&b , sizeof(b));
                pPalette[i].r = BYTE(a & 0x1F);
                pPalette[i].g = BYTE(((b & 0x03) << 3) | ((a & 0xE0) >> 5));
                pPalette[i].b = BYTE((b & 0x7C) >> 2);
                pPalette[i].a = 0;
            }
        }
        else if ( hdr.colormap_length == 24 )
        {
            for ( int i = 0; i < hdr.colormap_length; i++ )
            {
                MyReadData(&pPalette[i].b , sizeof(pPalette[i].b));
                MyReadData(&pPalette[i].g , sizeof(pPalette[i].g));
                MyReadData(&pPalette[i].r , sizeof(pPalette[i].r));
                pPalette[i].a = 0;
            }
        }             
        else if ( hdr.colormap_size == 32 )
        {
            for ( int i = 0; i < hdr.colormap_length; i++ )
            {
                MyReadData(&pPalette[i].b , sizeof(pPalette[i].b));
                MyReadData(&pPalette[i].g , sizeof(pPalette[i].g));
                MyReadData(&pPalette[i].r , sizeof(pPalette[i].r));
                MyReadData(&pPalette[i].a , sizeof(pPalette[i].a));
            }
        }
    }
    else if ( hdr.colormap_size == 0 && hdr.pixel_size == 8 )   // grey-scale image
    {
        // if not platted, but 8 bpp
        // create greyscale identity palette
        for ( BYTE i = 0; i < 256; i++ )
        {
            pPalette [i].b = i;
            pPalette [i].g = i;
            pPalette [i].r = i;
            pPalette [i].a = i;
        }
    }
    
    if ( hdr.image_type == UNCOMPRESSED_PALLETIZED ||
         hdr.image_type == UNCOMPRESSED_RGB ||
         hdr.image_type == UNCOMPRESSED_MONOCHROME )
    {
        if ( hdr.pixel_size == 8 )
        {
            hr = TGA::loadRGB8(pFile, hdr, (BYTE *) pData, pPalette);
        }
        else if ( hdr.pixel_size == 24 )
        {
            hr = TGA::loadRGB24(pFile, hdr, (BYTE *) pData);
        }
        else if ( hdr.pixel_size == 32 )
        {
            hr = TGA::loadRGB32(pFile, hdr, (BYTE *) pData);
        }
    }
    
CLEAN_UP:
    
    if(pPalette)
    {
        free(pPalette);
    }

    if (FAILED(hr))
    {
        if (pData)
        {
            free(pData);
        }
    }
    else
    {
        *pRetWidth = hdr.width;
        *pRetHeight = hdr.height;
        *pRetData = pData;
    }

    return hr;
}

HRESULT SaveBMP(const char* pFileName, ID3D11Device *pDevice, ID3D11Texture2D *pTexture)
{
    FILE * fp = NULL;

    errno_t result = fopen_s(&fp, pFileName, "wb");
    if (result != 0)
    {
        return E_FAIL;
    }

#pragma pack(push, 2)

    struct BMPHeader
    {
        UINT16  m_id;
        UINT32  m_fileSize;
        UINT32  m_unused;
        UINT32  m_pixelArrayOffset;
    };

    struct DIBHeader
    {
        UINT32  m_dibHeaderSize;
        UINT32  m_widthPixels;
        UINT32  m_heightPixels;
        UINT16  m_numPlanes;
        UINT16  m_bitsPerPixel;
        UINT32  m_compressionMethod;
        UINT32  m_pixelDataSize;
        UINT32  m_pixelsPerMeterHorizontal;
        UINT32  m_pixelsPerMeterVertical;
        UINT32  m_colorsInPalette;
        UINT32  m_importantColors;
    };

#pragma pack(pop)

    D3D11_TEXTURE2D_DESC desc;
    pTexture->GetDesc(&desc);

    boolean bSwapRGB = false;
    switch (desc.Format)
    {
    case DXGI_FORMAT_B8G8R8A8_UNORM:
        bSwapRGB = true;
        break;
    case DXGI_FORMAT_R8G8B8A8_UNORM:
        break;
    default:
        // unsupported format
        return E_FAIL;
    }

    UINT32 pixelWidth = desc.Width;
    UINT32 pixelHeight = desc.Height;
    UINT32 byteWidthNoPadding = pixelWidth * 3;
    UINT32 byteWidth = (byteWidthNoPadding + 3) & ~3;
    UINT32 bytePadding = byteWidth - byteWidthNoPadding;
    UINT32 pixelDataSize = byteWidth * pixelHeight;

    BMPHeader bmpHeader;

    bmpHeader.m_id = 0x4D42;
    bmpHeader.m_fileSize = sizeof(BMPHeader) + sizeof(DIBHeader) + pixelDataSize;
    bmpHeader.m_pixelArrayOffset = sizeof(BMPHeader) + sizeof(DIBHeader);
    bmpHeader.m_unused = 0;

    DIBHeader dibHeader;

    dibHeader.m_bitsPerPixel = 24;
    dibHeader.m_colorsInPalette = 0;
    dibHeader.m_compressionMethod = 0;
    dibHeader.m_dibHeaderSize = sizeof(DIBHeader);
    dibHeader.m_heightPixels = pixelHeight;
    dibHeader.m_importantColors = 0;
    dibHeader.m_numPlanes = 1;
    dibHeader.m_pixelDataSize = pixelDataSize;
    dibHeader.m_pixelsPerMeterHorizontal = 2835;
    dibHeader.m_pixelsPerMeterVertical = 2835;
    dibHeader.m_widthPixels = pixelWidth;

    fwrite(&bmpHeader, sizeof(bmpHeader), 1, fp);
    fwrite(&dibHeader, sizeof(dibHeader), 1, fp);

    D3D11_MAPPED_SUBRESOURCE mappedSubresource = { 0 };
    ID3D11DeviceContext *pImmediateContext;
    pDevice->GetImmediateContext(&pImmediateContext);
            
    if (SUCCEEDED(pImmediateContext->Map(pTexture, 0, D3D11_MAP_READ, 0, &mappedSubresource)))
    {
        UINT8 * pData = reinterpret_cast<UINT8*>(mappedSubresource.pData);
        assert(pData);

        DWORD padding = 0;
        for (UINT32 row = 0; row < pixelHeight; row++)
        {
            UINT8 * pRow = pData + ((pixelHeight - row - 1) * (pixelWidth * 4));

            for (UINT32 col = 0; col < pixelWidth; col++)
            {
                if (bSwapRGB)
                {
                    fwrite(pRow + 0, 1, 1, fp);
                    fwrite(pRow + 1, 1, 1, fp);
                    fwrite(pRow + 2, 1, 1, fp);
                }
                else
                {
                    fwrite(pRow + 2, 1, 1, fp);
                    fwrite(pRow + 1, 1, 1, fp);
                    fwrite(pRow + 0, 1, 1, fp);
                }

                pRow += 4;
            }
            if (bytePadding) fwrite(&padding, 1, bytePadding, fp);
        }

        pImmediateContext->Unmap(pTexture, 0);
    }
        
    fclose(fp);

    return S_OK;
}


