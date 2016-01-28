#include "pch.h"
#include "dolphinMain.h"
#include "Common\DirectXHelper.h"

using namespace dolphin;
using namespace Windows::Foundation;
using namespace Windows::System::Threading;
using namespace Concurrency;

// Loads and initializes application assets when the application is loaded.
dolphinMain::dolphinMain(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
	m_deviceResources(deviceResources)
{
	// Register to be notified if the Device is lost or recreated
	m_deviceResources->RegisterDeviceNotify(this);

    m_showDolphin = true;
    
    if (m_showDolphin)
    {
        m_dolphinSceneRenderer = std::unique_ptr<DolphinSceneRenderer>(new DolphinSceneRenderer(m_deviceResources));
    }
    else
    {
        m_sceneRenderer = std::unique_ptr<Sample3DSceneRenderer>(new Sample3DSceneRenderer(m_deviceResources));
        m_fpsTextRenderer = std::unique_ptr<SampleFpsTextRenderer>(new SampleFpsTextRenderer(m_deviceResources));
    }


	// TODO: Change the timer settings if you want something other than the default variable timestep mode.
	// e.g. for 60 FPS fixed timestep update logic, call:
	/*
	m_timer.SetFixedTimeStep(true);
	m_timer.SetTargetElapsedSeconds(1.0 / 60);
	*/
}

dolphinMain::~dolphinMain()
{
	// Deregister device notification
	m_deviceResources->RegisterDeviceNotify(nullptr);
}

// Updates application state when the window size changes (e.g. device orientation change)
void dolphinMain::CreateWindowSizeDependentResources() 
{
    if (m_showDolphin)
    {
        m_dolphinSceneRenderer->CreateWindowSizeDependentResources();
    }
    else
    {
        m_sceneRenderer->CreateWindowSizeDependentResources();
    }
}

// Updates the application state once per frame.
void dolphinMain::Update() 
{
	// Update scene objects.
	m_timer.Tick([&]()
	{
        if (m_showDolphin)
        {
            m_dolphinSceneRenderer->Update(m_timer);
        }
        else
        {
            m_sceneRenderer->Update(m_timer);
            m_fpsTextRenderer->Update(m_timer);
        }
	});
}

// Renders the current frame according to the current application state.
// Returns true if the frame was rendered and is ready to be displayed.
bool dolphinMain::Render() 
{
	// Don't try to render anything before the first Update.
	if (m_timer.GetFrameCount() == 0)
	{
		return false;
	}

	auto context = m_deviceResources->GetD3DDeviceContext();

	// Reset the viewport to target the whole screen.
	auto viewport = m_deviceResources->GetScreenViewport();
	context->RSSetViewports(1, &viewport);

	// Reset render targets to the screen.
	ID3D11RenderTargetView *const targets[1] = { m_deviceResources->GetBackBufferRenderTargetView() };
	context->OMSetRenderTargets(1, targets, m_deviceResources->GetDepthStencilView());

	// Clear the back buffer and depth stencil view.
	context->ClearRenderTargetView(m_deviceResources->GetBackBufferRenderTargetView(), DirectX::Colors::CornflowerBlue);
	context->ClearDepthStencilView(m_deviceResources->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// Render the scene objects.
    if (m_showDolphin)
    {
        m_dolphinSceneRenderer->Render();
    }
    else
    {
        m_sceneRenderer->Render();
        m_fpsTextRenderer->Render();
    }

	return true;
}

// Notifies renderers that device resources need to be released.
void dolphinMain::OnDeviceLost()
{
    if (m_showDolphin)
    {
        m_dolphinSceneRenderer->ReleaseDeviceDependentResources();
    }
    else
    {
        m_sceneRenderer->ReleaseDeviceDependentResources();
        m_fpsTextRenderer->ReleaseDeviceDependentResources();
    }
}

// Notifies renderers that device resources may now be recreated.
void dolphinMain::OnDeviceRestored()
{
    if (m_showDolphin)
    {
        m_dolphinSceneRenderer->CreateDeviceDependentResources();
    }
    else
    {
        m_sceneRenderer->CreateDeviceDependentResources();
        m_fpsTextRenderer->CreateDeviceDependentResources();
    }
	CreateWindowSizeDependentResources();
}
