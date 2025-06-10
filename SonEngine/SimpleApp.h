#pragma once

#include "BaseApp.h"
#include "Renderer.h"

namespace Core {
	class SimpleApp : public BaseApp
	{

	public:
		SimpleApp();
		SimpleApp(const int width, const int height);

	private:

		// Init

		virtual bool InitDirectX() override;
		virtual bool InitGUI() override;
		
		// Called when the window is resized
		virtual void OnResize() override;

		virtual void Update(float deltaTime) override;
		virtual void UpdateGUI(float deltaTime) override;

		virtual void Render(float deltaTime) override;
		virtual void RenderGUI(float deltaTime) override;

		// Fin

		virtual bool FinDirectX() override;
		virtual void Finalize(float deltaTime) override;

		void CreateCommandObjects();

		void CreateSwapChain();

	private:
		Microsoft::WRL::ComPtr<IDXGIFactory7> m_dxgiFactory;
		Microsoft::WRL::ComPtr<IDXGIAdapter4> m_adapter;
		Microsoft::WRL::ComPtr<ID3D12Device5> m_device;
		Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swapChain;


	private:
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_commandAllocator;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_commandQueue;

	private:
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_swapChainRtvHeap;


	private:
		int m_frameIndex;
		const static UINT m_swapChainBufferCount = 2;

		UINT m_cbvSrvDescriptorSize;
		UINT m_rtvDescriptorSize;
		UINT m_dsvDescriptorSize;
		
		D3D12_VIEWPORT m_viewport;
		D3D12_RECT m_scissorRect;

	private:
		GraphicsUtils::Utility* m_utility;
	};
}