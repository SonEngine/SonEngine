#pragma once

#include "directxtk12/SpriteBatch.h"
#include "directxtk12/SpriteFont.h"
#include "directxtk12/GraphicsMemory.h"

#include "BaseApp.h"
#include "Renderer.h"
#include "Constants.h"
#include "RenderEngine.h"
#include "PhysXEngine.h"

namespace Core {
	class MultiThreadApp : public BaseApp
	{
	public:

		MultiThreadApp();
		MultiThreadApp(const int width, const int height, const int guiWidth);
		//MultiThreadApp(const int width, const int height);

		virtual ~MultiThreadApp();
		virtual int Run() override;

	protected:
		virtual bool InitDirectX() override;
		virtual bool InitGUI() override;

	protected:
		void Update(float deltaTime);
		// Called when the window is resized
		void OnResize() override;

		// C 눌리면 호출
		void OnCapture() override;

	private:
		Microsoft::WRL::ComPtr<IDXGIFactory7> m_dxgiFactory;
		Microsoft::WRL::ComPtr<IDXGIAdapter4> m_adapter;
		Microsoft::WRL::ComPtr<ID3D12Device5> m_device;
		Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swapChain;


		float deltaTime = 0.f;
		int m_guiWidth;

	private:
		std::shared_ptr<RenderEngine> m_renderEngine;
		std::shared_ptr<PhysXEngine> m_physXEngine;
	};
}