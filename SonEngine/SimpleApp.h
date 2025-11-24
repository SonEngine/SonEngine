#pragma once

#include "directxtk12/SpriteBatch.h"
#include "directxtk12/SpriteFont.h"
#include "directxtk12/GraphicsMemory.h"

#include "BaseApp.h"
#include "Renderer.h"
#include "Constants.h"
#include "PhongHLSLCompat.h"
#include "Camera.h"
#include "Light.h"
#include "TextureLoader.h"
#include <array>

class StaticMesh;

namespace Core {
	class SimpleApp : public BaseApp
	{

	public:

		SimpleApp();
		SimpleApp(const int width, const int height);

		virtual ~SimpleApp();
		virtual int Run() override;
	private:

		// Init
		virtual bool InitDirectX() override;
		virtual bool InitGUI() override;

		// Called when the window is resized
		void OnResize() override;

		void Update(float deltaTime);
		void UpdateGUI(float deltaTime);

		void Render(float deltaTime);
		void RenderScene(const std::string& psoName);
		void RenderGUI(float deltaTime);
		void RenderText(const std::string& str);
		void DrawString(const std::string & str);

		// Fin

		virtual bool FinDirectX();
		virtual void Finalize(float deltaTime);

		void CreateCommandObjects();

		void CreateSwapChain();
		void CreateDepthBuffer();

		void BuildGeometry();

		void BuildConstantBuffers();

		void CreateTextures();
		void CreateTexts();

	private:




		D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentRtvCpuHandle() const;
		D3D12_CPU_DESCRIPTOR_HANDLE GetDSVCpuHandle() const;

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

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_swapChainRTVHeap;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_DSVHeap;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_guiFontHeap;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_textRtvHeap;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_textSrvHeap;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_fontSrvHeap;

	private:

		Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
		UINT m_currentFence = 0;

	private:

		int m_frameIndex = 0;
		static const UINT m_swapChainBufferCount = 2;
		static const UINT m_dsBufferCount = 1;

		UINT m_cbvSrvDescriptorSize = 0;
		UINT m_rtvDescriptorSize = 0;
		UINT m_dsvDescriptorSize = 0;

		D3D12_VIEWPORT m_viewport;
		D3D12_RECT m_scissorRect;

	private:
		Microsoft::WRL::ComPtr<ID3D12Resource> m_swapChainResources[m_swapChainBufferCount];
		Microsoft::WRL::ComPtr<ID3D12Resource> m_depthStencilBuffer;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_textRT;
		

	private:

		std::shared_ptr<StaticMesh> mesh;

		std::vector< std::shared_ptr<StaticMesh>> phongMeshes;
		std::vector <std::shared_ptr<Actor>> m_actors;
	private:

		LocalConstant localConstant;

		// Global Constant
		GlobalConstant globalConstant;
		PhongGlobalConstant phongGC;

		// Global Constant Buffers
		Microsoft::WRL::ComPtr<ID3D12Resource> m_globalCB;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_phongGCB;


		// ConstantBuffer Pointers
		// cpu에서 gpu 갱신 시 사용
		void* pGlobalConstant = nullptr;
		void* pPhongCB = nullptr;

	private:
		float m_aspectRatio;
		float m_fovDegrees = 60.f;
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
		std::array<FLOAT, 4> fontClearColor;

		// character
	private:
		std::shared_ptr<Camera> m_camera;
		std::shared_ptr<Light> m_directionLight;

	private:
		RECT windowRect;

	private:
		int selectedPSOIdx = 1;
		std::string renderPSO = "phongPSO";


	private:
		std::shared_ptr<TextureLoader> m_textureLoader;
		std::shared_ptr<TextureLoader> m_fallbackLoader;
		std::string texturePath;
		std::string fallbackPath;
		std::string DDSPath;
		std::string fallbackDDSPath;

		// gui에서 사용
	private:
		StaticMesh* selectedMesh = nullptr;

	// text render 용
	private:
		std::shared_ptr<DirectX::SpriteBatch> spriteBatch;
		std::shared_ptr<DirectX::SpriteFont> font;
		std::unique_ptr < DirectX::GraphicsMemory > m_graphicsMemory;
	};
}