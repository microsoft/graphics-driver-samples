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

//using namespace DirectX;

//
// Only works when CUBETEST_CVFS is defined in RosCompiler.cpp
//

#define CUBETEST_CVFS     1

#if VC4

#pragma pack(push, 1)

struct SimpleVertex
{
    FLOAT   x;
    FLOAT   y;
    FLOAT   z;
    FLOAT   w;
    FLOAT   u;
    FLOAT   v;
};

#pragma pack(pop)

#else

struct SimpleVertex
{
    DirectX::XMFLOAT3 Pos;
};

#endif

#if 0

static const int kWidth = 1920;
static const int kHeight = 1080;

#else

static const int kWidth = 256;
static const int kHeight = 256;

#endif

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
        D3D_FEATURE_LEVEL requestedLevel = D3D_FEATURE_LEVEL_11_1;

        ID3D11Device * pDevice;
        ID3D11DeviceContext * pContext;

        HRESULT hr = D3D11CreateDevice(m_pAdapter, D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_UNKNOWN, NULL, 0, &requestedLevel, 1, D3D11_SDK_VERSION, &pDevice, &level, &pContext);

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

class D3DTexture
{
public:

    D3DTexture(std::shared_ptr<D3DDevice> & inDevice, int inWidth, int inHeight)
    {
        D3D11_TEXTURE2D_DESC desc;

        desc.ArraySize = 1;
        desc.BindFlags = 0;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
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

    ~D3DTexture()
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

#pragma pack(pop)

        D3D11_TEXTURE2D_DESC desc;
        m_pTexture->GetDesc(&desc);

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
                fwrite(pRow + 2, 1, 1, fp);
                fwrite(pRow + 1, 1, 1, fp);
                fwrite(pRow + 0, 1, 1, fp);

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
        ULONG texWidth, texHeight, resSize;
        PVOID pRes;
        BYTE* pTexels = NULL;

        pRes = LoadResource(1010, &resSize);
        LoadBMP((BYTE*)pRes, &texWidth, &texHeight, &pTexels);
        
        D3D11_TEXTURE2D_DESC desc;

        desc.CPUAccessFlags = 0;
        desc.Width = texWidth;
        desc.Height = texHeight;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        desc.MiscFlags = 0;



        /*/

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

        */

        D3D11_SUBRESOURCE_DATA initData;

        ZeroMemory(&initData, sizeof(initData));
        initData.pSysMem = pTexels;
        initData.SysMemPitch = inWidth*4;
        initData.SysMemSlicePitch = inWidth*4*inHeight;

        ID3D11Texture2D * pTexture;

        HRESULT hr = inDevice->GetDevice()->CreateTexture2D(&desc, &initData, &pTexture);

        delete pTexels;

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

    PVOID LoadResource(INT Name, DWORD *pdwSize)
    {
        HRSRC hRsrc = ::FindResourceEx(NULL, RT_RCDATA, MAKEINTRESOURCE(Name), 0);
        if (hRsrc == NULL)
        {
            __debugbreak();
            return NULL;
        }
        HGLOBAL hRes = ::LoadResource(NULL, hRsrc);
        if (hRes == NULL)
        {
            __debugbreak();
            return NULL;
        }
        *pdwSize = ::SizeofResource(NULL, hRsrc);
        return (LockResource(hRes));
    }

    ID3D11Texture2D * GetTexture() { return m_pTexture; }
    ID3D11ShaderResourceView * GetShaderResourceView() { return m_pShaderResourceView;  }

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
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
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

#if VC4

        HRESULT hr = CompileShaderFromFile(L"VC4Test-Cube.fx", "VS", "vs_4_0_level_9_1", &pVSBlob);

#else

        HRESULT hr = CompileShaderFromFile(L"Tutorial02.fx", "VS", "vs_4_0", &pVSBlob);

#endif

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
            { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
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

#if VC4

        HRESULT hr = CompileShaderFromFile(L"VC4Test-Cube.fx", "PS", "ps_4_0_level_9_1", &pPSBlob);

#else

        HRESULT hr = CompileShaderFromFile(L"Tutorial02.fx", "PS", "ps_4_0", &pPSBlob);

#endif
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
#if VC4

#if 1
        SimpleVertex vertices[] =
        {
            //     X,      Y,    Z,    W,    U,    V // See Input layout.
            { -1.50f,  1.50f, 0.0f, 2.0f, 0.0f, 0.0f },
            {  1.50f,  1.50f, 0.0f, 2.0f, 1.0f, 0.0f },
            { -1.50f, -1.50f, 0.0f, 2.0f, 0.0f, 1.0f },
            {  1.50f, -1.50f, 0.0f, 2.0f, 1.0f, 1.0f }
        };

#else

        SimpleVertex vertices[] =
        {
            //     X,      Y,    Z,    W,    U,    V // See Input layout.
            { -0.75f,  0.75f, 0.0f, 1.0f, 0.0f, 0.0f },
            {  0.75f,  0.75f, 0.0f, 1.0f, 1.0f, 0.0f },
            { -0.75f, -0.75f, 0.0f, 1.0f, 0.0f, 1.0f },
            {  0.75f, -0.75f, 0.0f, 1.0f, 1.0f, 1.0f }
        };

#endif

#else

        SimpleVertex vertices[] =
        {
            DirectX::XMFLOAT3(0.0f, 0.5f, 0.5f),
            DirectX::XMFLOAT3(0.5f, -0.5f, 0.5f),
            DirectX::XMFLOAT3(-0.5f, -0.5f, 0.5f),
        };

#endif

#if 0
        {
            DirectX::XMMATRIX mWorld;
            DirectX::XMMATRIX mView;
            DirectX::XMMATRIX mProjection;

            mWorld = DirectX::XMMatrixRotationY(15.0f);

            DirectX::XMVECTOR Eye = DirectX::XMVectorSet(0.0f, 0.0f, -2.0f, 0.0f);
            DirectX::XMVECTOR At = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
            DirectX::XMVECTOR Up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
            mView = DirectX::XMMatrixLookAtLH(Eye, At, Up);

            mProjection = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV2, (FLOAT)kWidth / (FLOAT)kHeight, 0.01f, 100.0f);

            for (uint8_t i = 0; i < ARRAYSIZE(vertices); i++)
            {
                DirectX::XMVECTOR v = DirectX::XMVectorSet(vertices[i].x, vertices[i].y, vertices[i].z, vertices[i].w);

                v = DirectX::XMVector4Transform(v, mWorld);
                v = DirectX::XMVector4Transform(v, mView);
                v = DirectX::XMVector4Transform(v, mProjection);

                DirectX::XMFLOAT4 f4;
                DirectX::XMStoreFloat4(&f4, v);

                vertices[i].x = f4.x;
                vertices[i].y = f4.y;
                vertices[i].z = 0.0f;
                vertices[i].w = f4.w;
            }
        }
#endif
        
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
        USHORT indices[] =
        {
            0,
            1,
            2,
            3
        };

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

class D3DRasterState
{
public:

    D3DRasterState(std::shared_ptr<D3DDevice> &inDevice)
    {
        D3D11_RASTERIZER_DESC rd;

        memset(&rd, 0, sizeof(rd));
        rd.FillMode = D3D11_FILL_SOLID;
        rd.CullMode = D3D11_CULL_NONE;
        rd.DepthClipEnable = TRUE;

        HRESULT hr;
        ID3D11RasterizerState*  pRasterState;

        hr = inDevice->GetDevice()->CreateRasterizerState(&rd, &pRasterState);
        if (FAILED(hr))
        {
            throw std::exception("Unable to create Depth Stencil State");
        }

        m_pRasterState = pRasterState;
    }

    ~D3DRasterState()
    {
        // do nothing
    }

    ID3D11RasterizerState* GetRasterState() { return m_pRasterState; }

private:

    ID3D11RasterizerState*  m_pRasterState;
};

#if 0

class D3DConstantBuffer
{
public:

    D3DConstantBuffer(std::shared_ptr<D3DDevice> & inDevice)
    {
        // TODO: The XY scaling should be generated by shader comiler internally

        FLOAT data[] =
        {
            kWidth*16.0f/2.0f,
           -kHeight*16.0f/2.0f,
            1.0f,
            0.0f,

            // For Coordinate Shader, not used
            kWidth*16.0f/2.0f,
           -kHeight*16.0f/2.0f,
            0.0f,
            0.0f,
        };

        D3D11_BUFFER_DESC bd;
        ZeroMemory(&bd, sizeof(bd));
        bd.Usage = D3D11_USAGE_DEFAULT;
        bd.ByteWidth = sizeof(data);
        bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        bd.CPUAccessFlags = 0;

        D3D11_SUBRESOURCE_DATA initData;
        ZeroMemory(&initData, sizeof(initData));
        initData.pSysMem = data;

        ID3D11Buffer * pConstantBuffer;
        HRESULT hr = inDevice->GetDevice()->CreateBuffer(&bd, &initData, &pConstantBuffer);
        if (FAILED(hr)) throw std::exception("Unable to create Constant buffer");

        m_pConstantBuffer = pConstantBuffer;
    }

    ~D3DConstantBuffer()
    {
        // do nothing
    }

    ID3D11Buffer * GetConstantBuffer() { return m_pConstantBuffer; }

private:

    D3DPointer<ID3D11Buffer> m_pConstantBuffer;

};

#endif

class D3DPSConstantBuffer
{
public:

    D3DPSConstantBuffer(std::shared_ptr<D3DDevice> & inDevice)
    {
        FLOAT data[] =
        {
            2.0f,
            2.0f,
            4.0f,
            0.0f
        };

        D3D11_BUFFER_DESC bd;
        ZeroMemory(&bd, sizeof(bd));
        bd.Usage = D3D11_USAGE_DEFAULT;
        bd.ByteWidth = sizeof(data);
        bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        bd.CPUAccessFlags = 0;

        D3D11_SUBRESOURCE_DATA initData;
        ZeroMemory(&initData, sizeof(initData));
        initData.pSysMem = data;

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

    D3DEngine()
    {
        m_pDevice = std::shared_ptr<D3DDevice>(new D3DDevice(L"Render Only Sample Driver"));
        // m_pDevice = std::shared_ptr<D3DDevice>(new D3DDevice(L"Microsoft Basic Render Driver"));

        // Create render target

        m_pRenderTarget = std::unique_ptr<D3DRenderTarget>(new D3DRenderTarget(m_pDevice, kWidth, kHeight));

        ID3D11RenderTargetView * pRenderTargetView = m_pRenderTarget->GetRenderTargetView();

        // Create depth stencil buffer

        m_pDepthStencilBuffer = std::unique_ptr<D3DDepthStencilBuffer>(new D3DDepthStencilBuffer(m_pDevice, kWidth, kHeight));

        ID3D11DepthStencilView * pDepthStencilView = m_pDepthStencilBuffer->GetDepthStencilView();

        m_pRasterState = std::unique_ptr<D3DRasterState>(new D3DRasterState(m_pDevice));

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

#if 0

        m_pConstantBuffer = std::unique_ptr<D3DConstantBuffer>(new D3DConstantBuffer(m_pDevice));

#endif

        m_pDevice->GetContext()->IASetInputLayout(m_pVertexShader->GetVertexLayout());

        m_pPixelShader = std::unique_ptr<D3DPixelShader>(new D3DPixelShader(m_pDevice));

        m_pPSConstantBuffer = std::unique_ptr<D3DPSConstantBuffer>(new D3DPSConstantBuffer(m_pDevice));

        m_pDefaultTexture = std::unique_ptr<D3DDefaultTexture>(new D3DDefaultTexture(m_pDevice, kTexWidth, kTexHeight));

        m_pVertexBuffer = std::unique_ptr<D3DVertexBuffer>(new D3DVertexBuffer(m_pDevice));

        m_pIndexBuffer = std::unique_ptr<D3DIndexBuffer>(new D3DIndexBuffer(m_pDevice));

        m_pDepthStencilState = std::unique_ptr<D3DDepthStencilState>(new D3DDepthStencilState(m_pDevice));

        // Set vertex buffer
        UINT stride = sizeof(SimpleVertex);
        UINT offset = 0;
        ID3D11Buffer * pVertexBuffer = m_pVertexBuffer->GetVertexBuffer();
        m_pDevice->GetContext()->IASetVertexBuffers(0, 1, &pVertexBuffer, &stride, &offset);

        // Set index buffer
        ID3D11Buffer * pIndexBuffer = m_pIndexBuffer->GetIndexBuffer();
        m_pDevice->GetContext()->IASetIndexBuffer(pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

        // Set primitive topology
        m_pDevice->GetContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

        // Set depth stencil state
        m_pDevice->GetContext()->OMSetDepthStencilState(m_pDepthStencilState->GetDepthStencilState(), 0);

        // Set raster state
        m_pDevice->GetContext()->RSSetState(m_pRasterState->GetRasterState());

        m_pTexture = std::unique_ptr<D3DTexture>(new D3DTexture(m_pDevice, kWidth, kHeight));
    }

    void Render()
    {
        m_pDevice->GetContext()->ClearRenderTargetView(m_pRenderTarget->GetRenderTargetView(), DirectX::Colors::MidnightBlue);

        m_pDevice->GetContext()->ClearDepthStencilView(
            m_pDepthStencilBuffer->GetDepthStencilView(),
            D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
            0.75,
            3);

        m_pDevice->GetContext()->VSSetShader(m_pVertexShader->GetVertexShader(), nullptr, 0);

#if 0

        ID3D11Buffer *    pConstantBuffer = m_pConstantBuffer->GetConstantBuffer();
        m_pDevice->GetContext()->VSSetConstantBuffers(0, 1, &pConstantBuffer);

#endif

        m_pDevice->GetContext()->PSSetShader(m_pPixelShader->GetPixelShader(), nullptr, 0);

        ID3D11ShaderResourceView *  pShaderResourceViews = m_pDefaultTexture->GetShaderResourceView();
        m_pDevice->GetContext()->PSSetShaderResources(0, 1, &pShaderResourceViews);

        ID3D11Buffer *  pPSConstantBuffer = m_pPSConstantBuffer->GetConstantBuffer();
        m_pDevice->GetContext()->PSSetConstantBuffers(0, 1, &pPSConstantBuffer);

#if 0

        for (UINT i = 0; i < 100; i++)
        {
            m_pDevice->GetContext()->Draw(3, 0);

            m_pDevice->GetContext()->Flush();

            m_pDevice->GetContext()->Draw(3, 1);
        }

#endif

#if 0
        for (UINT i = 0; i < 100; i++)
        {
            m_pDevice->GetContext()->Draw(4, 0);

            m_pDevice->GetContext()->Flush();
        }

#endif

#if 1

        m_pDevice->GetContext()->Draw(4, 0);

#else

        m_pDevice->GetContext()->DrawIndexed(4, 0, 0);

#endif

        m_pDevice->GetContext()->CopyResource(m_pTexture->GetTexture(), m_pRenderTarget->GetRenderTarget());

        m_pTexture->WriteToBmp("c:\\temp\\image.bmp");
    }

    ~D3DEngine()
    {
        // do nothing
    }

    std::unique_ptr<D3DTexture> & GetTexture() { return m_pTexture; }

private:

    std::shared_ptr<D3DDevice>              m_pDevice;
    std::unique_ptr<D3DRenderTarget>        m_pRenderTarget;
    std::unique_ptr<D3DDepthStencilBuffer>  m_pDepthStencilBuffer;
    std::unique_ptr<D3DVertexShader>        m_pVertexShader;
    std::unique_ptr<D3DPixelShader>         m_pPixelShader;
    std::unique_ptr<D3DVertexBuffer>        m_pVertexBuffer;
    std::unique_ptr<D3DIndexBuffer>         m_pIndexBuffer;
    std::unique_ptr<D3DTexture>             m_pTexture;
    std::unique_ptr<D3DDepthStencilState>   m_pDepthStencilState;
    std::unique_ptr<D3DRasterState>         m_pRasterState;
#if 0

    std::unique_ptr<D3DConstantBuffer>      m_pConstantBuffer;

#endif
    std::unique_ptr<D3DPSConstantBuffer>    m_pPSConstantBuffer;
    std::unique_ptr<D3DDefaultTexture>      m_pDefaultTexture;
};

int main()
{
#if CUBETEST_CVFS

    try
    {
        D3DEngine engine;

        engine.Render();

        printf("Success\n");
    }

    catch (std::exception & e)
    {
        printf("ERROR: %s\n", e.what());
    }

#endif

    printf("Done\n");

    return 0;
}

