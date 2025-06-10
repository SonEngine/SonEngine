#pragma once

#include "BaseApp.h"
#include "Renderer.h"

class StaticMesh;

namespace Core {
	class SimpleApp : public BaseApp
	{

	public:

		SimpleApp();
		SimpleApp(const int width, const int height);

		virtual ~SimpleApp();

	private:

		// Init
		virtual bool InitDirectX() override;
		virtual bool InitGUI() override;
		
		// Called when the window is resized
		virtual void OnResize() override;

		virtual void Update(float deltaTime) override;
		virtual void UpdateGUI(float deltaTime) override;

		virtual void Render(float deltaTime) override;
		void RenderScene();
		virtual void RenderGUI(float deltaTime) override;

		// Fin

		virtual bool FinDirectX() override;
		virtual void Finalize(float deltaTime) override;

		void CreateCommandObjects();

		void CreateSwapChain();

		void BuildGeometry();

	private:

		D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentRtvCpuHandle() const;

		ID3D12Resource* GetCurrentSwapChainResource() const;

		void FlushCommands();

	private:

		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_commandAllocator;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_commandQueue;

	private:

		Microsoft::WRL::ComPtr<IDXGIFactory7> m_dxgiFactory;
		Microsoft::WRL::ComPtr<IDXGIAdapter4> m_adapter;
		Microsoft::WRL::ComPtr<ID3D12Device5> m_device;
		Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swapChain;

	private:

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_swapChainRtvHeap;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_guiFontHeap;

	private:

		Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
		UINT m_currentFence = 0;

	private:

		int m_frameIndex;
		static const UINT m_swapChainBufferCount = 2;

		UINT m_cbvSrvDescriptorSize;
		UINT m_rtvDescriptorSize;
		UINT m_dsvDescriptorSize;
		
		D3D12_VIEWPORT m_viewport;
		D3D12_RECT m_scissorRect;

	private:

		Microsoft::WRL::ComPtr<ID3D12Resource> m_swapChainResources[m_swapChainBufferCount];

	private:

		DXGI_FORMAT m_backbufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

	private:

		GraphicsUtils::Utility* m_utility;
		
	private:

		std::shared_ptr<StaticMesh> mesh;

	};
}