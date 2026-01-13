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
#include "DLModel.h"
#include "Scene.h"
#include "GameCommand.h"

enum RenderType {
	RT_TEXT,
	RT_Default,
	RT_PointCloud,
	RT_CubeMap,
	RT_Dot /*렌더타겟에 점하나 넣고 texture 그리기 용도*/
};

enum RenderPassType {
	RPT_Default,
	RPT_CubeMapPass,
	RPT_DepthOnlyPass
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
	void RequestRunDL();
	void OnResize();

public:
	// primitive 등록
	void RegisterPrimitive(class PrimitiveComponent* primitive);

protected:
	void CreateCommandObjects();
	void CreateSwapChain(IDXGIFactory7* factory, HWND wnd);
	void CreateMainDepthBuffer();
	void CreateDepthBuffers();
	void CreateTextureBuffers();
	void UpdateGUI();
	void CreateTextures();
	void CreateCubeMap();
	void CreateFonts();

	void RenderWithText();
	void RenderWithCompute();
	void DrawingWithMouse();
	void BuildFrameResources();

	void Update(float deltaTime);
	void AddTextProxy(SceneComponent* component);
	void UpdateMousePosition();
	void RenderMeshes(const std::string& psoName, ID3D12GraphicsCommandList* commandList, MeshType meshType, RenderPassType rpType, int cubeGCBIdx = 0);
	void Render(const std::string& psoName, int idx, MeshType meshType, bool isFinal, bool clear);

	//void Render(const std::string& psoName, int idx, RenderType renderType, bool isFinal, bool clear);
	void RenderGUI(bool isFinal);
	void Compute(const std::string& cpsoName, int idx, bool isFinal, D3D12_RESOURCE_STATES prevState);

	void RenderCube(const std::string& psoName, const std::string& proxyPsoName, int idx, MeshType meshType, bool isFinal, bool clear);

	//void RenderCube(const std::string& psoName, int idx, MeshType meshType, bool isFinal, bool clear);


	void DepthOnlyPass(const std::string& psoName, const std::string& proxyPsoName, int idx, MeshType meshType, bool isFinal, bool clear);

	void UpdateTexts();

public:
	void UpdateGlobalConstantBuffer(const ViewProjInfo& viewProjInfo, const std::vector<PBRLightInfo>& lightInfos);
	void UpdatePBGlobalConstantBuffer(const int& guiWidth, const MouseInputState& mouseInputState);
	void Tick(float deltaTime);
	void Quit();


private:
	void GenerateMips(ID3D12Resource* tex);
	void ClearTexture();

protected:
	D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentRtvCpuHandle() const;
	D3D12_CPU_DESCRIPTOR_HANDLE GetCubeMapRtvCpuHandle(int i) const;
	D3D12_CPU_DESCRIPTOR_HANDLE GetCubeMapDsvCpuHandle(int i) const;
	D3D12_CPU_DESCRIPTOR_HANDLE GetDSVCpuHandle(int idx = 0) const;
	ID3D12Resource* GetCurrentSwapChainResource() const;

private:
	void FlushCommands();
	void FlushResourceCommands();

	void SaveTextureGPU(const std::string& name, D3D12_RESOURCE_STATES state, UINT16 miplevel = 0, bool saveCPU = true);

	void SaveTextureCPU();
	void RunDLModel();

	// GameCommand Drain
private:
	void ApplyGameCommand(const GameCmd& cmd);
	void ApplyImpl(const CmdAddActor& c);
	void ApplyImpl(const CmdUpdateActorConstant& c);

private:
	ID3D12Device5* m_device;

private:
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_commandAllocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> gui_commandAllocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> gui_commandList;

	Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_commandQueue;

private:
	D3D12_VIEWPORT m_viewport;
	D3D12_VIEWPORT m_hdrViewport;
	D3D12_RECT m_scissorRect;
	int m_width;
	int m_height;
	int m_guiWidth;
	HWND mainWnd;

private:
	int m_currentBackBufferIndex = 0;
	static const UINT m_swapChainBufferCount = 2;
	static const UINT m_dsBufferCount = 2;
	UINT m_cbvSrvDescriptorSize = 0;
	UINT m_rtvDescriptorSize = 0;
	UINT m_dsvDescriptorSize = 0;
	std::array<float, 4> rtvClearColor;
	std::array<float, 4> blackClearColor;

private:
	Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swapChain;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_swapChainRTVHeap;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_swapChainResources[m_swapChainBufferCount];
	Microsoft::WRL::ComPtr<ID3D12Resource> m_depthStencilBuffer;

private:
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_UAVHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_UAVCPUHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_SRVHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_DSVHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_guiFontHeap;

	//cubemap
private:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_cubeMap;
	//DXGI_FORMAT cubeFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_cubeMapRtvHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_cubeMapSrvHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_cubeMapDsvHeap;

	Microsoft::WRL::ComPtr<ID3D12Resource> m_cubeDepthBuffer[6];

	PBRGlobalConstant m_cubePhongGC[6];
	Microsoft::WRL::ComPtr<ID3D12Resource> m_cubeMapGCB[6];
	std::vector<void*> m_pCubeGC;

	D3D12_VIEWPORT m_cubeViewport;
	D3D12_RECT m_cubeScissorRect;

	UINT cubeWidth = 512;
	UINT cubeHeight = 512;
	Vector3 cubeMapPos;
	//UINT cubeWidth = 1280;
	//UINT cubeHeight = 720;
	std::array<float, 4> cubeRtvClearColor;
	std::string playerCubeMapTextureName = "cubeMapPlayer";

// depthOnly
private:
	UINT depthOnlyWidth = 1024*4;
	UINT depthOnlyHeight = 1024*4;
	std::string depthOnlyTextureName = "depthOnlyTex";
	Microsoft::WRL::ComPtr<ID3D12Resource> m_depthOnlyBuffer;
	D3D12_VIEWPORT m_depthOnlyViewport;
	D3D12_RECT m_depthOnlyRect;

// Compute Shader 용
private:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_computeBuffer;
	//DXGI_FORMAT m_computeBufferFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;
	DXGI_FORMAT m_computeBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	//DXGI_FORMAT m_computeBufferFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;
	std::string m_computeTextureName = "ComTex";
	std::string m_cubeMapTextureName = "CubeMap_SkyDiffuseHDR";
	UINT computeTextureDIM = 1024 * 4;
	UINT computeTextureDIMX;
	UINT computeTextureDIMY;

	//hdr backbuffer
private:


	Microsoft::WRL::ComPtr<ID3D12Resource> m_hdrBuffer;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_hdrRtvHeap;
	std::string m_hdrTextureName = "hdrTex";

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
	std::string phongPSO = "phongPSO";
	std::string textPSO = "textPSO";
	std::string computePSO = "defaultCPSO";
	std::string genCubeMapPSO = "genCubeMapPSO";
	std::string genPBRCubeMapPSO = "genPBRCubeMapPSO";
	std::string cubeMapPSO = "cubeMapPSO";
	std::string currentPbrPSO = "pbrPSO";
	std::string wirePbrPSO = "wire_pbrPSO";
	std::string depthOnlyPbrPSO = "dsOnly_pbrPSO";
	std::string pbrPSO = "pbrPSO";

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
	std::string saveTextureName = "BackBuffer";

private:
	std::thread saveThread;
	std::thread renderThread;
	std::atomic<bool> frameReady = false;
	std::atomic<bool> saveReady = false;
	std::atomic<bool> runDLReady = false;
	std::atomic<bool> resize = false;
	std::atomic<bool> captureDirty = false;
	std::atomic<bool> runDLDirty = false;

	std::mutex r_mtx;
	std::mutex g_mtx;
	std::mutex queue_mtx;
	std::mutex capture_mtx;

	std::condition_variable cv;
	std::condition_variable captureCv;

private:
	std::vector<class PrimitiveComponent*> m_primitives;
	bool test = true;
	bool gui_wireFrameMode = false;

private:
	MouseInputStateHelper* pMouseinputStateHelper = nullptr;
	POINT currMousPt = { 0,0 };
	POINT prevMousePt = { 0,0 };

private:
	std::shared_ptr<BoundedQueue<FramePacket>> m_frameQueue;
	std::shared_ptr<BoundedQueue<RenderCmd>> m_renderCmdQueue;
	std::shared_ptr<BoundedQueue<GameCmd>> m_renderToMainCmdQueue;
	uint64_t m_frameId = 0;
	FramePacket packet;
	FramePacket r_packet;
	RenderCmd r_cmd;
	GameCmd g_cmd;

	std::unordered_map<uint32_t, std::string> r_idToName;
	std::unordered_map<std::string, uint32_t> r_nameToId;
	uint32_t r_idMax = 0;
	int r_selecteId = 0;

	int saveMipLevel = 4;

	LocalConstant guiLocalConstant;
	int gui_cubemapMipLevel = 0;
	FLOAT computeClearColor[4];
	float guiPenRadius;
	float guiPenColor[3];
	bool clearFlag = false;
	//ImGuiContext* m_imguiCtx = nullptr;

private:
	//std::shared_ptr<DLModel> dlModel;
	std::atomic<int> dlRet{ 0 };
	bool printRet = false;

private:
	std::shared_ptr<Scene> m_scene;
};