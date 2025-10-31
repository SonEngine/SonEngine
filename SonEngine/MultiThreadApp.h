#pragma once

#include "directxtk12/SpriteBatch.h"
#include "directxtk12/SpriteFont.h"
#include "directxtk12/GraphicsMemory.h"

#include <atomic>
#include <condition_variable>
#include <thread>
#include <mutex>
#include <array>

#include "BaseApp.h"
#include "Renderer.h"
#include "Constants.h"
#include "Camera.h"
#include "Light.h"
#include "TextureLoader.h"
#include "FrameResource.h"
#include "Actor.h"



namespace Core {
	class MultiThreadApp : public BaseApp
	{
	public:

		MultiThreadApp();
		MultiThreadApp(const int width, const int height);

		virtual ~MultiThreadApp();
		virtual int Run() override;

	protected:
		virtual bool InitDirectX() override;
		virtual bool InitGUI() override;

		// Called when the window is resized
		virtual void OnResize() override;

	protected:
		void CreateCommandObjects();
		void CreateSwapChain();
		void CreateDepthBuffer();
		void CreateTextures();
		void CreateFonts();

		void BuildGeometry();
		void BuildFrameResources();

		void PostActorChanges();
		void Update(float deltaTime);
		void BuildProxy();
		void AddProxy(SceneComponent* component);
		void AddTextProxy(SceneComponent* component);
		void Render(const std::string& psoName, int idx, bool isText, bool isFinal);

		void UpdateTexts();
		void UpdateGUI(float deltaTime);

	protected:
		D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentRtvCpuHandle() const;
		D3D12_CPU_DESCRIPTOR_HANDLE GetDSVCpuHandle() const;
		ID3D12Resource* GetCurrentSwapChainResource() const;

	private:
		void FlushCommands();

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
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_swapChainRTVHeap;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_DSVHeap;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_guiFontHeap;

	private:
		D3D12_VIEWPORT m_viewport;
		D3D12_RECT m_scissorRect;
		RECT windowRect;

	private:
		int m_currentBackBufferIndex = 0;
		static const UINT m_swapChainBufferCount = 2;
		static const UINT m_dsBufferCount = 1;
		UINT m_cbvSrvDescriptorSize = 0;
		UINT m_rtvDescriptorSize = 0;
		UINT m_dsvDescriptorSize = 0;
		std::array<float, 4> rtvClearColor;

	private:
		Microsoft::WRL::ComPtr<ID3D12Resource> m_swapChainResources[m_swapChainBufferCount];
		Microsoft::WRL::ComPtr<ID3D12Resource> m_depthStencilBuffer;

		// Texture
	private:
		std::shared_ptr<TextureLoader> m_textureLoader;
		std::shared_ptr<TextureLoader> m_fallbackLoader;
		std::string texturePath;
		std::string fallbackPath;
		std::string DDSPath;
		std::string fallbackDDSPath;

	private:
		std::vector<std::shared_ptr<Actor>> m_actors;
		std::vector<std::shared_ptr<Actor>> m_textActors;
		std::vector<std::shared_ptr<Actor>> m_addActors;
		std::shared_ptr<Actor> m_player;
		std::shared_ptr<Camera> m_camera;

	private:
		std::atomic<bool> isRunning = true;
		std::atomic<bool> frameReady = false;
		std::string renderPSO = "phongPSO";
		std::string textPSO = "defaultPSO";
		std::mutex r_mtx;
		std::mutex g_mtx;
		std::mutex queue_mtx;

		std::condition_variable cv;
		float deltaTime = 0.f;

		//FrameResource
	private:
		static const int m_frameResourceCount = 2;
		std::vector<std::shared_ptr<FrameResource>> m_frameResources;
		FrameResource* currentFrameResource = nullptr;
		FrameResource* r_currentFrameResource = nullptr;
		int m_currentResourceIndex = 0;
		int r_currentResourceIndex = 0;
		bool isFirstFrame = true;

		UINT64 m_currentFence = 0;
		UINT64 m_currentBufferFence = 0;

		Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
		Microsoft::WRL::ComPtr<ID3D12Fence> m_createBufferfence;

	// text용
	private:
		std::shared_ptr<DirectX::SpriteBatch> spriteBatch;
		std::shared_ptr<DirectX::SpriteFont> font;
		std::unique_ptr < DirectX::GraphicsMemory > m_graphicsMemory;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_fontSrvHeap;

	};
}