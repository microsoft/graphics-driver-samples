#include <windows.h>
#include <d3d11_1.h>
#include <dxgi1_3.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>

#include <stdio.h>

#include <exception>
#include <memory>

#include "BitmapDecode.h"

using namespace DirectX;

#define USE_VC4 VC4

//#define NO_TRANSFORM 1
//#define USE_QUAD 1
#define USE_TEX 1
#define USE_BITMAP_TEX 1

//#define USE_BGRA_RT 1

//#define USE_SCISSOR 1

#if USE_TEX
struct SimpleVertex
{
    XMFLOAT4 Pos;
    XMFLOAT2 Tex;
};
#else
struct SimpleVertex
{
    XMFLOAT4 Pos;
    XMFLOAT4 Color;
};
#endif // USE_TEX

struct VSConstantBuffer
{
    XMMATRIX World;
    XMMATRIX View;
    XMMATRIX Projection;
};

struct PSConstantBuffer
{
    XMFLOAT4 Ambient;
};

int kWidth = 800;
int kHeight = 600;
UINT frames = 8;

static const int kTexWidth = 64;
static const int kTexHeight = 64;

template<class T> class D3DPointer
{
public:

    D3DPointer() : m_p(NULL) {}
    ~D3DPointer() { if (m_p) m_p->Release(); }

    T * operator->() { return m_p; }

    operator bool() { return m_p != NULL; }
    operator T *() { return m_p; }

    D3DPointer<T> & operator=(T * p) { if (m_p) m_p->Release();  m_p = p; return *this; }

private:

    T * m_p;

};

class D3DDevice
{
public:

    D3DDevice(WCHAR * inDescription)
    {
        if (!FindAdapter(inDescription)) throw std::exception("Failed to find adapter");
        if (!CreateDevice()) throw std::exception("Failed to create device");
    }

    ~D3DDevice()
    {
        // do nothing
    }

    ID3D11Device * GetDevice() { return m_pDevice; }
    ID3D11DeviceContext * GetContext() { return m_pContext; }

private:

    D3DPointer<ID3D11Device>        m_pDevice;
    D3DPointer<ID3D11DeviceContext> m_pContext;
    D3DPointer<IDXGIAdapter2>       m_pAdapter;

    bool CreateDevice()
    {
        bool deviceCreated = false;
        D3D_FEATURE_LEVEL level;
        ID3D11DeviceContext * context = NULL;
#if VC4
        D3D_FEATURE_LEVEL  FeatureLevelsRequested[] = { D3D_FEATURE_LEVEL_9_1 };
#else
        D3D_FEATURE_LEVEL  FeatureLevelsRequested[] = { D3D_FEATURE_LEVEL_11_0 };
#endif 
        ID3D11Device * pDevice;
        ID3D11DeviceContext * pContext;

        HRESULT hr = D3D11CreateDevice(m_pAdapter, D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_UNKNOWN, NULL, 0, FeatureLevelsRequested, 1, D3D11_SDK_VERSION, &pDevice, &level, &pContext);

        deviceCreated = (hr == S_OK);

        if (deviceCreated)
        {
            m_pDevice = pDevice;
            m_pContext = pContext;
        }

        return deviceCreated;
    }

    bool FindAdapter(WCHAR * inDescription)
    {
        assert(inDescription != NULL);

        bool found = false;
        IDXGIFactory2 * factory = NULL;
        HRESULT hr = CreateDXGIFactory2(0, __uuidof(IDXGIFactory2), ((void **)&factory));
        if (FAILED(hr)) throw std::exception("Unable to create DXGIFactor2");

        UINT adapterIndex = 0;
        bool done = false;

        while (!done && !found) {

            IDXGIAdapter1 * adapter = NULL;

            hr = factory->EnumAdapters1(adapterIndex, &adapter);

            if (hr == S_OK)
            {
                IDXGIAdapter2 * adapter2 = NULL;

                hr = adapter->QueryInterface(__uuidof(IDXGIAdapter2), (void **)&adapter2);

                if (hr == S_OK)
                {
                    DXGI_ADAPTER_DESC2 desc;
                    adapter2->GetDesc2(&desc);

                    found = (wcscmp(inDescription, desc.Description) == 0);

                    if (found)
                    {
                        m_pAdapter = adapter2;
                    }
                    else
                    {
                        adapter2->Release();
                    }
                }

                adapter->Release();

                adapterIndex++;
            }
            else
            {
                done = true;
            }
        }

        factory->Release();

        return found;
    }

};

class D3DStagingTexture
{
public:

    D3DStagingTexture(std::shared_ptr<D3DDevice> & inDevice, int inWidth, int inHeight)
    {
        D3D11_TEXTURE2D_DESC desc;

        desc.ArraySize = 1;
        desc.BindFlags = 0;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
#if USE_BGRA_RT
        desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
#else
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
#endif // USE_BGRA_RT
        desc.Height = inHeight;
        desc.MipLevels = 1;
        desc.MiscFlags = 0;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Usage = D3D11_USAGE_STAGING;
        desc.Width = inWidth;

        ID3D11Texture2D * pTexture;

        HRESULT hr = inDevice->GetDevice()->CreateTexture2D(&desc, NULL, &pTexture);
        if (FAILED(hr)) throw std::exception("Failed to create texture");

        m_pTexture = pTexture;
        m_pDevice = inDevice;
    }

    ~D3DStagingTexture()
    {
        // do nothing
    }

    ID3D11Texture2D * GetTexture() { return m_pTexture; }

    void WriteToBmp(const char * inFilePath)
    {
        FILE * fp = NULL;

        errno_t result = fopen_s(&fp, inFilePath, "wb");

        if (result != 0) throw std::exception("Failed to open file");

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

        UINT32  colorMasks[3];

#pragma pack(pop)

        D3D11_TEXTURE2D_DESC desc;
        m_pTexture->GetDesc(&desc);

        UINT32 pixelWidth = desc.Width;
        UINT32 pixelHeight = desc.Height;
        UINT32 byteWidthNoPadding = pixelWidth * 4; // 32bpp
        UINT32 byteWidth = (byteWidthNoPadding + 3) & ~3;
        UINT32 bytePadding = byteWidth - byteWidthNoPadding;
        UINT32 pixelDataSize = byteWidth * pixelHeight;

        BMPHeader bmpHeader;

        bmpHeader.m_id = 0x4D42;
        bmpHeader.m_fileSize = sizeof(BMPHeader) + sizeof(DIBHeader) + sizeof(colorMasks) + pixelDataSize;
        bmpHeader.m_pixelArrayOffset = sizeof(BMPHeader) + sizeof(DIBHeader) + sizeof(colorMasks);
        bmpHeader.m_unused = 0;

        DIBHeader dibHeader;

        dibHeader.m_bitsPerPixel = 32;
        dibHeader.m_colorsInPalette = 0;
        dibHeader.m_compressionMethod = BI_BITFIELDS;
        dibHeader.m_dibHeaderSize = sizeof(DIBHeader);
        dibHeader.m_heightPixels = pixelHeight;
        dibHeader.m_importantColors = 0;
        dibHeader.m_numPlanes = 1;
        dibHeader.m_pixelDataSize = pixelDataSize;
        dibHeader.m_pixelsPerMeterHorizontal = 2835;
        dibHeader.m_pixelsPerMeterVertical = 2835;
        dibHeader.m_widthPixels = pixelWidth;

#if USE_BGRA_RT
        colorMasks[0] = 0xFF0000; // R
        colorMasks[1] = 0xFF00;   // G
        colorMasks[2] = 0xFF;     // B
#else
        colorMasks[0] = 0xFF;     // R
        colorMasks[1] = 0xFF00;   // G
        colorMasks[2] = 0xFF0000; // B
#endif // USE_BGRA_RT

        fwrite(&bmpHeader, sizeof(bmpHeader), 1, fp);
        fwrite(&dibHeader, sizeof(dibHeader), 1, fp);
        fwrite(&colorMasks[0], sizeof(colorMasks), 1, fp);

        D3D11_MAPPED_SUBRESOURCE mappedSubresource;
        HRESULT hr = m_pDevice->GetContext()->Map(m_pTexture, 0, D3D11_MAP_READ, 0, &mappedSubresource);
        if (FAILED(hr)) throw std::exception("Failed to map texture");

        UINT8 * pData = reinterpret_cast<UINT8*>(mappedSubresource.pData);

        DWORD padding = 0;
        for (UINT32 row = 0; row < pixelHeight; row++)
        {
            UINT8 * pRow = pData + ((pixelHeight - row - 1) * (pixelWidth * 4));

            for (UINT32 col = 0; col < pixelWidth; col++)
            {
                fwrite(pRow, 4, 1, fp);
                pRow += 4;
            }
            if (bytePadding) fwrite(&padding, 1, bytePadding, fp);
        }

        m_pDevice->GetContext()->Unmap(m_pTexture, 0);

        fclose(fp);
    }

private:

    D3DPointer<ID3D11Texture2D>     m_pTexture;
    std::shared_ptr<D3DDevice>      m_pDevice;
};

class D3DDefaultTexture
{
public:

    D3DDefaultTexture(std::shared_ptr<D3DDevice> & inDevice, int inWidth, int inHeight)
    {
        D3D11_TEXTURE2D_DESC desc;

        desc.CPUAccessFlags = 0;
        desc.Width = inWidth;
        desc.Height = inHeight;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        desc.MiscFlags = 0;

        DWORD * pTexels = new DWORD[inWidth*inHeight];
        if (!pTexels)
        {
            throw std::exception("Failed to allocate texels");
        }

        DWORD   checkerColors[] =
        {
            0x00400000,
            0x00000040
        };

        for (int j = 0; j < inHeight; j++)
        {
            for (int i = 0; i < inWidth; i++)
            {
                pTexels[j*inWidth + i] =  checkerColors[((i >> 3 & 0x1) ^ (j >> 3 & 0x1))];
            }
        }

        D3D11_SUBRESOURCE_DATA initData;

        ZeroMemory(&initData, sizeof(initData));
        initData.pSysMem = pTexels;
        initData.SysMemPitch = inWidth*4;
        initData.SysMemSlicePitch = inWidth*4*inHeight;

        ID3D11Texture2D * pTexture;

        HRESULT hr = inDevice->GetDevice()->CreateTexture2D(&desc, &initData, &pTexture);

        delete [] pTexels;

        if (FAILED(hr))
        {
            throw std::exception("Failed to create texture");
        }

        ID3D11ShaderResourceView *  pShaderResourceView;

        hr = inDevice->GetDevice()->CreateShaderResourceView(pTexture, NULL, &pShaderResourceView);
        if (FAILED(hr))
        {
            throw std::exception("Failed to create shader resource view");
        }

        m_pTexture = pTexture;
        m_pDevice = inDevice;
        m_pShaderResourceView = pShaderResourceView;
    }

    ~D3DDefaultTexture()
    {
        // do nothing
    }

    ID3D11Texture2D * GetTexture() { return m_pTexture; }
    ID3D11ShaderResourceView * GetShaderResourceView() { return m_pShaderResourceView;  }

private:

    D3DPointer<ID3D11Texture2D>             m_pTexture;
    D3DPointer<ID3D11ShaderResourceView>    m_pShaderResourceView;
    std::shared_ptr<D3DDevice>              m_pDevice;
};

const char *DXGIFormatToString(DXGI_FORMAT format)
{
    switch (format)
    {
    case DXGI_FORMAT_R8G8B8A8_UNORM:
        return "R8G8B8A8_UNORM";

    case DXGI_FORMAT_R8G8_UNORM:
        return "DXGI_FORMAT_R8G8_UNORM";

    case DXGI_FORMAT_R8_UNORM:
        return "DXGI_FORMAT_R8_UNORM";

    case DXGI_FORMAT_A8_UNORM:
        return "DXGI_FORMAT_A8_UNORM";

    default:
        return "UNKNOWN";
    }
}

class D3DBitmapTexture
{
public:

    UINT GetFormatBytesPerPixel(DXGI_FORMAT inFormat)
    {
        switch (inFormat)
        {
        case DXGI_FORMAT_R8G8B8A8_UNORM:
        {
            return 4;
        }
        break;

        case DXGI_FORMAT_R8G8_UNORM:
        {
            return 2;
        }
        break;

        case DXGI_FORMAT_R8_UNORM:
        {
            return 1;
        }
        break;

        case DXGI_FORMAT_A8_UNORM:
        {
            return 1;
        }
        break;

        default:
        {
            throw std::exception("Not implemented texture color format passed");
        }
        
        }        
    }

    std::unique_ptr<BYTE[]> ConvertTextureFromRGBA(PBYTE _In_ pTexels, DXGI_FORMAT _In_ outFormat, ULONG _In_ texWidth, ULONG _In_ texHeight, UINT _Out_ &outTextureBytesPerPixel)
    {
        outTextureBytesPerPixel = GetFormatBytesPerPixel(outFormat);        
        std::unique_ptr<BYTE[]> colorConvertedBuffer(new BYTE[texWidth * texHeight * outTextureBytesPerPixel]);
        BYTE *colorConvertedBufferRaw = (BYTE*)colorConvertedBuffer.get();

        switch (outFormat)
        {
        case DXGI_FORMAT_R8G8B8A8_UNORM:        
        {            
            // Just do the copy
            UINT pSysPitch = texWidth * outTextureBytesPerPixel;
            for (UINT k = 0; k < texHeight; k++)
            {
                memcpy(colorConvertedBufferRaw, pTexels, pSysPitch);
                colorConvertedBufferRaw += pSysPitch;
                pTexels += pSysPitch;
            }

            return colorConvertedBuffer;
        }

        case DXGI_FORMAT_R8G8_UNORM:
        {                                
            // Extract only red and green colors
            for (UINT k = 0; k < texHeight; k++)
            {
                for (UINT i = 0; i < texWidth * 4; i += 4)
                {
                    BYTE red = pTexels[i];
                    BYTE green = pTexels[i + 1];

                    *colorConvertedBufferRaw++ = (BYTE)red;
                    *colorConvertedBufferRaw++ = (BYTE)green;
                }
                pTexels += texWidth * 4;
            }
            return colorConvertedBuffer;
        }

        case DXGI_FORMAT_R8_UNORM:
        {
            // Extract only red color
            for (UINT k = 0; k < texHeight; k++)
            {
                for (UINT i = 0; i < texWidth * 4; i += 4)
                {
                    BYTE red = pTexels[i];
                    *colorConvertedBufferRaw++ = (BYTE)red;
                }
                pTexels += texWidth * 4;
            }
            return colorConvertedBuffer;
        }

        case DXGI_FORMAT_A8_UNORM:
        {
            // Fill with some red-based alpha
            for (UINT k = 0; k < texHeight; k++)
            {
                for (UINT i = 0; i < texWidth * 4; i += 4)
                {                   
					 BYTE red = pTexels[i];
                    *colorConvertedBufferRaw++ = (BYTE)red;
                }
                pTexels += texWidth * 4;
            }

            return colorConvertedBuffer;
          }

        default:
        {
            throw std::exception("Not implemented texture color format passed");
        }
        }

    }

    D3DBitmapTexture(std::shared_ptr<D3DDevice> & inDevice, DXGI_FORMAT textureFormat = DXGI_FORMAT_R8G8B8A8_UNORM)
    {
        ULONG texWidth, texHeight, resSize;
        PVOID pRes;
        BYTE* pTexels = NULL;

        pRes = LoadTextureResource(1010, &resSize);
        if (!pRes)
        {
            throw std::exception("Failed to load bitmap resource");
        }
        HRESULT hr = LoadBMP((BYTE*)pRes, &texWidth, &texHeight, &pTexels);
        if (FAILED(hr))
        {
            throw std::exception("Failed to load bitmap resource");
        }

        D3D11_TEXTURE2D_DESC desc;

        desc.CPUAccessFlags = 0;
        desc.Width = texWidth;
        desc.Height = texHeight;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = textureFormat;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        desc.MiscFlags = 0;

        D3D11_SUBRESOURCE_DATA initData;

        ZeroMemory(&initData, sizeof(initData));
        UINT bytesPerPixel = 0;

        auto texture = ConvertTextureFromRGBA(pTexels, textureFormat, texWidth, texHeight, bytesPerPixel);
        initData.SysMemPitch = texWidth * bytesPerPixel;
        initData.SysMemSlicePitch = texWidth * texHeight * bytesPerPixel;
        initData.pSysMem = texture.get();
        ID3D11Texture2D * pTexture;

        hr = inDevice->GetDevice()->CreateTexture2D(&desc, &initData, &pTexture);

        if (FAILED(hr))
        {
            throw std::exception("Failed to create texture");
        }

        ID3D11ShaderResourceView *  pShaderResourceView;

        hr = inDevice->GetDevice()->CreateShaderResourceView(pTexture, NULL, &pShaderResourceView);
        if (FAILED(hr))
        {
            throw std::exception("Failed to create shader resource view");
        }

        m_pTexture = pTexture;
        m_pDevice = inDevice;
        m_pShaderResourceView = pShaderResourceView;
    }

    ~D3DBitmapTexture()
    {
        // do nothing
    }

    PVOID LoadTextureResource(INT Name, DWORD *pdwSize)
    {
        HRSRC hRsrc = ::FindResourceExW(NULL, (WCHAR*)RT_RCDATA, (WCHAR*)MAKEINTRESOURCE(Name), 0);
        if (hRsrc == NULL)
        {
            return NULL;
        }
        HGLOBAL hRes = ::LoadResource(NULL, hRsrc);
        if (hRes == NULL)
        {
            return NULL;
        }
        *pdwSize = ::SizeofResource(NULL, hRsrc);
        return (LockResource(hRes));
    }

    ID3D11Texture2D * GetTexture() { return m_pTexture; }
    ID3D11ShaderResourceView * GetShaderResourceView() { return m_pShaderResourceView; }

private:

    D3DPointer<ID3D11Texture2D>             m_pTexture;
    D3DPointer<ID3D11ShaderResourceView>    m_pShaderResourceView;
    std::shared_ptr<D3DDevice>              m_pDevice;
};

class D3DRenderTarget
{
public:

    D3DRenderTarget(std::shared_ptr<D3DDevice> & inDevice, int inWidth, int inHeight)
    {
        D3D11_TEXTURE2D_DESC desc;

        desc.ArraySize = 1;
        desc.BindFlags = D3D11_BIND_RENDER_TARGET;
        desc.CPUAccessFlags = 0;
#if USE_BGRA_RT
        desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
#else
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
#endif // USE_BGRA_RT
        desc.Height = inHeight;

        desc.MipLevels = 1;
        desc.MiscFlags = 0;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.Width = inWidth;

        ID3D11Texture2D * pRenderTarget;
        HRESULT hr = inDevice->GetDevice()->CreateTexture2D(&desc, NULL, &pRenderTarget);
        if (FAILED(hr)) throw std::exception("Failed to create render target texture");
        m_pRenderTarget = pRenderTarget;

        ID3D11RenderTargetView * pRenderTargetView;
        hr = inDevice->GetDevice()->CreateRenderTargetView(pRenderTarget, nullptr, &pRenderTargetView);
        if (FAILED(hr)) throw std::exception("Failed to create render target view");
        m_pRenderTargetView = pRenderTargetView;

        m_pDevice = inDevice;
    }

    ~D3DRenderTarget()
    {
        // do nothing
    }

    ID3D11Texture2D * GetRenderTarget() { return m_pRenderTarget; }
    ID3D11RenderTargetView * GetRenderTargetView() { return m_pRenderTargetView; }

private:

    D3DPointer<ID3D11Texture2D>         m_pRenderTarget;
    D3DPointer<ID3D11RenderTargetView>  m_pRenderTargetView;
    std::shared_ptr<D3DDevice>          m_pDevice;
};

class D3DShader
{
public:

protected:

    HRESULT CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
    {
        HRESULT hr = S_OK;

        DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
        // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
        // Setting this flag improves the shader debugging experience, but still allows 
        // the shaders to be optimized and to run exactly the way they will run in 
        // the release configuration of this program.
        dwShaderFlags |= D3DCOMPILE_DEBUG;

        // Disable optimizations to further improve shader debugging
        dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

        ID3DBlob* pErrorBlob = nullptr;
        hr = D3DCompileFromFile(szFileName, nullptr, nullptr, szEntryPoint, szShaderModel,
            dwShaderFlags, 0, ppBlobOut, &pErrorBlob);
        if (FAILED(hr))
        {
            if (pErrorBlob)
            {
                OutputDebugStringA(reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()));
                pErrorBlob->Release();
            }
            return hr;
        }
        if (pErrorBlob) pErrorBlob->Release();

        return S_OK;
    }

};

class D3DVertexShader : public D3DShader
{
public:

    D3DVertexShader(std::shared_ptr<D3DDevice> & inDevice)
    {
        ID3DBlob* pVSBlob = nullptr;

#if USE_TEX
        HRESULT hr = CompileShaderFromFile(L"VC4Test-Cube_Tex.fx", "VS", "vs_4_0_level_9_1", &pVSBlob);
#else
        HRESULT hr = CompileShaderFromFile(L"VC4Test-Cube_Color.fx", "VS", "vs_4_0_level_9_1", &pVSBlob);
#endif // USE_TEX
        if (FAILED(hr)) throw std::exception("Failed to compile shader from file");

        D3DPointer<ID3DBlob> vsBlob;

        vsBlob = pVSBlob;

        // Create the vertex shader
        ID3D11VertexShader * pVertexShader;
        hr = inDevice->GetDevice()->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &pVertexShader);
        if (FAILED(hr)) throw std::exception("Failed to create vertex shader");

        m_pVertexShader = pVertexShader;

        // Define the input layout
        D3D11_INPUT_ELEMENT_DESC layout[] =
        {
#if USE_TEX
            { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
#else
            { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
#endif // USE_TEX
        };
        UINT numElements = ARRAYSIZE(layout);

        // Create the input layout
        ID3D11InputLayout * pVertexLayout;
        hr = inDevice->GetDevice()->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(),
            pVSBlob->GetBufferSize(), &pVertexLayout);
        if (FAILED(hr)) throw std::exception("Failed to create input layout");

        m_pVertexLayout = pVertexLayout;

    }

    ~D3DVertexShader()
    {

    }

    ID3D11VertexShader * GetVertexShader() { return m_pVertexShader; }
    ID3D11InputLayout * GetVertexLayout() { return m_pVertexLayout; }

private:

    D3DPointer<ID3D11VertexShader> m_pVertexShader;
    D3DPointer<ID3D11InputLayout> m_pVertexLayout;

};

class D3DPixelShader : public D3DShader
{
public:

    D3DPixelShader(std::shared_ptr<D3DDevice> & inDevice)
    {
        // Compile the pixel shader
        ID3DBlob* pPSBlob = nullptr;

#if USE_TEX
        HRESULT hr = CompileShaderFromFile(L"VC4Test-Cube_Tex.fx", "PS", "ps_4_0_level_9_1", &pPSBlob);
#else
        HRESULT hr = CompileShaderFromFile(L"VC4Test-Cube_Color.fx", "PS", "ps_4_0_level_9_1", &pPSBlob);
#endif // USE_TEX
        if (FAILED(hr)) throw std::exception("Unable to compile pixel shader");

        D3DPointer<ID3DBlob> psBlob;

        psBlob = pPSBlob;

        // Create the pixel shader
        ID3D11PixelShader * pPixelShader;
        hr = inDevice->GetDevice()->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &pPixelShader);
        if (FAILED(hr)) throw std::exception("Unable to create pixel shader");

        m_pPixelShader = pPixelShader;
    }

    ~D3DPixelShader()
    {
        // do nothing
    }

    ID3D11PixelShader * GetPixelShader() { return m_pPixelShader; }

private:

    D3DPointer<ID3D11PixelShader> m_pPixelShader;

};

class D3DVertexBuffer
{
public:

    D3DVertexBuffer(std::shared_ptr<D3DDevice> & inDevice)
    {
#if USE_QUAD
        SimpleVertex vertices[] =
        {
            //              X,      Y,    Z,    W,              U,    V // See Input layout.
            { XMFLOAT4(-1.00f,  1.00f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
            { XMFLOAT4( 1.00f,  1.00f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) },
            { XMFLOAT4(-1.00f, -1.00f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
            { XMFLOAT4( 1.00f, -1.00f, 0.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) }
        };
#elif USE_TEX
        SimpleVertex vertices[] =
        {
            { XMFLOAT4(-1.0f, 1.0f, -1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) },
            { XMFLOAT4( 1.0f, 1.0f, -1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
            { XMFLOAT4( 1.0f, 1.0f,  1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
            { XMFLOAT4(-1.0f, 1.0f,  1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },

            { XMFLOAT4(-1.0f, -1.0f, -1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
            { XMFLOAT4( 1.0f, -1.0f, -1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) },
            { XMFLOAT4( 1.0f, -1.0f,  1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
            { XMFLOAT4(-1.0f, -1.0f,  1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },

            { XMFLOAT4(-1.0f, -1.0f,  1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
            { XMFLOAT4(-1.0f, -1.0f, -1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
            { XMFLOAT4(-1.0f,  1.0f, -1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) },
            { XMFLOAT4(-1.0f,  1.0f,  1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },

            { XMFLOAT4( 1.0f, -1.0f,  1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
            { XMFLOAT4( 1.0f, -1.0f, -1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
            { XMFLOAT4( 1.0f,  1.0f, -1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
            { XMFLOAT4( 1.0f,  1.0f,  1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) },

            { XMFLOAT4(-1.0f, -1.0f, -1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
            { XMFLOAT4( 1.0f, -1.0f, -1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
            { XMFLOAT4( 1.0f,  1.0f, -1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) },
            { XMFLOAT4(-1.0f,  1.0f, -1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },

            { XMFLOAT4(-1.0f, -1.0f,  1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
            { XMFLOAT4( 1.0f, -1.0f,  1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
            { XMFLOAT4( 1.0f,  1.0f,  1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
            { XMFLOAT4(-1.0f,  1.0f,  1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) },
        };
#else
        SimpleVertex vertices[] =
        {
            { XMFLOAT4(-1.0f,  1.0f, -1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },
            { XMFLOAT4( 1.0f,  1.0f, -1.0f, 1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
            { XMFLOAT4( 1.0f,  1.0f,  1.0f, 1.0f), XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f) },
            { XMFLOAT4(-1.0f,  1.0f,  1.0f, 1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
            { XMFLOAT4(-1.0f, -1.0f, -1.0f, 1.0f), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f) },
            { XMFLOAT4( 1.0f, -1.0f, -1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) },
            { XMFLOAT4( 1.0f, -1.0f,  1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) },
            { XMFLOAT4(-1.0f, -1.0f,  1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) },
        };
#endif // USE_TEX

        D3D11_BUFFER_DESC bd;
        ZeroMemory(&bd, sizeof(bd));
        bd.Usage = D3D11_USAGE_DEFAULT;
        bd.ByteWidth = sizeof(vertices);
        bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bd.CPUAccessFlags = 0;

        D3D11_SUBRESOURCE_DATA initData;
        ZeroMemory(&initData, sizeof(initData));
        initData.pSysMem = vertices;

        ID3D11Buffer * pVertexBuffer;
        HRESULT hr = inDevice->GetDevice()->CreateBuffer(&bd, &initData, &pVertexBuffer);
        if (FAILED(hr)) throw std::exception("Unable to create vertex buffer");

        m_pVertexBuffer = pVertexBuffer;
    }

    ~D3DVertexBuffer()
    {
        // do nothing
    }

    ID3D11Buffer * GetVertexBuffer() { return m_pVertexBuffer; }

private:

    D3DPointer<ID3D11Buffer> m_pVertexBuffer;

};

class D3DIndexBuffer
{
public:

    D3DIndexBuffer(std::shared_ptr<D3DDevice> & inDevice)
    {
#if USE_QUAD
        USHORT indices[] =
        {
            0,
            1,
            2,
            3
        };
#elif USE_TEX
        USHORT indices[] =
        {
            3,1,0,
            2,1,3,

            6,4,5,
            7,4,6,

            11,9,8,
            10,9,11,

            14,12,13,
            15,12,14,

            19,17,16,
            18,17,19,

            22,20,21,
            23,20,22
        };
#else
        USHORT indices[] =
        {
            3,1,0,
            2,1,3,

            0,5,4,
            1,5,0,

            3,4,7,
            0,4,3,

            1,6,5,
            2,6,1,

            2,7,6,
            3,7,2,

            6,4,5,
            7,4,6,
        };
#endif // USE_QUAD

        D3D11_BUFFER_DESC bd;
        ZeroMemory(&bd, sizeof(bd));
        bd.Usage = D3D11_USAGE_DEFAULT;
        bd.ByteWidth = sizeof(indices);
        bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
        bd.CPUAccessFlags = 0;

        D3D11_SUBRESOURCE_DATA initData;
        ZeroMemory(&initData, sizeof(initData));
        initData.pSysMem = indices;

        ID3D11Buffer * pIndexBuffer;
        HRESULT hr = inDevice->GetDevice()->CreateBuffer(&bd, &initData, &pIndexBuffer);
        if (FAILED(hr)) throw std::exception("Unable to create Index buffer");

        m_pIndexBuffer = pIndexBuffer;
    }

    ~D3DIndexBuffer()
    {
        // do nothing
    }

    ID3D11Buffer * GetIndexBuffer() { return m_pIndexBuffer; }

private:

    D3DPointer<ID3D11Buffer> m_pIndexBuffer;

};

class D3DDepthStencilBuffer
{

public:

    D3DDepthStencilBuffer(std::shared_ptr<D3DDevice> & inDevice, int inWidth, int inHeight)
    {
        D3D11_TEXTURE2D_DESC desc;

        desc.ArraySize = 1;
        desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        desc.CPUAccessFlags = 0;
        desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        desc.Height = inHeight;
        desc.MipLevels = 1;
        desc.MiscFlags = 0;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.Width = inWidth;

        ID3D11Texture2D * pDepthStencilBuffer;
        HRESULT hr = inDevice->GetDevice()->CreateTexture2D(&desc, NULL, &pDepthStencilBuffer);
        if (FAILED(hr))
        {
            throw std::exception("Failed to create depth stencil buffer");
        }
        
        m_pDepthStencilBuffer = pDepthStencilBuffer;

        ID3D11DepthStencilView * pDepthStencilView;
        hr = inDevice->GetDevice()->CreateDepthStencilView(pDepthStencilBuffer, nullptr, &pDepthStencilView);
        if (FAILED(hr))
        {
            throw std::exception("Failed to create depth stencil view");
        }

        m_pDepthStencilView = pDepthStencilView;
    }

    ~D3DDepthStencilBuffer()
    {
        // do nothing
    }

    ID3D11DepthStencilView * GetDepthStencilView() { return m_pDepthStencilView; }

private:

    D3DPointer<ID3D11Texture2D>         m_pDepthStencilBuffer;
    D3DPointer<ID3D11DepthStencilView>  m_pDepthStencilView;
};

class D3DDepthStencilState
{
public:

    D3DDepthStencilState(std::shared_ptr<D3DDevice> & inDevice)
    {
        D3D11_DEPTH_STENCIL_DESC depthStencilDesc = { 0 };

        depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;

#if 1

        depthStencilDesc.DepthEnable = 1;
        depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

#else

        depthStencilDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;

#endif

        HRESULT hr;
        ID3D11DepthStencilState * pDepthStencilState;

        hr = inDevice->GetDevice()->CreateDepthStencilState(&depthStencilDesc, &pDepthStencilState);
        if (FAILED(hr))
        {
            throw std::exception("Unable to create Depth Stencil State");
        }

        m_pDepthStencilState = pDepthStencilState;
    }

    ~D3DDepthStencilState()
    {
        // do nothing
    }

    ID3D11DepthStencilState * GetDepthStencilState() { return m_pDepthStencilState; }

private:

    D3DPointer<ID3D11DepthStencilState> m_pDepthStencilState;
};

//
// TODO[indyz]: Investigate : Texture address mode of clamp causes 1 triangle to miss texture
//

class D3DSamplerState
{
public:

    D3DSamplerState(std::shared_ptr<D3DDevice> & inDevice)
    {
        D3D11_SAMPLER_DESC samplerState;

        ZeroMemory(&samplerState, sizeof(samplerState));

        samplerState.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerState.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerState.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

        samplerState.MaxLOD = D3D11_FLOAT32_MAX;
        samplerState.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;

        HRESULT hr;
        ID3D11SamplerState * pSamplerState;

        hr = inDevice->GetDevice()->CreateSamplerState(&samplerState, &pSamplerState);
        if (FAILED(hr))
        {
            throw std::exception("Unable to create Sampler State");
        }

        m_pSamplerState = pSamplerState;
    }

    ~D3DSamplerState()
    {
        // do nothing
    }

    ID3D11SamplerState * GetSamplerState() { return m_pSamplerState; }

private:

    D3DPointer<ID3D11SamplerState> m_pSamplerState;
};

class D3DRasterizerState
{
public:

    D3DRasterizerState(std::shared_ptr<D3DDevice> & inDevice)
    {
        D3D11_RASTERIZER_DESC rasterizerState;

        ZeroMemory(&rasterizerState, sizeof(rasterizerState));

        rasterizerState.FillMode = D3D11_FILL_SOLID;
        rasterizerState.CullMode = D3D11_CULL_NONE;
        rasterizerState.FrontCounterClockwise = FALSE;
        rasterizerState.DepthBias = 0;
        rasterizerState.SlopeScaledDepthBias = 0.0f;
        rasterizerState.DepthBiasClamp = 0;
        rasterizerState.DepthClipEnable = true;
#if USE_SCISSOR
        rasterizerState.ScissorEnable = true;
#else
        rasterizerState.ScissorEnable = false;
#endif // USE_SCISSOR
        rasterizerState.MultisampleEnable = false;
        rasterizerState.AntialiasedLineEnable = false;

        HRESULT hr;
        ID3D11RasterizerState * pRasterizerState;

        hr = inDevice->GetDevice()->CreateRasterizerState(&rasterizerState, &pRasterizerState);
        if (FAILED(hr))
        {
            throw std::exception("Unable to create Rasterizer State");
        }

        m_pRasterizerState = pRasterizerState;
    }

    ~D3DRasterizerState()
    {
        // do nothing
    }

    ID3D11RasterizerState * GetRasterizerState() { return m_pRasterizerState; }

private:

    D3DPointer<ID3D11RasterizerState> m_pRasterizerState;
};

class D3DVSConstantBuffer
{
public:

    D3DVSConstantBuffer(std::shared_ptr<D3DDevice> & inDevice)
    {
        XMMATRIX mWorld;
        XMMATRIX mView;
        XMMATRIX mProjection;

#if NO_TRANSFORM
        mWorld = DirectX::XMMatrixIdentity();
        mView  = DirectX::XMMatrixIdentity();
        mProjection = DirectX::XMMatrixIdentity();
#else
        mWorld = DirectX::XMMatrixIdentity();
        
        DirectX::XMVECTOR Eye = DirectX::XMVectorSet(0.0f, 2.0f, -4.0f, 0.0f);
        DirectX::XMVECTOR At  = DirectX::XMVectorSet(0.0f, 0.0f,  0.0f, 0.0f);
        DirectX::XMVECTOR Up  = DirectX::XMVectorSet(0.0f, 1.0f,  0.0f, 0.0f);
        mView = DirectX::XMMatrixLookAtLH(Eye, At, Up);

        mProjection = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV2, (FLOAT)kWidth / (FLOAT)kHeight, 0.01f, 100.0f);
#endif // NO_TRANSFORM
        
        m_data.World = DirectX::XMMatrixTranspose(mWorld);
        m_data.View = DirectX::XMMatrixTranspose(mView);
        m_data.Projection = DirectX::XMMatrixTranspose(mProjection);

        D3D11_BUFFER_DESC bd;
        ZeroMemory(&bd, sizeof(bd));
#if USE_MAP_FOR_CONSTANT_UPDATE
        bd.Usage = D3D11_USAGE_DYNAMIC;
#else
        bd.Usage = D3D11_USAGE_DEFAULT;
#endif
        bd.ByteWidth = sizeof(m_data);
        bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
#if USE_MAP_FOR_CONSTANT_UPDATE
        bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
#else
        bd.CPUAccessFlags = 0;
#endif

        D3D11_SUBRESOURCE_DATA initData;
        ZeroMemory(&initData, sizeof(initData));
        initData.pSysMem = &m_data;

        ID3D11Buffer * pConstantBuffer;
        HRESULT hr = inDevice->GetDevice()->CreateBuffer(&bd, &initData, &pConstantBuffer);
        if (FAILED(hr)) throw std::exception("Unable to create Constant buffer");

        m_pVSConstantBuffer = pConstantBuffer;
    }

    ~D3DVSConstantBuffer()
    {
        // do nothing
    }

    ID3D11Buffer * GetConstantBuffer() { return m_pVSConstantBuffer; }

    VSConstantBuffer m_data;

private:

    D3DPointer<ID3D11Buffer> m_pVSConstantBuffer;
};

class D3DPSConstantBuffer
{
public:

    D3DPSConstantBuffer(std::shared_ptr<D3DDevice> & inDevice)
    {
        PSConstantBuffer data;
        data.Ambient = XMFLOAT4(2.0f, 2.0f, 4.0f, 0.0f);

        D3D11_BUFFER_DESC bd;
        ZeroMemory(&bd, sizeof(bd));
        bd.Usage = D3D11_USAGE_DEFAULT;
        bd.ByteWidth = sizeof(data);
        bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        bd.CPUAccessFlags = 0;

        D3D11_SUBRESOURCE_DATA initData;
        ZeroMemory(&initData, sizeof(initData));
        initData.pSysMem = &data;

        ID3D11Buffer * pConstantBuffer;
        HRESULT hr = inDevice->GetDevice()->CreateBuffer(&bd, &initData, &pConstantBuffer);
        if (FAILED(hr)) throw std::exception("Unable to create Constant buffer");

        m_pConstantBuffer = pConstantBuffer;
    }

    ~D3DPSConstantBuffer()
    {
        // do nothing
    }

    ID3D11Buffer * GetConstantBuffer() { return m_pConstantBuffer; }

private:

    D3DPointer<ID3D11Buffer> m_pConstantBuffer;
};

class D3DEngine
{
public:

    D3DEngine(DXGI_FORMAT textureFormat = DXGI_FORMAT_R8G8B8A8_UNORM)
    {

        m_textureFormat = textureFormat;
#if USE_VC4
        m_pDevice = std::shared_ptr<D3DDevice>(new D3DDevice(L"Render Only Sample Driver"));
#else
        m_pDevice = std::shared_ptr<D3DDevice>(new D3DDevice(L"Microsoft Basic Render Driver"));
#endif // VC4

        // Create render target

        m_pRenderTarget = std::unique_ptr<D3DRenderTarget>(new D3DRenderTarget(m_pDevice, kWidth, kHeight));

        ID3D11RenderTargetView * pRenderTargetView = m_pRenderTarget->GetRenderTargetView();

        // Create depth stencil buffer

        m_pDepthStencilBuffer = std::unique_ptr<D3DDepthStencilBuffer>(new D3DDepthStencilBuffer(m_pDevice, kWidth, kHeight));

        ID3D11DepthStencilView * pDepthStencilView = m_pDepthStencilBuffer->GetDepthStencilView();

        // Set render target and depth stencil buffer

        m_pDevice->GetContext()->OMSetRenderTargets(1, &pRenderTargetView, pDepthStencilView);

        // Setup the viewport

        D3D11_VIEWPORT vp;
        vp.Width = (FLOAT)kWidth;
        vp.Height = (FLOAT)kHeight;
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        vp.TopLeftX = 0;
        vp.TopLeftY = 0;
        m_pDevice->GetContext()->RSSetViewports(1, &vp);

        m_pVertexShader = std::unique_ptr<D3DVertexShader>(new D3DVertexShader(m_pDevice));

        m_pVSConstantBuffer = std::unique_ptr<D3DVSConstantBuffer>(new D3DVSConstantBuffer(m_pDevice));

        m_pDevice->GetContext()->IASetInputLayout(m_pVertexShader->GetVertexLayout());

        m_pPixelShader = std::unique_ptr<D3DPixelShader>(new D3DPixelShader(m_pDevice));

        m_pPSConstantBuffer = std::unique_ptr<D3DPSConstantBuffer>(new D3DPSConstantBuffer(m_pDevice));

#if USE_BITMAP_TEX
        m_pDefaultTexture = std::unique_ptr<D3DBitmapTexture>(new D3DBitmapTexture(m_pDevice, m_textureFormat));
#else
        m_pDefaultTexture = std::unique_ptr<D3DDefaultTexture>(new D3DDefaultTexture(m_pDevice, 256, 256));
#endif // USE_BITMAP_TEX

        m_pVertexBuffer = std::unique_ptr<D3DVertexBuffer>(new D3DVertexBuffer(m_pDevice));

        m_pIndexBuffer = std::unique_ptr<D3DIndexBuffer>(new D3DIndexBuffer(m_pDevice));

        m_pDepthStencilState = std::unique_ptr<D3DDepthStencilState>(new D3DDepthStencilState(m_pDevice));

        m_pSamplerState = std::unique_ptr<D3DSamplerState>(new D3DSamplerState(m_pDevice));

        m_pRasterizerState = std::unique_ptr<D3DRasterizerState>(new D3DRasterizerState(m_pDevice));

        // Set vertex buffer
        UINT stride = sizeof(SimpleVertex);
        UINT offset = 0;
        ID3D11Buffer * pVertexBuffer = m_pVertexBuffer->GetVertexBuffer();
        m_pDevice->GetContext()->IASetVertexBuffers(0, 1, &pVertexBuffer, &stride, &offset);

        // Set index buffer
        ID3D11Buffer * pIndexBuffer = m_pIndexBuffer->GetIndexBuffer();
        m_pDevice->GetContext()->IASetIndexBuffer(pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

        // Set primitive topology
#if USE_QUAD
        m_pDevice->GetContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
#else
        m_pDevice->GetContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
#endif // USE_QUAD

        // Set depth stencil state
        m_pDevice->GetContext()->OMSetDepthStencilState(m_pDepthStencilState->GetDepthStencilState(), 0);

        m_pStagingTexture = std::unique_ptr<D3DStagingTexture>(new D3DStagingTexture(m_pDevice, kWidth, kHeight));

        // Set sampler state
        ID3D11SamplerState *    pSamplerState = m_pSamplerState->GetSamplerState();
        m_pDevice->GetContext()->PSSetSamplers(0, 1, &pSamplerState);

        // Set rasterizer state
        m_pDevice->GetContext()->RSSetState(m_pRasterizerState->GetRasterizerState());

#if USE_SCISSOR
        RECT ScissorRect = { kWidth/2, 0, kWidth, kHeight};
        m_pDevice->GetContext()->RSSetScissorRects(1, &ScissorRect);
#endif // USE_SCISSOR
    
        m_pDevice->GetContext()->VSSetShader(m_pVertexShader->GetVertexShader(), nullptr, 0);

        ID3D11Buffer *pVSConstantBuffer = m_pVSConstantBuffer->GetConstantBuffer();
        m_pDevice->GetContext()->VSSetConstantBuffers(0, 1, &pVSConstantBuffer);

        m_pDevice->GetContext()->PSSetShader(m_pPixelShader->GetPixelShader(), nullptr, 0);

        ID3D11ShaderResourceView *pShaderResourceViews = m_pDefaultTexture->GetShaderResourceView();
        m_pDevice->GetContext()->PSSetShaderResources(0, 1, &pShaderResourceViews);

        ID3D11Buffer *pPSConstantBuffer = m_pPSConstantBuffer->GetConstantBuffer();
        m_pDevice->GetContext()->PSSetConstantBuffers(0, 1, &pPSConstantBuffer);
    }

    void Render(UINT iFrame, float fAngle, BOOL bPerfMode)
    {
        // Clear
        m_pDevice->GetContext()->ClearRenderTargetView(
            m_pRenderTarget->GetRenderTargetView(), 
            DirectX::Colors::MidnightBlue);

        m_pDevice->GetContext()->ClearDepthStencilView(
            m_pDepthStencilBuffer->GetDepthStencilView(),
            D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
            1.0f,
            0);

        // Change rotation.
        m_pVSConstantBuffer->m_data.World = XMMatrixTranspose(XMMatrixRotationY(fAngle));
#if USE_MAP_FOR_CONSTANT_UPDATE
        {
            ID3D11Resource *pResource;
            m_pVSConstantBuffer->GetConstantBuffer()->QueryInterface(&pResource);

            D3D11_MAPPED_SUBRESOURCE MappedResource;
            m_pDevice->GetContext()->Map(pResource, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);

            CopyMemory(MappedResource.pData, reinterpret_cast<VOID*>(&m_pVSConstantBuffer->m_data), sizeof(VSConstantBuffer));

            m_pDevice->GetContext()->Unmap(m_pVSConstantBuffer->GetConstantBuffer(), 0);
            pResource->Release();
        }
#else
        m_pDevice->GetContext()->UpdateSubresource(
            m_pVSConstantBuffer->GetConstantBuffer(), 0, nullptr, 
            &(m_pVSConstantBuffer->m_data), sizeof(m_pVSConstantBuffer->m_data), 0);
#endif // USE_MAP_FOR_CONSTANT_UPDATE
        
        // Draw.
#if USE_QUAD
        m_pDevice->GetContext()->DrawIndexed(4, 0, 0);
#else
        m_pDevice->GetContext()->DrawIndexed(36, 0, 0);
#endif // USE_QUAD

        if (! bPerfMode)
        {
            // Save frame.
            m_pDevice->GetContext()->CopyResource(m_pStagingTexture->GetTexture(), m_pRenderTarget->GetRenderTarget());
            {
                char fileName[MAX_PATH];
#if USE_VC4
                sprintf_s(fileName, MAX_PATH, "c:\\temp\\image_%s_%d_vc4.bmp", DXGIFormatToString(m_textureFormat),iFrame);
#else
                sprintf_s(fileName, MAX_PATH, "c:\\temp\\image_%d_warp.bmp", iFrame);
#endif // USE_VC4
                m_pStagingTexture->WriteToBmp(fileName);
            }
        }
    }

    ~D3DEngine()
    {
        // do nothing
    }

    std::unique_ptr<D3DStagingTexture> & GetTexture() { return m_pStagingTexture; }

    ID3D11Device* GetDevice() { return m_pDevice->GetDevice(); }
    ID3D11DeviceContext* GetContext() { return m_pDevice->GetContext(); }

private:

    std::shared_ptr<D3DDevice>              m_pDevice;
    std::unique_ptr<D3DRenderTarget>        m_pRenderTarget;
    std::unique_ptr<D3DDepthStencilBuffer>  m_pDepthStencilBuffer;
    std::unique_ptr<D3DVertexShader>        m_pVertexShader;
    std::unique_ptr<D3DPixelShader>         m_pPixelShader;
    std::unique_ptr<D3DVertexBuffer>        m_pVertexBuffer;
    std::unique_ptr<D3DIndexBuffer>         m_pIndexBuffer;
    std::unique_ptr<D3DStagingTexture>      m_pStagingTexture;
    std::unique_ptr<D3DDepthStencilState>   m_pDepthStencilState;
    std::unique_ptr<D3DRasterizerState>     m_pRasterizerState;
    std::unique_ptr<D3DSamplerState>        m_pSamplerState;
    std::unique_ptr<D3DVSConstantBuffer>    m_pVSConstantBuffer;
    std::unique_ptr<D3DPSConstantBuffer>    m_pPSConstantBuffer;
    DXGI_FORMAT                             m_textureFormat;
#if USE_BITMAP_TEX
    std::unique_ptr<D3DBitmapTexture>       m_pDefaultTexture;
#else
    std::unique_ptr<D3DDefaultTexture>      m_pDefaultTexture;
#endif // USE_BITMAP_TEX
};

#define SAFE_RELEASE(x) { if (x) { (x)->Release(); (x) = NULL; } }

ID3D11Device*           pd3dDevice = NULL;
ID3D11DeviceContext*    pd3dContext = NULL;
IDXGIDevice2*           pDxgiDev2 = NULL;
HANDLE                  hQueueEvent = NULL;

void UninitPerf()
{
    SAFE_RELEASE(pDxgiDev2);

    if (hQueueEvent)
    {
        CloseHandle(hQueueEvent);
    }
}

BOOL InitPerf()
{
    pd3dDevice->QueryInterface<IDXGIDevice2>(&pDxgiDev2);

    // Create a manual-reset event object.
    hQueueEvent = CreateEvent(
        NULL,               // default security attributes
        TRUE,               // manual-reset event
        FALSE,              // initial state is nonsignaled
        FALSE
        );

    return hQueueEvent ? true : false;
}

const UINT TESTED_FORMATS_COUNT = 4;

int main(int argc, char* argv[])
{

    DXGI_FORMAT formatsToTest[TESTED_FORMATS_COUNT] =
    {
        DXGI_FORMAT_R8G8B8A8_UNORM,
        DXGI_FORMAT_R8G8_UNORM,
        DXGI_FORMAT_R8_UNORM,
        DXGI_FORMAT_A8_UNORM
    };

    BOOL bPerfMode = false;

    LARGE_INTEGER   framesStart;
    LARGE_INTEGER   framesEnd;

    LARGE_INTEGER   frequenceStart;
    LARGE_INTEGER   frequenceEnd;

    if (argc >= 3)
    {
        bPerfMode = true;
    }

    if (bPerfMode)
    {
        sscanf_s(argv[1], "%d", &kWidth);
        sscanf_s(argv[2], "%d", &kHeight);

        if (argc > 3)
        {
            sscanf_s(argv[3], "%d", &frames);

            if (frames < 20)
            {
                frames = 20;
            }
        }
        else
        {
            frames = 20;
        }
    }

    for (int currentTest = 0; currentTest < TESTED_FORMATS_COUNT; currentTest++)
    {
        printf("Test for [%s] \n", DXGIFormatToString(formatsToTest[currentTest]));
        try
        {
            D3DEngine engine(formatsToTest[currentTest]);

            if (bPerfMode)
            {
                pd3dDevice = engine.GetDevice();
                pd3dContext = engine.GetContext();

                InitPerf();
            }

            float t = 0.0f;
            for (UINT i = 0; i < frames; i++)
            {
                // Skip the 1st frame for shader compilation time
                if (bPerfMode && (i == 1))
                {
                    QueryPerformanceCounter(&framesStart);
                    QueryPerformanceFrequency(&frequenceStart);
                }

                t += (float)XM_PI * 0.125f;
                engine.Render(i, t, bPerfMode);

                if (bPerfMode)
                {
                    if (i == 0)
                    {
                        //
                        // Wait for the 1st frame to finish to account for the GPU paging cost
                        //

                        DWORD dwWaitResult;

                        pDxgiDev2->EnqueueSetEvent(hQueueEvent);

                        dwWaitResult = WaitForSingleObject(
                            hQueueEvent,    // event handle
                            INFINITE);      // indefinite wait

                        ResetEvent(hQueueEvent);
                    }
                    else if (i < (frames - 1))
                    {
                        pd3dContext->Flush();
                    }
                    else
                    {
                        pDxgiDev2->EnqueueSetEvent(hQueueEvent);
                    }
                }
            }

            if (bPerfMode)
            {
                DWORD dwWaitResult;

                dwWaitResult = WaitForSingleObject(
                    hQueueEvent,    // event handle
                    INFINITE);      // indefinite wait

                if (dwWaitResult == WAIT_OBJECT_0)
                {
                    QueryPerformanceCounter(&framesEnd);
                    QueryPerformanceFrequency(&frequenceEnd);

                    UINT measuredFrames = frames - 1;

                    if (frequenceStart.QuadPart != frequenceEnd.QuadPart)
                    {
                        printf("Perf frequence changed during %d frames of rendering", measuredFrames);
                    }

                    printf(
                        "Average rendering time for (%d x %d) from %d frames: %I64d ms\n",
                        kWidth,
                        kHeight,
                        measuredFrames,
                        ((framesEnd.QuadPart - framesStart.QuadPart) * 1000) / (measuredFrames*frequenceEnd.QuadPart));
                }

                UninitPerf();
            }

            printf("Success\n");
        }

        catch (const std::exception &e)
        {
            printf("ERROR: %s\n", e.what());
        }
    }

    printf("Done\n");

    return 0;
}

