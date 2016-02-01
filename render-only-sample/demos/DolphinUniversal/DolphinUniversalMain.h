#pragma once

#include "Common\StepTimer.h"
#include "Common\DeviceResources.h"
#include "Content\Sample3DSceneRenderer.h"
#include "Content\SampleFpsTextRenderer.h"

// Renders Direct2D and 3D content on the screen.
namespace DolphinUniversal
{
	class DolphinUniversalMain : public DX::IDeviceNotify
	{
	public:
		DolphinUniversalMain(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		~DolphinUniversalMain();
		void CreateWindowSizeDependentResources();
		void Update();
		bool Render();

		// IDeviceNotify
		virtual void OnDeviceLost();
		virtual void OnDeviceRestored();

	private:
		// Cached pointer to device resources.
		std::shared_ptr<DX::DeviceResources> m_deviceResources;

		// TODO: Replace with your own content renderers.
		std::unique_ptr<Sample3DSceneRenderer> m_sceneRenderer;
#ifdef USE_FPS
		std::unique_ptr<SampleFpsTextRenderer> m_fpsTextRenderer;
#endif

		// Rendering loop timer.
		DX::StepTimer m_timer;
	};
}