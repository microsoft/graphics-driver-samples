// DolphinTests.cpp : Defines the entry point for the console application.
//

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
#include "resource.h"
#include "xTimer.h"

#include <math.h>
#include <directxmath.h>

#include <DolphinRender.h>

#define CHR(x) { hr = (x); if (FAILED(hr)) {__debugbreak(); goto EXIT_RETURN; } }

#define SAFE_RELEASE(x) { if (x) { (x)->Release(); (x) = NULL; } }

class DeviceState
{
public:

    DeviceState()
    {
        m_device = NULL;
        m_context = NULL;
        m_adapter = NULL;
    }

    HRESULT Init(bool useRosDriver)
    {
        HRESULT hr;

        IDXGIFactory*   pFactory = NULL;

        CHR(CreateDXGIFactory1(__uuidof(IDXGIFactory), (void**)&pFactory));

        for (UINT i = 0; ; i++)
        {
            DXGI_ADAPTER_DESC adapterDesc = { 0 };
            CHR(pFactory->EnumAdapters(i, &m_adapter));
            if (!useRosDriver) break;
            m_adapter->GetDesc(&adapterDesc);
            if (_tcsicmp(adapterDesc.Description, TEXT("Render Only Sample Driver")) == 0)
            {
                break;
            }
            SAFE_RELEASE(m_adapter);
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

            CHR(D3D11CreateDevice(m_adapter,
                D3D_DRIVER_TYPE_UNKNOWN,
                NULL,
                D3D11_CREATE_DEVICE_SINGLETHREADED /*| D3D11_CREATE_DEVICE_DEBUG*/,
                FeatureLevelsRequested,
                RTL_NUMBER_OF(FeatureLevelsRequested),
                D3D11_SDK_VERSION,
                &m_device,
                &FeatureLevelsSupported,
                &m_context));
        }

    EXIT_RETURN:

        SAFE_RELEASE(pFactory);

        return hr;

    }

    void Uninit()
    {
        SAFE_RELEASE(m_context);
        SAFE_RELEASE(m_device);
        SAFE_RELEASE(m_adapter);
    }

    IDXGIAdapter*              m_adapter;
    ID3D11Device*              m_device;
    ID3D11DeviceContext*       m_context;
};

class TargetState
{
public:

    TargetState()
    {
        m_staging = NULL;
        m_stagingResource = NULL;
        m_renderTarget = NULL;
        m_renderTargetResource = NULL;
        m_renderTargetView = NULL;
        m_depthStencil = NULL;
        m_depthStencilView = NULL;
    }

    HRESULT Init(bool useRosDriver, UINT rtWidth, UINT rtHeight, IDXGIAdapter* pAdapter, ID3D11Device* pDevice)
    {
        BOOL bRet = FALSE;
        HRESULT hr;

        IDXGIOutput*    pOutput = NULL;

        DXGI_FORMAT selectedFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
        DXGI_MODE_DESC ModeToMatch = { 0 };
        DXGI_MODE_DESC ModeDesc = { 0 };

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
                pDevice));
        }

        m_width = ModeDesc.Width;
        m_height = ModeDesc.Height;

        {
            D3D11_TEXTURE2D_DESC desc;

            desc.ArraySize = 1;
            desc.BindFlags = D3D11_BIND_RENDER_TARGET;
            desc.CPUAccessFlags = 0;
            desc.Format = ModeDesc.Format;
            desc.Width = m_width;
            desc.Height = m_height;
            desc.MipLevels = 1;
            desc.MiscFlags = 0;
            desc.SampleDesc.Count = 1;
            desc.SampleDesc.Quality = 0;
            desc.Usage = D3D11_USAGE_DEFAULT;

            CHR(pDevice->CreateTexture2D(&desc, NULL, &m_renderTarget));
            m_renderTarget->QueryInterface<ID3D11Resource>(&m_renderTargetResource);
            CHR(pDevice->CreateRenderTargetView(m_renderTargetResource, NULL, &m_renderTargetView));

            desc.BindFlags = 0;
            desc.Usage = D3D11_USAGE_STAGING;
            desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
            CHR(pDevice->CreateTexture2D(&desc, NULL, &m_staging));
            m_staging->QueryInterface<ID3D11Resource>(&m_stagingResource);
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

            CHR(pDevice->CreateTexture2D(&descDepth, NULL, &m_depthStencil));

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

            CHR(pDevice->CreateDepthStencilView(m_depthStencil, &descDSV, &m_depthStencilView));

        }

    EXIT_RETURN:

        SAFE_RELEASE(pOutput);

        return hr;
    }

    void Uninit()
    {
        SAFE_RELEASE(m_staging);
        SAFE_RELEASE(m_stagingResource);
        SAFE_RELEASE(m_renderTarget);
        SAFE_RELEASE(m_renderTargetResource);
        SAFE_RELEASE(m_renderTargetView);

        SAFE_RELEASE(m_depthStencil);
        SAFE_RELEASE(m_depthStencilView);
    }

    ID3D11Texture2D*           m_staging;
    ID3D11Resource*            m_stagingResource;
    ID3D11Texture2D*           m_renderTarget;
    ID3D11Resource*            m_renderTargetResource;
    ID3D11RenderTargetView*    m_renderTargetView;
    ID3D11Texture2D*           m_depthStencil;
    ID3D11DepthStencilView*    m_depthStencilView;
    UINT                       m_width;
    UINT                       m_height;

};

DeviceState g_deviceState;
TargetState g_targetState;

static void * MyLoadResource(INT Name, DWORD *pdwSize)
{
    HRSRC hRsrc = FindResourceExW(NULL, RT_RCDATA, MAKEINTRESOURCE(Name), 0);
    if (hRsrc == NULL)
    {
        __debugbreak();
        return NULL;
    }
    HGLOBAL hRes = LoadResource(NULL, hRsrc);
    if (hRes == NULL)
    {
        __debugbreak();
        return NULL;
    }
    *pdwSize = SizeofResource(NULL, hRsrc);
    return (LockResource(hRes));
}

void SaveDolphin(int iFrame, ID3D11Device *pDevice, ID3D11DeviceContext* pContext)
{
    ID3D11Resource*            pStagingResource = g_targetState.m_stagingResource;
    ID3D11Resource*            pRenderTargetResource = g_targetState.m_renderTargetResource;
    ID3D11Texture2D*           pStaging = g_targetState.m_staging;

    char fileName[MAX_PATH];
    sprintf_s(fileName, MAX_PATH, ".\\Dolphin_%d.bmp", iFrame);
    pContext->CopyResource(pStagingResource, pRenderTargetResource);
    SaveBMP(fileName, pDevice, pStaging);
}

int main(int argc, char *argv[])
{
    BOOL            bPerfMode = false;
    bool            useRosDriver = false;
    bool            useTweenedNormal = true;

#if defined(_M_ARM) && defined(VC4)
    useRosDriver = true;
#endif

    UINT            rtWidth = 800;
    UINT            rtHeight = 600;
    UINT            frames = 3;

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
        sscanf_s(argv[1], "%d", &rtWidth);
        sscanf_s(argv[2], "%d", &rtHeight);

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

    // TODO: We don't check return result of InitD3D
    g_deviceState.Init(useRosDriver);
    g_targetState.Init(useRosDriver, rtWidth, rtHeight, g_deviceState.m_adapter, g_deviceState.m_device);

    InitTargetSizeDependentDolphinResources(g_targetState.m_width, g_targetState.m_height, g_deviceState.m_device, g_deviceState.m_context,
        g_targetState.m_renderTargetView, g_targetState.m_depthStencilView);

    InitDeviceDependentDolphinResources(useTweenedNormal, MyLoadResource, g_deviceState.m_device, g_deviceState.m_context);

    IDXGIDevice2*   pDxgiDev2 = NULL;
    HANDLE          hQueueEvent = NULL;

    if (bPerfMode)
    {
        g_deviceState.m_device->QueryInterface<IDXGIDevice2>(&pDxgiDev2);

        // Create a manual-reset event object. 
        hQueueEvent = CreateEvent(
            NULL,               // default security attributes
            TRUE,               // manual-reset event
            FALSE,              // initial state is nonsignaled
            FALSE
            );

        QueryPerformanceCounter(&framesStart);
        QueryPerformanceFrequency(&frequenceStart);
    }

    for (UINT i = 0; i < frames; i++)
    {
        // Skip the 1st frame for shader compilation time
        if (bPerfMode && (i == 1))
        {
            QueryPerformanceCounter(&framesStart);
            QueryPerformanceFrequency(&frequenceStart);
        }

        UpdateDolphin(useTweenedNormal, g_deviceState.m_context);
        RenderDolphin(useTweenedNormal, g_deviceState.m_context, g_targetState.m_renderTargetView, g_targetState.m_depthStencilView);

        if (!bPerfMode)
        {
            SaveDolphin(i, g_deviceState.m_device, g_deviceState.m_context);
        }
        else
        {
            if (i == 0)
            {
                //
                // Wait for the 1st frame to finish to account for the GPU paging cost
                //

                pDxgiDev2->EnqueueSetEvent(hQueueEvent);
                
                WaitForSingleObject(
                    hQueueEvent,    // event handle
                    INFINITE);      // indefinite wait

                ResetEvent(hQueueEvent);
            }
            else if (i < (frames - 1))
            {
                g_deviceState.m_context->Flush();
            }
            else
            {
                pDxgiDev2->EnqueueSetEvent(hQueueEvent);
            }
        }
    }

    if (bPerfMode)
    {
        DWORD dwWaitResult = WaitForSingleObject(
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
                rtWidth,
                rtHeight,
                measuredFrames,
                ((framesEnd.QuadPart - framesStart.QuadPart) * 1000) / (measuredFrames*frequenceEnd.QuadPart));
        }

        SAFE_RELEASE(pDxgiDev2);

        if (hQueueEvent)
        {
            CloseHandle(hQueueEvent);
        }
    }

    UninitDeviceDependentDolphinResources();
    UninitTargetSizeDependentDolphinResources();
    g_targetState.Uninit();
    g_deviceState.Uninit();

    return 0;
}
