#pragma once

#include "Common\StepTimer.h"
#include "Common\DeviceResources.h"
#include "Content\Sample3DSceneRenderer.h"
#include "Content\SampleFpsTextRenderer.h"
#include "Content\DolphinSceneRenderer.h"

// Renders Direct2D and 3D content on the screen.
namespace dolphin
{
	class dolphinMain : public DX::IDeviceNotify
	{
	public:
		dolphinMain(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		~dolphinMain();
		void CreateWindowSizeDependentResources();
		void Update();
		bool Render();

		// IDeviceNotify
		virtual void OnDeviceLost();
		virtual void OnDeviceRestored();

	private:
		// Cached pointer to device resources.
		std::shared_ptr<DX::DeviceResources> m_deviceResources;

        bool m_showDolphin;

        std::unique_ptr<DolphinSceneRenderer> m_dolphinSceneRenderer;
        std::unique_ptr<Sample3DSceneRenderer> m_sceneRenderer;
		std::unique_ptr<SampleFpsTextRenderer> m_fpsTextRenderer;

		// Rendering loop timer.
		DX::StepTimer m_timer;
	};
}