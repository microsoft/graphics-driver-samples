#include "DolphinRender.h"

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

#include "BitmapDecode.h"
#include "SDKmesh.h"
#include "xTimer.h"

#include <math.h>
#include <directxmath.h>

#include "Resource.h"

using namespace DirectX;

#define MAX_LOADSTRING 100

ULONG ErrorLine = 0;
#define CHR(x) { hr = (x); if (FAILED(hr)) {ErrorLine = __LINE__; __debugbreak(); goto EXIT_RETURN; } }

#define SAFE_RELEASE(x) { if (x) { (x)->Release(); (x) = NULL; } }

xTimer AppTimer;
FLOAT fWaterColor[4] = { 0.0f, 0.5f, 1.0f, 1.0f };
FLOAT fAmbient[4] = { 0.25f, 0.25f, 0.25f, 0.25f };

// D3D Variables:
ID3D11Device*              pd3dDevice = NULL;
ID3D11DeviceContext*       pd3dContext = NULL;
ID3D11Texture2D*           pStaging = NULL;
ID3D11Resource*            pStagingResource = NULL;
ID3D11Texture2D*           pRenderTarget = NULL;
ID3D11Resource*            pRenderTargetResource = NULL;
ID3D11RenderTargetView*    pRenderTargetView = NULL;
ID3D11Texture2D*           pDepthStencil = NULL;
ID3D11DepthStencilView*    pDepthStencilView = NULL;
ID3D11RasterizerState*     pDefaultRasterState = NULL;

// Transform matrices
XMMATRIX                matWorld;
XMMATRIX                matView;
XMMATRIX                matProj;

// Dolphon Variables:
// Vertex data structure used to
// create a constant buffer.
struct VS_CONSTANT_BUFFER
{
    XMVECTOR vZero;
    XMVECTOR vConstants;
    XMVECTOR vWeight;

    XMMATRIX matTranspose;
    XMMATRIX matCameraTranspose;
    XMMATRIX matViewTranspose;
    XMMATRIX matProjTranspose;

    XMVECTOR vLight;
    XMVECTOR vLightDolphinSpace;
    XMVECTOR vDiffuse;
    XMVECTOR vAmbient;
    XMVECTOR vFog;
    XMVECTOR vCaustics;
};

// Pixel data structure used to
// create a constant buffer.
struct PS_CONSTANT_BUFFER
{
    FLOAT fAmbient[4];
    FLOAT fFogColor[4];
};

ID3D11Buffer*       pVSConstantBuffer = NULL;
ID3D11Buffer*       pPSConstantBuffer = NULL;

// Dolphin RenderState
ID3D11BlendState*        pBlendStateNoBlend = NULL;
ID3D11RasterizerState*   pRasterizerStateNoCull = NULL;
ID3D11DepthStencilState* pLessEqualDepth = NULL;
ID3D11SamplerState*      pSamplerWrap = NULL;
ID3D11SamplerState*      pSamplerMirror = NULL;

// Dolphin Data
ID3D11ShaderResourceView*  pDolphinTextureView = NULL;
ID3D11VertexShader*        pDolphinVertexShader = NULL;
ID3D11InputLayout*         pDolphinVertexLayout = NULL;

CDXUTSDKMesh        DolphinMesh1;
CDXUTSDKMesh        DolphinMesh2;
CDXUTSDKMesh        DolphinMesh3;

ID3D11Buffer*       pDolphinVB1 = NULL;
ID3D11Buffer*       pDolphinVB2 = NULL;
ID3D11Buffer*       pDolphinVB3 = NULL;
ID3D11Buffer*       pDolphinIB = NULL;

#pragma pack(1)
typedef struct
{
    XMFLOAT3 pos;
    XMFLOAT3 normal;
    XMFLOAT2 tex;
} DOLPHIN_VERTEX;
#pragma pack()
DOLPHIN_VERTEX* pDolphinTweenedNormalBuffer = NULL;
ID3D11Buffer* pDolphinTweenedNormalVB = NULL;

D3D11_PRIMITIVE_TOPOLOGY    DolphinPrimType;
DWORD                       dwDolphinVertexStride;
DWORD                       dwNumDolphinIndices;
DXGI_FORMAT                 DolphinVertexFormat;

// Seafloor Data
ID3D11ShaderResourceView*   pSeaFloorTextureView = NULL;
ID3D11VertexShader*         pSeaFloorVertexShader = NULL;
ID3D11InputLayout*          pSeaFloorVertexLayout = NULL;

CDXUTSDKMesh                SeaFloorMesh;

ID3D11Buffer*               pSeaFloorVB = NULL;
ID3D11Buffer*               pSeaFloorIB = NULL;

D3D11_PRIMITIVE_TOPOLOGY    SeaFloorPrimType;
DWORD                       dwSeaFloorVertexStride;
DWORD                       dwNumSeaFloorIndices;
DXGI_FORMAT                 SeaFloorVertexFormat;

// Common pixel shader
ID3D11PixelShader*          pPixelShader = NULL;

// Water caustics
ID3D11ShaderResourceView *  pCausticTextureViews[32] = { NULL };
ID3D11ShaderResourceView *  pCurrentCausticTextureView = NULL;

IDXGIDevice2*   pDxgiDev2 = NULL;
HANDLE          hQueueEvent = NULL;

LoadResourceFunc MyLoadResource;

static UINT GetDXGIFormatTexelSize(DXGI_FORMAT Format)
{
    switch (Format)
    {
    case DXGI_FORMAT_UNKNOWN:
        return 0;
    case DXGI_FORMAT_R32G32B32A32_TYPELESS:
        return sizeof(INT) * 4;
    case DXGI_FORMAT_R32G32B32A32_FLOAT:
        return sizeof(FLOAT) * 4;
    case DXGI_FORMAT_R32G32B32A32_UINT:
        return sizeof(UINT) * 4;
    case DXGI_FORMAT_R32G32B32A32_SINT:
        return sizeof(INT) * 4;
    case DXGI_FORMAT_R32G32B32_TYPELESS:
        return sizeof(INT) * 3;
    case DXGI_FORMAT_R32G32B32_FLOAT:
        return sizeof(FLOAT) * 3;
    case DXGI_FORMAT_R32G32B32_UINT:
        return sizeof(UINT) * 3;
    case DXGI_FORMAT_R32G32B32_SINT:
        return sizeof(INT) * 3;
    case DXGI_FORMAT_R16G16B16A16_TYPELESS:
        return sizeof(INT) * 2;
    case DXGI_FORMAT_R16G16B16A16_FLOAT:
        return sizeof(FLOAT) * 2;
    case DXGI_FORMAT_R16G16B16A16_UNORM:
        return sizeof(UINT) * 2;
    case DXGI_FORMAT_R16G16B16A16_UINT:
        return sizeof(UINT) * 2;
    case DXGI_FORMAT_R16G16B16A16_SNORM:
        return sizeof(INT) * 2;
    case DXGI_FORMAT_R16G16B16A16_SINT:
        return sizeof(INT) * 2;
    case DXGI_FORMAT_R32G32_TYPELESS:
        return sizeof(INT) * 2;
    case DXGI_FORMAT_R32G32_FLOAT:
        return sizeof(FLOAT) * 2;
    case DXGI_FORMAT_R32G32_UINT:
        return sizeof(UINT) * 2;
    case DXGI_FORMAT_R32G32_SINT:
        return sizeof(INT) * 2;
    case DXGI_FORMAT_R32G8X24_TYPELESS:
        return sizeof(INT) * 2;
    case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
        return sizeof(FLOAT) * 2;
    case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
        return sizeof(INT) * 2;
    case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
        return sizeof(UINT) * 2;
    case DXGI_FORMAT_R10G10B10A2_TYPELESS:
        return sizeof(INT);
    case DXGI_FORMAT_R10G10B10A2_UNORM:
        return sizeof(UINT);
    case DXGI_FORMAT_R10G10B10A2_UINT:
        return sizeof(UINT);
    case DXGI_FORMAT_R11G11B10_FLOAT:
        return sizeof(FLOAT);
    case DXGI_FORMAT_R8G8B8A8_TYPELESS:
        return sizeof(INT);
    case DXGI_FORMAT_R8G8B8A8_UNORM:
        return sizeof(INT);
    case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
        return sizeof(UINT);
    case DXGI_FORMAT_R8G8B8A8_UINT:
        return sizeof(UINT);
    case DXGI_FORMAT_R8G8B8A8_SNORM:
        return sizeof(INT);
    case DXGI_FORMAT_R8G8B8A8_SINT:
        return sizeof(UINT);
    case DXGI_FORMAT_R16G16_TYPELESS:
        return sizeof(UINT);
    case DXGI_FORMAT_R16G16_FLOAT:
        return sizeof(FLOAT);
    case DXGI_FORMAT_R16G16_UNORM:
        return sizeof(UINT);
    case DXGI_FORMAT_R16G16_UINT:
        return sizeof(UINT);
    case DXGI_FORMAT_R16G16_SNORM:
        return sizeof(INT);
    case DXGI_FORMAT_R16G16_SINT:
        return sizeof(INT);
    case DXGI_FORMAT_R32_TYPELESS:
        return sizeof(INT);
    case DXGI_FORMAT_D32_FLOAT:
        return sizeof(FLOAT);
    case DXGI_FORMAT_R32_FLOAT:
        return sizeof(FLOAT);
    case DXGI_FORMAT_R32_UINT:
        return sizeof(UINT);
    case DXGI_FORMAT_R32_SINT:
        return sizeof(INT);
    case DXGI_FORMAT_R24G8_TYPELESS:
        return sizeof(INT);
    case DXGI_FORMAT_D24_UNORM_S8_UINT:
        return sizeof(UINT);
    case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
        return sizeof(INT);
    case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
        return sizeof(UINT);
    case DXGI_FORMAT_R8G8_TYPELESS:
        return sizeof(WORD);
    case DXGI_FORMAT_R8G8_UNORM:
        return sizeof(WORD);
    case DXGI_FORMAT_R8G8_UINT:
        return sizeof(WORD);
    case DXGI_FORMAT_R8G8_SNORM:
        return sizeof(WORD);
    case DXGI_FORMAT_R8G8_SINT:
        return sizeof(WORD);
    case DXGI_FORMAT_R16_TYPELESS:
        return sizeof(WORD);
    case DXGI_FORMAT_R16_FLOAT:
        return sizeof(FLOAT) / 2;
    case DXGI_FORMAT_D16_UNORM:
        return sizeof(WORD);
    case DXGI_FORMAT_R16_UNORM:
        return sizeof(WORD);
    case DXGI_FORMAT_R16_UINT:
        return sizeof(WORD);
    case DXGI_FORMAT_R16_SNORM:
        return sizeof(WORD);
    case DXGI_FORMAT_R16_SINT:
        return sizeof(WORD);
    case DXGI_FORMAT_R8_TYPELESS:
        return sizeof(BYTE);
    case DXGI_FORMAT_R8_UNORM:
        return sizeof(BYTE);
    case DXGI_FORMAT_R8_UINT:
        return sizeof(BYTE);
    case DXGI_FORMAT_R8_SNORM:
        return sizeof(CHAR);
    case DXGI_FORMAT_R8_SINT:
        return sizeof(CHAR);
    case DXGI_FORMAT_A8_UNORM:
        return sizeof(BYTE);
    case DXGI_FORMAT_R1_UNORM:
        return 1;
    case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
        return sizeof(FLOAT);
    case DXGI_FORMAT_R8G8_B8G8_UNORM:
        return sizeof(UINT);
    case DXGI_FORMAT_G8R8_G8B8_UNORM:
        return sizeof(UINT);
        // TODO: Figure out what to do for compressed fornmats.
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
    case DXGI_FORMAT_B5G6R5_UNORM:
    case DXGI_FORMAT_B5G5R5A1_UNORM:
    case DXGI_FORMAT_B8G8R8A8_UNORM:
    case DXGI_FORMAT_B8G8R8X8_UNORM:
    default:
        ErrorLine = __LINE__;
        __debugbreak();
        return 0;
    }
}

static HRESULT MyCreateShaderResourceViewFromBuffer(PVOID pBuffer, UINT Width, UINT Height, DXGI_FORMAT Format, ID3D11ShaderResourceView **ppSRView, ID3D11Device* pDevice)
{
    HRESULT                         hr = S_OK;
    ID3D11Texture2D *               pTexture = NULL;
    ID3D11ShaderResourceView *      pSRView = NULL;
    D3D11_TEXTURE2D_DESC            desc = { 0 };
    D3D11_SUBRESOURCE_DATA          PixelData[1] = { 0 };
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    ZeroMemory(&srvDesc, sizeof(srvDesc));

    desc.Width = Width;
    desc.Height = Height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = Format;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;

    PixelData[0].pSysMem = pBuffer;
    PixelData[0].SysMemPitch = Width * GetDXGIFormatTexelSize(Format);
    PixelData[0].SysMemSlicePitch = 0;

    CHR(pDevice->CreateTexture2D(&desc, PixelData, &pTexture));

    srvDesc.Format = desc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = desc.MipLevels;
    // Must be between 0 and MipLevels - 1.
    srvDesc.Texture2D.MostDetailedMip = 0;
    CHR(pDevice->CreateShaderResourceView(pTexture, &srvDesc, &pSRView));

    SAFE_RELEASE(pTexture);
    *ppSRView = pSRView;

EXIT_RETURN:

    return hr;
}

void UninitD3D()
{
    SAFE_RELEASE(pd3dDevice);
    SAFE_RELEASE(pd3dContext);

    SAFE_RELEASE(pStaging);
    SAFE_RELEASE(pStagingResource);
    SAFE_RELEASE(pRenderTarget);
    SAFE_RELEASE(pRenderTargetResource);
    SAFE_RELEASE(pRenderTargetView);

    SAFE_RELEASE(pDepthStencil);
    SAFE_RELEASE(pDepthStencilView);

    SAFE_RELEASE(pDefaultRasterState);
}

bool InitD3D(bool useRosDriver, unsigned int rtWidth, unsigned int rtHeight)
{
    BOOL bRet = FALSE;
    HRESULT hr;

    IDXGIFactory*   pFactory = NULL;
    IDXGIAdapter*   pAdapter = NULL;
    IDXGIOutput*    pOutput = NULL;

    DXGI_FORMAT selectedFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    DXGI_MODE_DESC ModeToMatch = { 0 };
    DXGI_MODE_DESC ModeDesc = { 0 };

    CHR(CreateDXGIFactory1(__uuidof(IDXGIFactory), (void**)&pFactory));

    for (UINT i = 0; ; i++)
    {
        DXGI_ADAPTER_DESC adapterDesc = { 0 };
        CHR(pFactory->EnumAdapters(i, &pAdapter));
        if (!useRosDriver) break;
        pAdapter->GetDesc(&adapterDesc);
        if (_tcsicmp(adapterDesc.Description, TEXT("Render Only Sample Driver")) == 0)
        {
            break;
        }
        SAFE_RELEASE(pAdapter);
    }

    {
        // Create Direct3D
        D3D_FEATURE_LEVEL  FeatureLevelsRequested[] = { D3D_FEATURE_LEVEL_11_0 };
        D3D_FEATURE_LEVEL  FeatureLevelsSupported = D3D_FEATURE_LEVEL_11_0;

        if (useRosDriver)
        {
            FeatureLevelsRequested[0] = D3D_FEATURE_LEVEL_9_1;
            FeatureLevelsSupported = D3D_FEATURE_LEVEL_9_1;
        }

        CHR(D3D11CreateDevice(pAdapter,
            D3D_DRIVER_TYPE_UNKNOWN,
            NULL,
            D3D11_CREATE_DEVICE_SINGLETHREADED /*| D3D11_CREATE_DEVICE_DEBUG*/,
            FeatureLevelsRequested,
            RTL_NUMBER_OF(FeatureLevelsRequested),
            D3D11_SDK_VERSION,
            &pd3dDevice,
            &FeatureLevelsSupported,
            &pd3dContext));
    }

    if (useRosDriver)
    {
        ModeDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        ModeDesc.Width = rtWidth;
        ModeDesc.Height = rtHeight;
    }
    else
    {
        //
        // Get the current mode information from the output (i.e. as it is
        // currently being used for the desktop).
        //
        CHR(pAdapter->EnumOutputs(0, &pOutput));
        ZeroMemory(&ModeToMatch, sizeof(ModeToMatch));
        CHR(pOutput->FindClosestMatchingMode(&ModeToMatch,
            &ModeDesc,
            pd3dDevice));
    }

    {
        D3D11_TEXTURE2D_DESC desc;

        desc.ArraySize = 1;
        desc.BindFlags = D3D11_BIND_RENDER_TARGET;
        desc.CPUAccessFlags = 0;
        desc.Format = ModeDesc.Format;
        desc.Width = ModeDesc.Width;
        desc.Height = ModeDesc.Height;
        desc.MipLevels = 1;
        desc.MiscFlags = 0;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Usage = D3D11_USAGE_DEFAULT;

        CHR(pd3dDevice->CreateTexture2D(&desc, NULL, &pRenderTarget));
        pRenderTarget->QueryInterface<ID3D11Resource>(&pRenderTargetResource);
        CHR(pd3dDevice->CreateRenderTargetView(pRenderTargetResource, NULL, &pRenderTargetView));

        desc.BindFlags = 0;
        desc.Usage = D3D11_USAGE_STAGING;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
        CHR(pd3dDevice->CreateTexture2D(&desc, NULL, &pStaging));
        pStaging->QueryInterface<ID3D11Resource>(&pStagingResource);
    }

    {
        // Create depth stencil texture
        D3D11_TEXTURE2D_DESC descDepth;
        D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;

        ZeroMemory(&descDepth, sizeof(descDepth));
        descDepth.Width = ModeDesc.Width;
        descDepth.Height = ModeDesc.Height;
        descDepth.MipLevels = 1;
        descDepth.ArraySize = 1;
        descDepth.Format = DXGI_FORMAT_D16_UNORM;
        descDepth.SampleDesc.Count = 1;
        descDepth.SampleDesc.Quality = 0;
        descDepth.Usage = D3D11_USAGE_DEFAULT;
        descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        descDepth.CPUAccessFlags = 0;
        descDepth.MiscFlags = 0;

        CHR(pd3dDevice->CreateTexture2D(&descDepth, NULL, &pDepthStencil));

        // Create the depth stencil view
        ZeroMemory(&descDSV, sizeof(descDSV));
        descDSV.Format = descDepth.Format;
        if (descDepth.SampleDesc.Count > 1)
        {
            descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
            descDSV.Texture2D.MipSlice = 0;
        }
        else
        {
            descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
        }

        CHR(pd3dDevice->CreateDepthStencilView(pDepthStencil, &descDSV, &pDepthStencilView));
    }

    {
        // Create a default rasterizer state
        D3D11_RASTERIZER_DESC RSDesc;
        ZeroMemory(&RSDesc, sizeof(RSDesc));
        RSDesc.FillMode = D3D11_FILL_SOLID;
        RSDesc.CullMode = D3D11_CULL_BACK;
        RSDesc.FrontCounterClockwise = FALSE;
        RSDesc.DepthBias = 0;
        RSDesc.DepthBiasClamp = 0;
        RSDesc.DepthClipEnable = TRUE;
        RSDesc.SlopeScaledDepthBias = 0.0f;
        RSDesc.ScissorEnable = FALSE;
        RSDesc.MultisampleEnable = FALSE;
        RSDesc.AntialiasedLineEnable = FALSE;
        CHR(pd3dDevice->CreateRasterizerState(&RSDesc, &pDefaultRasterState));

        pd3dContext->RSSetState(pDefaultRasterState);
    }

    {
        // Setup the viewport
        D3D11_VIEWPORT vp;
        ZeroMemory(&vp, sizeof(vp));
        vp.Width = (FLOAT)ModeDesc.Width;
        vp.Height = (FLOAT)ModeDesc.Height;
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        vp.TopLeftX = 0;
        vp.TopLeftY = 0;
        pd3dContext->RSSetViewports(1, &vp);
    }

    // Set RenderTarget.
    pd3dContext->OMSetRenderTargets(1, &pRenderTargetView, pDepthStencilView);

    {
        // Determine the aspect ratio
        FLOAT fAspectRatio = (FLOAT)ModeDesc.Width / (FLOAT)ModeDesc.Height;
        if (ModeDesc.Width >= ModeDesc.Height)
        {
            // Set the transform matrices
            XMVECTOR vEyePt = XMVectorSet(0.0f, 0.0f, -5.0f, 0.0f);
            XMVECTOR vLookatPt = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
            XMVECTOR vUpVec = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
            matWorld = XMMatrixIdentity();
            matView = XMMatrixLookAtLH(vEyePt, vLookatPt, vUpVec);
            matProj = XMMatrixPerspectiveFovLH(XM_PI / 3, fAspectRatio, 1.0f, 10000.0f);
        }
        else
        {
            // Set the transform matrices
            XMVECTOR vEyePt = XMVectorSet(0.0f, 0.0f, -5.0f, 0.0f);
            XMVECTOR vLookatPt = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
            XMVECTOR vUpVec = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
            matWorld = XMMatrixIdentity();
            matView = XMMatrixLookAtLH(vEyePt, vLookatPt, vUpVec);
            matProj = XMMatrixPerspectiveFovLH(XM_PI / 2, fAspectRatio, 1.0f, 10000.0f);
        }
    }

    bRet = TRUE;

EXIT_RETURN:

    SAFE_RELEASE(pOutput);
    SAFE_RELEASE(pAdapter);
    SAFE_RELEASE(pFactory);

    if (!bRet)
    {
        UninitD3D();
    }

    return bRet != 0;
}

void UninitDolphin()
{
    for (UINT t = 0; t < 32; t++)
    {
        SAFE_RELEASE(pCausticTextureViews[t]);
    }

    SAFE_RELEASE(pBlendStateNoBlend);
    SAFE_RELEASE(pRasterizerStateNoCull);
    SAFE_RELEASE(pLessEqualDepth);
    SAFE_RELEASE(pSamplerWrap);
    SAFE_RELEASE(pSamplerMirror);

    SAFE_RELEASE(pPixelShader);

    SAFE_RELEASE(pVSConstantBuffer);
    SAFE_RELEASE(pPSConstantBuffer);

    SAFE_RELEASE(pDolphinVertexShader);
    SAFE_RELEASE(pDolphinVB1);
    SAFE_RELEASE(pDolphinVB2);
    SAFE_RELEASE(pDolphinVB3);
    SAFE_RELEASE(pDolphinTweenedNormalVB);
    SAFE_RELEASE(pDolphinIB);
    SAFE_RELEASE(pDolphinTextureView);
    SAFE_RELEASE(pDolphinVertexLayout);

    SAFE_RELEASE(pSeaFloorVertexShader);
    SAFE_RELEASE(pSeaFloorVB);
    SAFE_RELEASE(pSeaFloorIB);
    SAFE_RELEASE(pSeaFloorTextureView);
    SAFE_RELEASE(pSeaFloorVertexLayout);
}


bool InitDolphin(bool useTweenedNormal, LoadResourceFunc loadResourceFunc)
{
    HRESULT hr;
    BOOL bRet = FALSE;

    MyLoadResource = loadResourceFunc;

    // Create our vertex input layout
    const D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "POSITION",  1, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",    1, DXGI_FORMAT_R32G32B32_FLOAT, 1, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD",  1, DXGI_FORMAT_R32G32_FLOAT,    1, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "POSITION",  2, DXGI_FORMAT_R32G32B32_FLOAT, 2, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",    2, DXGI_FORMAT_R32G32B32_FLOAT, 2, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD",  2, DXGI_FORMAT_R32G32_FLOAT,    2, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        // used when using tweened normal
        { "NORMAL",    4, DXGI_FORMAT_R32G32B32_FLOAT, 3,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    UINT layoutLength = ARRAYSIZE(layout);

    if (!useTweenedNormal) layoutLength--;

    // Create constant buffers
    {
        D3D11_BUFFER_DESC cbDesc;
        ZeroMemory(&cbDesc, sizeof(cbDesc));
        cbDesc.ByteWidth = sizeof(VS_CONSTANT_BUFFER);
        cbDesc.Usage = D3D11_USAGE_DYNAMIC;
        cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        cbDesc.MiscFlags = 0;
        CHR(pd3dDevice->CreateBuffer(&cbDesc, NULL, &pVSConstantBuffer));

        cbDesc.ByteWidth = sizeof(PS_CONSTANT_BUFFER);
        CHR(pd3dDevice->CreateBuffer(&cbDesc, NULL, &pPSConstantBuffer));
    }

    // Initialize Dolphin Data.
    {
        {
            PBYTE DolphinData;
            ULONG DolphinHeight, DolphinWidth;
            DWORD dwDolphinBitmapSize = 0;
            PBYTE pDolphinBitmap = (PBYTE)MyLoadResource(IDD_DOLPHIN_BMP, &dwDolphinBitmapSize);
            if (pDolphinBitmap == NULL)
            {
                ErrorLine = __LINE__;
                __debugbreak();
                goto EXIT_RETURN;
            }
            CHR(LoadBMP(pDolphinBitmap, &DolphinHeight, &DolphinWidth, &DolphinData));
            if (FAILED(MyCreateShaderResourceViewFromBuffer((PVOID)DolphinData, DolphinWidth, DolphinHeight, DXGI_FORMAT_R8G8B8A8_UNORM, &pDolphinTextureView, pd3dDevice)))
            {
                free(DolphinData);
                ErrorLine = __LINE__;
                __debugbreak();
                goto EXIT_RETURN;
            }
            free(DolphinData);
        }

        {
            DWORD dwDolphinMesh1Size = 0;
            PBYTE pDolphinMesh1 = (PBYTE)MyLoadResource(IDD_DOLPHIN_MESH1, &dwDolphinMesh1Size);
            DWORD dwDolphinMesh2Size = 0;
            PBYTE pDolphinMesh2 = (PBYTE)MyLoadResource(IDD_DOLPHIN_MESH2, &dwDolphinMesh2Size);
            DWORD dwDolphinMesh3Size = 0;
            PBYTE pDolphinMesh3 = (PBYTE)MyLoadResource(IDD_DOLPHIN_MESH3, &dwDolphinMesh3Size);
            if (pDolphinMesh1 == NULL || pDolphinMesh2 == NULL || pDolphinMesh3 == NULL)
            {
                ErrorLine = __LINE__;
                __debugbreak();
                goto EXIT_RETURN;
            }
            CHR(DolphinMesh1.Create(pd3dDevice, pDolphinMesh1, dwDolphinMesh1Size));
            CHR(DolphinMesh2.Create(pd3dDevice, pDolphinMesh2, dwDolphinMesh2Size));
            CHR(DolphinMesh3.Create(pd3dDevice, pDolphinMesh3, dwDolphinMesh3Size));
        }

        pDolphinVB1 = DolphinMesh1.GetVB11(0, 0);
        pDolphinVB2 = DolphinMesh2.GetVB11(0, 0);
        pDolphinVB3 = DolphinMesh3.GetVB11(0, 0);
        pDolphinIB = DolphinMesh1.GetIB11(0);
        {
            SDKMESH_SUBSET* pSubset = DolphinMesh1.GetSubset(0, 0);
            if (!pSubset)
            {
                ErrorLine = __LINE__;
                __debugbreak();
                goto EXIT_RETURN;
            }
            DolphinPrimType = CDXUTSDKMesh::GetPrimitiveType11((SDKMESH_PRIMITIVE_TYPE)pSubset->PrimitiveType);
            dwNumDolphinIndices = (DWORD)DolphinMesh1.GetNumIndices(0);
            dwDolphinVertexStride = (DWORD)DolphinMesh1.GetVertexStride(0, 0);
            DolphinVertexFormat = DolphinMesh1.GetIBFormat11(0);
        }

        if (useTweenedNormal)
        {
            assert(sizeof(DOLPHIN_VERTEX) == DolphinMesh1.GetVertexStride(0, 0));
            assert(DolphinMesh1.GetNumVertices(0, 0) == DolphinMesh2.GetNumVertices(0, 0));
            assert(DolphinMesh2.GetNumVertices(0, 0) == DolphinMesh3.GetNumVertices(0, 0));
            assert(DolphinMesh1.GetVertexStride(0, 0) == DolphinMesh2.GetVertexStride(0, 0));
            assert(DolphinMesh2.GetVertexStride(0, 0) == DolphinMesh3.GetVertexStride(0, 0));

            D3D11_BUFFER_DESC bufferDesc = {};
            bufferDesc.ByteWidth = (UINT)DolphinMesh1.GetNumVertices(0, 0) * sizeof(XMFLOAT3);
            bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
            bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            bufferDesc.MiscFlags = 0;

            CHR(pd3dDevice->CreateBuffer(&bufferDesc, NULL, &pDolphinTweenedNormalVB));
        }

        {
            DWORD dwDolphinVSSize = 0;
            PBYTE pDolphinVSData = (PBYTE)MyLoadResource(IDD_DOLPHIN_VS, &dwDolphinVSSize);
            if (pDolphinVSData == NULL)
            {
                ErrorLine = __LINE__;
                __debugbreak();
                goto EXIT_RETURN;
            }
            CHR(pd3dDevice->CreateVertexShader((DWORD*)pDolphinVSData, dwDolphinVSSize, NULL, &pDolphinVertexShader));
            CHR(pd3dDevice->CreateInputLayout(layout, layoutLength, pDolphinVSData, dwDolphinVSSize, &pDolphinVertexLayout));
        }
    }

    // Initialize Seafloor data.
    {
        {
            PBYTE SeaFloorData;
            ULONG SeaFloorHeight, SeaFloorWidth;
            DWORD dwSeaFloorBitmapSize = 0;
            PBYTE pSeaFloorBitmap = (PBYTE)MyLoadResource(IDD_SEAFLOOR_BMP, &dwSeaFloorBitmapSize);
            if (pSeaFloorBitmap == NULL)
            {
                ErrorLine = __LINE__;
                __debugbreak();
                goto EXIT_RETURN;
            }
            CHR(LoadBMP(pSeaFloorBitmap, &SeaFloorHeight, &SeaFloorWidth, &SeaFloorData));
            if (FAILED(MyCreateShaderResourceViewFromBuffer((PVOID)SeaFloorData, SeaFloorWidth, SeaFloorHeight, DXGI_FORMAT_R8G8B8A8_UNORM, &pSeaFloorTextureView, pd3dDevice)))
            {
                free(SeaFloorData);
                ErrorLine = __LINE__;
                __debugbreak();
                goto EXIT_RETURN;
            }
            free(SeaFloorData);
        }

        {
            DWORD dwSeaFloorMeshSize = 0;
            PBYTE pSeaFloorMesh = (PBYTE)MyLoadResource(IDD_SEAFLOOR_MESH, &dwSeaFloorMeshSize);
            if (pSeaFloorMesh == NULL)
            {
                ErrorLine = __LINE__;
                __debugbreak();
                goto EXIT_RETURN;
            }
            CHR(SeaFloorMesh.Create(pd3dDevice, pSeaFloorMesh, dwSeaFloorMeshSize));
        }

        pSeaFloorVB = SeaFloorMesh.GetVB11(0, 0);
        pSeaFloorIB = SeaFloorMesh.GetIB11(0);
        {
            SDKMESH_SUBSET* pSubset = SeaFloorMesh.GetSubset(0, 0);
            if (!pSubset)
            {
                ErrorLine = __LINE__;
                __debugbreak();
                goto EXIT_RETURN;
            }

            SeaFloorPrimType = CDXUTSDKMesh::GetPrimitiveType11((SDKMESH_PRIMITIVE_TYPE)pSubset->PrimitiveType);
            dwNumSeaFloorIndices = (DWORD)SeaFloorMesh.GetNumIndices(0);
            dwSeaFloorVertexStride = (DWORD)SeaFloorMesh.GetVertexStride(0, 0);
            SeaFloorVertexFormat = SeaFloorMesh.GetIBFormat11(0);
        }

        // Add some bumpiness to the seafloor
        {
            D3D11_MAPPED_SUBRESOURCE mappedResource = { 0 };
            CHR(pd3dContext->Map(pSeaFloorVB, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &mappedResource));

            srand(5);
            BYTE* pDst = (BYTE *)mappedResource.pData;
            for (DWORD i = 0; i < SeaFloorMesh.GetNumVertices(0, 0); i++)
            {
                ((XMFLOAT3*)pDst)->y += (rand() / (FLOAT)RAND_MAX);
                ((XMFLOAT3*)pDst)->y += (rand() / (FLOAT)RAND_MAX);
                ((XMFLOAT3*)pDst)->y += (rand() / (FLOAT)RAND_MAX);
                pDst += sizeof(FLOAT) * 8;
            }
            pd3dContext->Unmap(pSeaFloorVB, 0);
        }

        {
            DWORD dwSeaFloorVSSize = 0;
            PBYTE pSeaFloorVSData = (PBYTE)MyLoadResource(IDD_SEAFLOOR_VS, &dwSeaFloorVSSize);
            if (pSeaFloorVSData == NULL)
            {
                ErrorLine = __LINE__;
                __debugbreak();
                goto EXIT_RETURN;
            }
            CHR(pd3dDevice->CreateVertexShader((DWORD*)pSeaFloorVSData, dwSeaFloorVSSize, NULL, &pSeaFloorVertexShader));
            CHR(pd3dDevice->CreateInputLayout(layout, 3, pSeaFloorVSData, dwSeaFloorVSSize, &pSeaFloorVertexLayout));
        }
    }

    // Load caustic data.
    for (DWORD t = 0; t < 32; t++)
    {
        PBYTE CaustData;
        ULONG CaustHeight, CaustWidth;
        DWORD dwCaustBitmapSize = 0;
        PBYTE pCaustBitmap = (PBYTE)MyLoadResource(IDD_CAUST00_TGA + t, &dwCaustBitmapSize);
        if (pCaustBitmap == NULL)
        {
            ErrorLine = __LINE__;
            __debugbreak();
            goto EXIT_RETURN;
        }
        CHR(LoadTGA(pCaustBitmap, &CaustHeight, &CaustWidth, &CaustData));
        if (FAILED(MyCreateShaderResourceViewFromBuffer((PVOID)CaustData, CaustWidth, CaustHeight, DXGI_FORMAT_R8G8B8A8_UNORM, &pCausticTextureViews[t], pd3dDevice)))
        {
            free(CaustData);
            ErrorLine = __LINE__;
            __debugbreak();
            goto EXIT_RETURN;
        }
        free(CaustData);
    }

    // Create Commom pixel shader
    {
        DWORD dwPSSize = 0;
        PBYTE pPSData = (PBYTE)MyLoadResource(IDD_SHADE_PS, &dwPSSize);
        if (pPSData == NULL)
        {
            ErrorLine = __LINE__;
            __debugbreak();
            goto EXIT_RETURN;
        }
        CHR(pd3dDevice->CreatePixelShader((DWORD*)pPSData, dwPSSize, NULL, &pPixelShader));
    }

    // Create a blend state to disable alpha blending
    {
        D3D11_BLEND_DESC blendState;
        ZeroMemory(&blendState, sizeof(blendState));
        blendState.RenderTarget[0].BlendEnable = FALSE;
        blendState.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        CHR(pd3dDevice->CreateBlendState(&blendState, &pBlendStateNoBlend));
    }

    // Create a rasterizer state to disable culling
    {
        D3D11_RASTERIZER_DESC RSDesc;
        ZeroMemory(&RSDesc, sizeof(RSDesc));
        RSDesc.FillMode = D3D11_FILL_SOLID;
        RSDesc.CullMode = D3D11_CULL_BACK;
        RSDesc.FrontCounterClockwise = FALSE;
        RSDesc.DepthBias = 0;
        RSDesc.DepthBiasClamp = 0;
        RSDesc.DepthClipEnable = TRUE;
        RSDesc.SlopeScaledDepthBias = 0.0f;
        RSDesc.ScissorEnable = FALSE;
        RSDesc.MultisampleEnable = FALSE;
        RSDesc.AntialiasedLineEnable = FALSE;
        CHR(pd3dDevice->CreateRasterizerState(&RSDesc, &pRasterizerStateNoCull));
    }

    // Create a depth stencil state to enable less-equal depth testing
    {
        D3D11_DEPTH_STENCIL_DESC DSDesc;
        ZeroMemory(&DSDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
        DSDesc.DepthEnable = TRUE;
        DSDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
        DSDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        CHR(pd3dDevice->CreateDepthStencilState(&DSDesc, &pLessEqualDepth));
    }

    // Create Mirror Sampler state
    {
        D3D11_SAMPLER_DESC sampMirror;
        ZeroMemory(&sampMirror, sizeof(D3D11_SAMPLER_DESC));
        sampMirror.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR;
        sampMirror.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
        sampMirror.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR;
        sampMirror.MaxLOD = D3D11_FLOAT32_MAX;
        sampMirror.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
        CHR(pd3dDevice->CreateSamplerState(&sampMirror, &pSamplerMirror));
    }

    // Create Wrap sampler state
    {
        D3D11_SAMPLER_DESC sampWrap;
        ZeroMemory(&sampWrap, sizeof(D3D11_SAMPLER_DESC));
        sampWrap.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        sampWrap.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        sampWrap.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        sampWrap.MaxLOD = D3D11_FLOAT32_MAX;
        sampWrap.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
        CHR(pd3dDevice->CreateSamplerState(&sampWrap, &pSamplerWrap));
    }

    bRet = TRUE;

EXIT_RETURN:

    if (!bRet)
    {
        UninitDolphin();
    }

    return bRet != 0;
}

void UpdateDolphin(bool useTweenedNormal)
{
    /*
    static float deg = 0;
    fWaterColor[2] = sinf((FLOAT)deg*(XM_PI/180));
    deg+=0.05f;
    */

    // Get the current time
    FLOAT fTime = (FLOAT)AppTimer.GetAppTime();

    // Animation attributes for the dolphin
    FLOAT fKickFreq = 2 * fTime;
    FLOAT fPhase = fTime / 3;
    FLOAT fBlendWeight = sinf(fKickFreq);

    // Move the dolphin in a circle
    XMMATRIX matDolphin, matTrans, matRotate1, matRotate2;
    matDolphin = XMMatrixScaling(0.01f, 0.01f, 0.01f);
    matRotate1 = XMMatrixRotationZ(-cosf(fKickFreq) / 6);
    matDolphin = XMMatrixMultiply(matDolphin, matRotate1);
    matRotate2 = XMMatrixRotationY(fPhase);
    matDolphin = XMMatrixMultiply(matDolphin, matRotate2);
    matTrans = XMMatrixTranslation(-5 * sinf(fPhase), sinf(fKickFreq) / 2, 10 - 10 * cosf(fPhase));
    matDolphin = XMMatrixMultiply(matDolphin, matTrans);

    // Animate the caustic textures
    DWORD tex = ((DWORD)(fTime * 32)) % 32;
    pCurrentCausticTextureView = pCausticTextureViews[tex];

    FLOAT fWeight1;
    FLOAT fWeight2;
    FLOAT fWeight3;

    if (fBlendWeight > 0.0f)
    {
        fWeight1 = fabsf(fBlendWeight);
        fWeight2 = 1.0f - fabsf(fBlendWeight);
        fWeight3 = 0.0f;
    }
    else
    {
        fWeight1 = 0.0f;
        fWeight2 = 1.0f - fabsf(fBlendWeight);
        fWeight3 = fabsf(fBlendWeight);
    }

    D3D11_MAPPED_SUBRESOURCE MappedResource;

    if (useTweenedNormal)
    {
        pd3dContext->Map(pDolphinTweenedNormalVB, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
        {
            XMFLOAT3* pTweenedNormal = (XMFLOAT3*)MappedResource.pData;

            // Gather normal data from DolphinMesh.
            DOLPHIN_VERTEX *pVertices1 = (DOLPHIN_VERTEX *)DolphinMesh1.GetRawVerticesAt(0);
            DOLPHIN_VERTEX *pVertices2 = (DOLPHIN_VERTEX *)DolphinMesh2.GetRawVerticesAt(0);
            DOLPHIN_VERTEX *pVertices3 = (DOLPHIN_VERTEX *)DolphinMesh3.GetRawVerticesAt(0);

            for (UINT n = 0; n < DolphinMesh1.GetNumVertices(0, 0); n++)
            {
                // float3 vModelNormal = vNormal0 * g_vBlendWeights.x + 
                //                       vNormal1 * g_vBlendWeights.y + 
                //                       vNormal2 * g_vBlendWeights.z;

                XMVECTOR vNormal0 = XMLoadFloat3(&pVertices1[n].normal);
                XMVECTOR vNormal1 = XMLoadFloat3(&pVertices2[n].normal);
                XMVECTOR vNormal2 = XMLoadFloat3(&pVertices3[n].normal);

                vNormal0 = XMVectorScale(vNormal0, fWeight1);
                vNormal1 = XMVectorScale(vNormal1, fWeight2);
                vNormal2 = XMVectorScale(vNormal2, fWeight3);

                XMVECTOR vNormal = XMVectorAdd(XMVectorAdd(vNormal0, vNormal1), vNormal2);

                XMStoreFloat3(&pTweenedNormal[n], vNormal);
            }
        }
        pd3dContext->Unmap(pDolphinTweenedNormalVB, 0);
    }

    pd3dContext->Map(pVSConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
    {
        VS_CONSTANT_BUFFER* pVsConstData = (VS_CONSTANT_BUFFER*)MappedResource.pData;

        pVsConstData->vZero = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
        pVsConstData->vConstants = XMVectorSet(1.0f, 0.5f, 0.2f, 0.05f);

        pVsConstData->vWeight = XMVectorSet(fWeight1, fWeight2, fWeight3, 0.0f);

        // Lighting vectors (in world space and in dolphin model space)
        // and other constants
        pVsConstData->vLight = XMVectorSet(0.00f, 1.00f, 0.00f, 0.00f);
        pVsConstData->vLightDolphinSpace = XMVectorSet(0.00f, 1.00f, 0.00f, 0.00f);
        pVsConstData->vDiffuse = XMVectorSet(1.00f, 1.00f, 1.00f, 1.00f);
        pVsConstData->vAmbient = XMVectorSet(fAmbient[0], fAmbient[1], fAmbient[2], fAmbient[3]);
        pVsConstData->vFog = XMVectorSet(0.50f, 50.00f, 1.00f / (50.0f - 1.0f), 0.00f);
        pVsConstData->vCaustics = XMVectorSet(0.05f, 0.05f, sinf(fTime) / 8, cosf(fTime) / 10);

        XMVECTOR vDeterminant;
        XMMATRIX matDolphinInv = XMMatrixInverse(&vDeterminant, matDolphin);
        pVsConstData->vLightDolphinSpace = XMVector4Normalize(XMVector4Transform(pVsConstData->vLight, matDolphinInv));

        // Vertex shader operations use transposed matrices
        XMMATRIX mat, matCamera;
        matCamera = XMMatrixMultiply(matDolphin, matView);
        mat = XMMatrixMultiply(matCamera, matProj);
        pVsConstData->matTranspose = XMMatrixTranspose(mat);
        pVsConstData->matCameraTranspose = XMMatrixTranspose(matCamera);
        pVsConstData->matViewTranspose = XMMatrixTranspose(matView);
        pVsConstData->matProjTranspose = XMMatrixTranspose(matProj);
    }
    pd3dContext->Unmap(pVSConstantBuffer, 0);

    pd3dContext->Map(pPSConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
    {
        PS_CONSTANT_BUFFER* pPsConstData = (PS_CONSTANT_BUFFER*)MappedResource.pData;
        memcpy(pPsConstData->fAmbient, fAmbient, sizeof(fAmbient));
        memcpy(pPsConstData->fFogColor, fWaterColor, sizeof(fWaterColor));
    }
    pd3dContext->Unmap(pPSConstantBuffer, 0);
}

void RenderDolphin(bool useTweenedNormal)
{
    //
    // Clear the back buffer
    //
    pd3dContext->ClearRenderTargetView(pRenderTargetView, fWaterColor);
    pd3dContext->ClearDepthStencilView(pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0, 0);

    // Set state
    pd3dContext->OMSetRenderTargets(1, &pRenderTargetView, pDepthStencilView); // MPO: reset at every frame due to swapeffect
    pd3dContext->OMSetBlendState(pBlendStateNoBlend, 0, 0xffffffff);
    pd3dContext->RSSetState(pRasterizerStateNoCull);
    pd3dContext->OMSetDepthStencilState(pLessEqualDepth, 0);

    ID3D11SamplerState* pSamplers[] = { pSamplerMirror, pSamplerWrap };
    pd3dContext->PSSetSamplers(0, 2, pSamplers);

    pd3dContext->VSSetConstantBuffers(0, 1, &pVSConstantBuffer);
    pd3dContext->PSSetConstantBuffers(0, 1, &pPSConstantBuffer);

    /////////////////////////////////////////////////////
    //
    // Render sea floor
    //
    ////////////////////////////////////////////////////

    pd3dContext->IASetInputLayout(pSeaFloorVertexLayout);
    UINT SeaFloorStrides[1];
    UINT SeaFloorOffsets[1];
    SeaFloorStrides[0] = (UINT)dwSeaFloorVertexStride;
    SeaFloorOffsets[0] = 0;
    pd3dContext->IASetVertexBuffers(0, 1, &pSeaFloorVB, SeaFloorStrides, SeaFloorOffsets);
    pd3dContext->IASetIndexBuffer(pSeaFloorIB, SeaFloorVertexFormat, 0);
    pd3dContext->IASetPrimitiveTopology(SeaFloorPrimType);
    pd3dContext->VSSetShader(pSeaFloorVertexShader, NULL, 0);
    pd3dContext->GSSetShader(NULL, NULL, 0);
    pd3dContext->PSSetShader(pPixelShader, NULL, 0);
    pd3dContext->PSSetShaderResources(0, 1, &pSeaFloorTextureView);
    pd3dContext->PSSetShaderResources(1, 1, &pCurrentCausticTextureView);
    pd3dContext->DrawIndexed((UINT)dwNumSeaFloorIndices, 0, 0);

    ////////////////////////////////////////////////////
    //
    // Render Dolphin
    //
    ////////////////////////////////////////////////////

    pd3dContext->IASetInputLayout(pDolphinVertexLayout);
    UINT DolphinStrides[4];
    UINT DolphinOffsets[4];
    ID3D11Buffer* pDolphinVB[4];
    pDolphinVB[0] = pDolphinVB1;
    pDolphinVB[1] = pDolphinVB2;
    pDolphinVB[2] = pDolphinVB3;
    DolphinStrides[0] = (UINT)dwDolphinVertexStride;
    DolphinStrides[1] = (UINT)dwDolphinVertexStride;
    DolphinStrides[2] = (UINT)dwDolphinVertexStride;
    DolphinOffsets[0] = 0;
    DolphinOffsets[1] = 0;
    DolphinOffsets[2] = 0;

    if (useTweenedNormal)
    {
        pDolphinVB[3] = pDolphinTweenedNormalVB;
        DolphinStrides[3] = sizeof(XMFLOAT3);
        DolphinOffsets[3] = 0;
        pd3dContext->IASetVertexBuffers(0, 4, pDolphinVB, DolphinStrides, DolphinOffsets);
    }
    else
    {
        pd3dContext->IASetVertexBuffers(0, 3, pDolphinVB, DolphinStrides, DolphinOffsets);
    }

    pd3dContext->IASetIndexBuffer(pDolphinIB, DolphinVertexFormat, 0);
    pd3dContext->IASetPrimitiveTopology(DolphinPrimType);
    pd3dContext->VSSetShader(pDolphinVertexShader, NULL, 0);
    pd3dContext->GSSetShader(NULL, NULL, 0);
    pd3dContext->PSSetShader(pPixelShader, NULL, 0);
    pd3dContext->PSSetShaderResources(0, 1, &pDolphinTextureView);
    pd3dContext->PSSetShaderResources(1, 1, &pCurrentCausticTextureView);
    pd3dContext->DrawIndexed((UINT)dwNumDolphinIndices, 0, 0);
}

void SaveDolphin(int iFrame)
{
    char fileName[MAX_PATH];
    sprintf_s(fileName, MAX_PATH, ".\\Dolphin_%d.bmp", iFrame);
    pd3dContext->CopyResource(pStagingResource, pRenderTargetResource);
    SaveBMP(fileName, pd3dDevice, pStaging);
}

void UninitPerf()
{
    SAFE_RELEASE(pDxgiDev2);

    if (hQueueEvent)
    {
        CloseHandle(hQueueEvent);
    }
}

bool InitPerf()
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

void EnqueueRenderEvent()
{
    pDxgiDev2->EnqueueSetEvent(hQueueEvent);
}

unsigned long WaitForRenderEvent()
{
    DWORD dwWaitResult;

    dwWaitResult = WaitForSingleObject(
        hQueueEvent,    // event handle
        INFINITE);      // indefinite wait

    return dwWaitResult;
}

void ResetRenderEvent()
{
    ResetEvent(hQueueEvent);
}

void FlushRender()
{
    pd3dContext->Flush();
}