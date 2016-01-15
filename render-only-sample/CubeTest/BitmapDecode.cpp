#include <windows.h>
#include <stdio.h>

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
