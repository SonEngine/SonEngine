#pragma once

#include <array>
#include <atomic>
#include <condition_variable>
#include <thread>
#include <mutex>

#include "d3d12.h"
#include "directxtk12/SpriteBatch.h"
#include "directxtk12/SpriteFont.h"
#include "directxtk12/GraphicsMemory.h"

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"

#include "TextureLoader.h"
#include "FrameResource.h"
//#include "ViewProjInfo.h"
#include "ImageInfo.h"
#include "Renderer.h"
#include "MouseInputState.h"
#include "BoundedQueue.h"

enum RenderType {
	RT_TEXT,
	RT_Default,
	RT_PointCloud,
	RT_Dot /*렌더타겟에 점하나 넣고 texture 그리기 용도*/
};

class RenderEngine {
public:
	RenderEngine(ID3D12Device5* device = nullptr);
	virtual ~RenderEngine();

public:
	bool Initialize(int width, int height, int guiWidth, IDXGIFactory7* factory, HWND wnd, MouseInputStateHelper* mouseInputState);
	bool InitGUI(HWND wnd);

	void RequestResize(int newWidth, int newHeight);
	void RequestCapture();
	void OnResize();

public:
	// primitive 등록
	void RegisterPrimitive(class PrimitiveComponent* primitive);

protected:
	void CreateCommandObjects();
	void CreateSwapChain(IDXGIFactory7* factory, HWND wnd);
	void CreateDepthBuffer();
	void UpdateGUI();
	void CreateTextures();
	void CreateFonts();

	void RenderWithText();
	void RenderWithCompute();
	void DrawingWithMouse();
	void BuildFrameResources();

	void PostActorChanges();
	void BuildRenderProxy();
	void AddProxy(SceneComponent* component, FrameResource* fr);
	void Update(float deltaTime);
	//void BuildRenderProxy(const std::vector<std::shared_ptr<Actor>>& actors, Actor* player);
	//void AddProxy(SceneComponent* component);
	void AddTextProxy(SceneComponent* component);
	void UpdateMousePosition();
	void Render(const std::string& psoName, int idx, RenderType renderType, bool isFinal, bool clear);
	void RenderGUI(bool isFinal);
	void Compute(const std::string& cpsoName, int idx, bool isFinal, D3D12_RESOURCE_STATES prevState);

	void UpdateTexts();


public:
	void UpdateGlobalConstantBuffer(const ViewProjInfo& viewProjInfo, const std::vector<LightInfo>& lightInfos);
	void UpdatePBGlobalConstantBuffer(const int& guiWidth, const MouseInputState& mouseInputState);
	void Tick(float deltaTime);
	void Quit();

protected:
	D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentRtvCpuHandle() const;
	D3D12_CPU_DESCRIPTOR_HANDLE GetDSVCpuHandle() const;
	ID3D12Resource* GetCurrentSwapChainResource() const;

private:
	void FlushCommands();
	void FlushResourceCommands();

	void SaveTextureGPU(const std::string& name, D3D12_RESOURCE_STATES state);
	void SaveTextureCPU();


private:
	ID3D12Device5* m_device;

private:
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_commandAllocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_commandQueue;

private:
	D3D12_VIEWPORT m_viewport;
	D3D12_RECT m_scissorRect;
	int m_width;
	int m_height;
	int m_guiWidth;
	HWND mainWnd;

private:
	int m_currentBackBufferIndex = 0;
	static const UINT m_swapChainBufferCount = 2;
	static const UINT m_dsBufferCount = 1;
	UINT m_cbvSrvDescriptorSize = 0;
	UINT m_rtvDescriptorSize = 0;
	UINT m_dsvDescriptorSize = 0;
	std::array<float, 4> rtvClearColor;

private:
	Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swapChain;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_swapChainRTVHeap;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_swapChainResources[m_swapChainBufferCount];
	Microsoft::WRL::ComPtr<ID3D12Resource> m_depthStencilBuffer;

private:
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_UAVHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_SRVHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_DSVHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_guiFontHeap;

	// Compute Shader 용
private:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_computeBuffer;
	//DXGI_FORMAT m_computeBufferFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;
	DXGI_FORMAT m_computeBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	std::string m_computeTextureName = "ComTex";
	UINT computeTextureDIM = 1024 * 4;
	UINT computeTextureDIMX;
	UINT computeTextureDIMY;

	// TextureLoader
private:
	std::shared_ptr<TextureLoader> m_textureLoader;
	std::shared_ptr<TextureLoader> m_fallbackLoader;
	std::string texturePath;
	std::string fallbackPath;
	std::string DDSPath;
	std::string fallbackDDSPath;

private:
	std::string renderPSO = "phongPSO";
	std::string textPSO = "textPSO";
	std::string computePSO = "defaultCPSO";

	//FrameResource
private:
	static const int m_frameResourceCount = 3;
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
	UINT textCount = 0;
	std::vector<std::wstring> cpuTexts;

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_saveBuffer;
	void* pSaveBuffer;
	std::string imageFilePath = "images\\";
	ImageInfo imageInfo;

private:
	std::thread saveThread;
	std::thread renderThread;
	std::atomic<bool> frameReady = false;
	std::atomic<bool> saveReady = false;
	std::atomic<bool> resize = false;
	std::atomic<bool> captureDirty = false;

	std::mutex r_mtx;
	std::mutex g_mtx;
	std::mutex queue_mtx;
	std::mutex capture_mtx;

	std::condition_variable cv;
	std::condition_variable captureCv;

private:
	std::vector<class PrimitiveComponent*> m_primitives;
	bool test = false;

private:
	MouseInputStateHelper* pMouseinputStateHelper = nullptr;
	POINT currMousPt = { 0,0 };
	POINT prevMousePt = { 0,0 };

private:
	std::shared_ptr<BoundedQueue> m_boundedQueue;
	uint64_t m_frameId = 0;
	FramePacket packet;
	FramePacket r_packet;
};