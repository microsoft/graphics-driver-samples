#include "pch.h"
#include "Sample3DSceneRenderer.h"

#include "..\Common\DirectXHelper.h"
#include "DolphinRender.h"
#include "Resource.h"

#include <assert.h>

using namespace DolphinUniversal;

using namespace DirectX;
using namespace Windows::Foundation;

// Loads vertex and pixel shaders from files and instantiates the cube geometry.
Sample3DSceneRenderer::Sample3DSceneRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
	m_loadingComplete(false),
	m_deviceResources(deviceResources)
{
	CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();
}

// Initializes view parameters when the window size changes.
void Sample3DSceneRenderer::CreateWindowSizeDependentResources()
{
	Size outputSize = m_deviceResources->GetOutputSize();

    auto device = m_deviceResources->GetD3DDevice();
    auto context = m_deviceResources->GetD3DDeviceContext();
    auto renderTargetView = m_deviceResources->GetBackBufferRenderTargetView();
    auto depthStencilView = m_deviceResources->GetDepthStencilView();

    InitTargetSizeDependentDolphinResources((UINT) outputSize.Width, (UINT) outputSize.Height, device, context, renderTargetView, depthStencilView);
}

// Called once per frame, rotates the cube and calculates the model and view matrices.
void Sample3DSceneRenderer::Update(DX::StepTimer const& timer)
{
    // Need to wait for loading to complete before we can update
    if (m_loadingComplete)
    {
        auto context = m_deviceResources->GetD3DDeviceContext();
        UpdateDolphin(true, context);
    }
}

// Renders one frame using the vertex and pixel shaders.
bool Sample3DSceneRenderer::Render()
{
	// Loading is asynchronous. Only draw geometry after it's loaded.
	if (!m_loadingComplete)
	{
        return false;
	}

    auto context = m_deviceResources->GetD3DDeviceContext();
    auto renderTargetView = m_deviceResources->GetBackBufferRenderTargetView();
    auto depthStencilView = m_deviceResources->GetDepthStencilView();

    RenderDolphin(true, context, renderTargetView, depthStencilView);

    return true;
}

const wchar_t *  Sample3DSceneRenderer::ResourceFileName(int id)
{
    switch (id)
    {
    case IDD_DOLPHIN_BMP: return L"Dolphin.bmp";
    case IDD_SEAFLOOR_BMP: return L"seafloor.bmp";

    case IDD_DOLPHIN_VS: return L"DolphinTween.xvu";
    case IDD_SEAFLOOR_VS: return L"SeaFloor.xvu";
    case IDD_SHADE_PS: return L"ShadeCausticsPixel.xpu";

    case IDD_DOLPHIN_MESH1: return L"Dolphin1.sdkmesh";
    case IDD_DOLPHIN_MESH2: return L"Dolphin2.sdkmesh";
    case IDD_DOLPHIN_MESH3: return L"Dolphin3.sdkmesh";
    case IDD_SEAFLOOR_MESH: return L"seafloor.sdkmesh";

    case IDD_CAUST00_TGA: return L"caust00.tga";
    case IDD_CAUST01_TGA: return L"caust01.tga";
    case IDD_CAUST02_TGA: return L"caust02.tga";
    case IDD_CAUST03_TGA: return L"caust03.tga";
    case IDD_CAUST04_TGA: return L"caust04.tga";
    case IDD_CAUST05_TGA: return L"caust05.tga";
    case IDD_CAUST06_TGA: return L"caust06.tga";
    case IDD_CAUST07_TGA: return L"caust07.tga";
    case IDD_CAUST08_TGA: return L"caust08.tga";
    case IDD_CAUST09_TGA: return L"caust09.tga";
    case IDD_CAUST10_TGA: return L"caust10.tga";
    case IDD_CAUST11_TGA: return L"caust11.tga";
    case IDD_CAUST12_TGA: return L"caust12.tga";
    case IDD_CAUST13_TGA: return L"caust13.tga";
    case IDD_CAUST14_TGA: return L"caust14.tga";
    case IDD_CAUST15_TGA: return L"caust15.tga";
    case IDD_CAUST16_TGA: return L"caust16.tga";
    case IDD_CAUST17_TGA: return L"caust17.tga";
    case IDD_CAUST18_TGA: return L"caust18.tga";
    case IDD_CAUST19_TGA: return L"caust19.tga";
    case IDD_CAUST20_TGA: return L"caust20.tga";
    case IDD_CAUST21_TGA: return L"caust21.tga";
    case IDD_CAUST22_TGA: return L"caust22.tga";
    case IDD_CAUST23_TGA: return L"caust23.tga";
    case IDD_CAUST24_TGA: return L"caust24.tga";
    case IDD_CAUST25_TGA: return L"caust25.tga";
    case IDD_CAUST26_TGA: return L"caust26.tga";
    case IDD_CAUST27_TGA: return L"caust27.tga";
    case IDD_CAUST28_TGA: return L"caust28.tga";
    case IDD_CAUST29_TGA: return L"caust29.tga";
    case IDD_CAUST30_TGA: return L"caust30.tga";
    case IDD_CAUST31_TGA: return L"caust31.tga";
    default: assert(0); return NULL;
    }
}


void * Sample3DSceneRenderer::LoadResource(int inId, unsigned long * pdwSize)
{
    void * pData;
    auto task = DX::ReadDataAsync(ResourceFileName(inId));
    task.then([pdwSize, &pData](const std::vector<byte>& fileData)
    {
        size_t size = fileData.size();
        void * data = malloc(size);
        assert(data != nullptr);
        memcpy(data, fileData.data(), size);
        *pdwSize = (unsigned long) size;
        pData = data;
    }).wait();

    return pData;
}

void Sample3DSceneRenderer::CreateDeviceDependentResources()
{
    auto task = concurrency::create_task<>([this]()
    {
        bool useTweenedNormal = true;
        LoadResourceFunc loadResourceFunc = Sample3DSceneRenderer::LoadResource;
        ID3D11Device * device = m_deviceResources->GetD3DDevice();
        ID3D11DeviceContext * deviceContext = m_deviceResources->GetD3DDeviceContext();

        InitDeviceDependentDolphinResources(useTweenedNormal, loadResourceFunc, device, deviceContext);

        m_loadingComplete = true;
    });
}

void Sample3DSceneRenderer::ReleaseDeviceDependentResources()
{
	m_loadingComplete = false;
    UninitDeviceDependentDolphinResources();
}