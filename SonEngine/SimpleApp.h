#pragma once

#include "BaseApp.h"
#include "Renderer.h"
#include "Constants.h"
#include "Camera.h"
#include "Light.h"
#include <array>

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
		void RenderScene(const std::string & psoName);
		virtual void RenderGUI(float deltaTime) override;

		// Fin

		virtual bool FinDirectX() override;
		virtual void Finalize(float deltaTime) override;

		void CreateCommandObjects();

		void CreateSwapChain();

		void BuildGeometry();

		void BuildConstantBuffers();

		void CreateTextures();

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
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_texturesHeap;

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

		std::shared_ptr<StaticMesh> mesh;

	private:

		LocalConstant localConstant;

		// Global Constant
		GlobalConstant globalConstant;
		PhongGlobalConstant phongGC;

		// Local Constant Buffers
		Microsoft::WRL::ComPtr<ID3D12Resource> m_localCB;

		// Global Constant Buffers
		Microsoft::WRL::ComPtr<ID3D12Resource> m_globalCB;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_phongGCB;

		// Texture Buffers
		Microsoft::WRL::ComPtr<ID3D12Resource> m_texture;

		// ConstantBuffer Pointers
		// cpu에서 gpu 갱신 시 사용
		void* pLocalConstant = nullptr;
		void* pGlobalConstant = nullptr;
		void* pPhongCB = nullptr;

	private:
		float m_aspectRatio;
		float m_fovDegrees = 80.f;
		float m_fovRadians;
		float m_fovAngle = 70.f;
		float m_nearZ = 0.1f;
		float m_farZ = 100.f;

	private:
		float m_zValue = 0.f;

	private:
		bool m_projFlag = false;

	private:
		std::array<float, 4> rtvClearColor;

		// character
	private:
		std::shared_ptr<Camera> m_camera;
		std::shared_ptr<Light> m_directionLight;

	private:
		RECT windowRect;
	};
}