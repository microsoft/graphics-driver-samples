#include "pch.h"
#include "DolphinUniversalMain.h"
#include "Common\DirectXHelper.h"

using namespace DolphinUniversal;
using namespace Windows::Foundation;
using namespace Windows::System::Threading;
using namespace Concurrency;

// Loads and initializes application assets when the application is loaded.
DolphinUniversalMain::DolphinUniversalMain(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
	m_deviceResources(deviceResources)
{
	// Register to be notified if the Device is lost or recreated
	m_deviceResources->RegisterDeviceNotify(this);

	// TODO: Replace this with your app's content initialization.
	m_sceneRenderer = std::unique_ptr<Sample3DSceneRenderer>(new Sample3DSceneRenderer(m_deviceResources));

#ifdef USE_FPS
	m_fpsTextRenderer = std::unique_ptr<SampleFpsTextRenderer>(new SampleFpsTextRenderer(m_deviceResources));
#endif

}

DolphinUniversalMain::~DolphinUniversalMain()
{
	// Deregister device notification
	m_deviceResources->RegisterDeviceNotify(nullptr);
}

// Updates application state when the window size changes (e.g. device orientation change)
void DolphinUniversalMain::CreateWindowSizeDependentResources() 
{
	// TODO: Replace this with the size-dependent initialization of your app's content.
	m_sceneRenderer->CreateWindowSizeDependentResources();
}

// Updates the application state once per frame.
void DolphinUniversalMain::Update() 
{
	// Update scene objects.
	m_timer.Tick([&]()
	{
		m_sceneRenderer->Update(m_timer);

#ifdef USE_FPS
		m_fpsTextRenderer->Update(m_timer);
#endif

	});
}

// Renders the current frame according to the current application state.
// Returns true if the frame was rendered and is ready to be displayed.
bool DolphinUniversalMain::Render() 
{
	// Don't try to render anything before the first Update.
	if (m_timer.GetFrameCount() == 0)
	{
		return false;
	}

    bool bRendered;

	// Render the scene objects.
	bRendered = m_sceneRenderer->Render();

#ifdef USE_FPS
    if (bRendered)
        m_fpsTextRenderer->Render();
#endif

	return bRendered;
}

// Notifies renderers that device resources need to be released.
void DolphinUniversalMain::OnDeviceLost()
{
	m_sceneRenderer->ReleaseDeviceDependentResources();

#ifdef USE_FPS
	m_fpsTextRenderer->ReleaseDeviceDependentResources();
#endif
}

// Notifies renderers that device resources may now be recreated.
void DolphinUniversalMain::OnDeviceRestored()
{
	m_sceneRenderer->CreateDeviceDependentResources();
#if USE_FPS
	m_fpsTextRenderer->CreateDeviceDependentResources();
#endif
	CreateWindowSizeDependentResources();
}
