#pragma once

#include "Common\StepTimer.h"
#include "Common\DeviceResources.h"
#include "Content\SceneRenderer.h"
#include "Content\StatusTextRenderer.h"

// Renders Direct2D and 3D content on the screen.
namespace MPM_Snow_DX
{
	class MPM_Snow_DXMain : public DX::IDeviceNotify
	{
	public:
		MPM_Snow_DXMain(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		~MPM_Snow_DXMain();
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
		std::unique_ptr<SceneRenderer> m_sceneRenderer;
		std::unique_ptr<StatusTextRenderer> m_statusTextRenderer;

		// Rendering loop timer.
		DX::StepTimer m_timer;
	};
}