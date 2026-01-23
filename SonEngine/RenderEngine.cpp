#pragma warning(disable : 4996)

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

#include "RenderEngine.h"
#include "GraphicsCommon.h"

#include "RootSignature.h"
#include "PipelineState.h"
#include "GeometryGenerator.h"
#include "PrimitiveComponent.h"
#include "LightComponent.h"
#include "SkinnedMeshComponent.h"
#include "CollisionComponent.h"
#include "CubeMapComponent.h"
#include "DotComponent.h"
#include "ModelLoader.h"

#include "Directxtk12/DDSTextureLoader.h"
#include "directxtk12/ResourceUploadBatch.h"

#include <fp16.h>
#include <pix3.h>

using Microsoft::WRL::ComPtr;
using namespace GraphicsUtils;
using namespace Graphics;
using namespace Renderer;

RenderEngine::RenderEngine(ID3D12Device5* device)
	:m_device(device)
{
	m_scene = std::make_shared<Scene>(m_frameResourceCount);
}

RenderEngine::~RenderEngine()
{
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

bool RenderEngine::Initialize(int width, int height, int guiWidth, IDXGIFactory7* factory, HWND wnd, MouseInputStateHelper* mouseInputState)
{
	/*dlModel = std::make_shared<DLModel>();
	dlModel->Initialize("DL/models/mlp.pt");*/

	// TODO actorcount 개수 조절
	m_frameQueue = std::make_shared<BoundedQueue<FramePacket>>(m_frameResourceCount);
	m_renderAddCmdQueue.resize(actorCount);
	m_renderUpdateCmdQueue.resize(actorCount);
	for (size_t i = 0; i < actorCount; i++)
	{
		m_renderAddCmdQueue[i] = std::make_shared<BoundedQueue<AddCmd>>(m_frameResourceCount);
		m_renderUpdateCmdQueue[i] = std::make_shared<BoundedQueue<UpdateCmd>>(m_frameResourceCount);
	}
	m_renderToMainCmdQueue = std::make_shared<BoundedQueue<GameCmd>>(1024);

	pMouseinputStateHelper = mouseInputState;
	m_guiWidth = guiWidth;
	m_width = width;
	m_height = height;
	mainWnd = wnd;

	m_viewport = CD3DX12_VIEWPORT((FLOAT)m_guiWidth, 0.F, (FLOAT)(m_width - m_guiWidth), (FLOAT)m_height);
	m_scissorRect = CD3DX12_RECT((LONG)0, 0, (LONG)(m_width), (LONG)m_height);
	m_hdrViewport = CD3DX12_VIEWPORT((FLOAT)0.F, 0.F, (FLOAT)(m_width), (FLOAT)m_height);

	rtvClearColor = { 0.53F, 0.81F, 0.92F, 1.0F };
	blackClearColor = { 0.F, 0.F, 0.F, 1.0F };

	CreateCommandObjects();

	utility = std::make_shared<GraphicsUtils::Utility>(m_device, m_commandList.Get());
	m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_fence.GetAddressOf()));
	m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_createBufferfence.GetAddressOf()));

	// Descriptor Handle offset 구하기
	m_cbvSrvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	m_dsvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	// DescriptorHeap 생성
	utility->CreateDescriptorHeap(m_swapChainBufferCount, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, m_swapChainRTVHeap);
	utility->CreateDescriptorHeap(1, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, m_hdrRtvHeap);
	utility->CreateDescriptorHeap(m_dsBufferCount, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, m_DSVHeap);
	utility->CreateDescriptorHeap(1, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, m_fontSrvHeap, 0, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);

	// Compute Shader에서 사용할 Heap생성
	{
		utility->CreateDescriptorHeap(1, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, m_UAVHeap, 0, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
		utility->CreateDescriptorHeap(1, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, m_UAVCPUHeap, 0, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
		utility->CreateDescriptorHeap(1, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, m_SRVHeap, 0, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
	}

	CreateSwapChain(factory, wnd);
	CreateFonts();

	// Create SwapChain RTVs
	CD3DX12_CPU_DESCRIPTOR_HANDLE handle(m_swapChainRTVHeap->GetCPUDescriptorHandleForHeapStart());
	for (int i = 0; i < m_swapChainBufferCount; i++)
	{
		m_swapChain->GetBuffer(i, IID_PPV_ARGS(m_swapChainResources[i].ReleaseAndGetAddressOf()));
		m_device->CreateRenderTargetView(m_swapChainResources[i].Get(), nullptr, handle);

		handle.Offset(1, m_rtvDescriptorSize);
	}

	CreateTextures();
	CreateTextureBuffers();
	CreateDepthBuffers();

	clearFlag = true;

	CreateCubeMap();

	float r = 0.f;
	float g = 0.f;
	float b = 0.f;
	computeClearColor[0] = r;
	computeClearColor[1] = g;
	computeClearColor[2] = b;
	computeClearColor[3] = 1.f;
	{
		ThrowIfFailed(
			m_device->CreateCommandAllocator(
				D3D12_COMMAND_LIST_TYPE_DIRECT,
				IID_PPV_ARGS(gui_commandAllocator.ReleaseAndGetAddressOf())
			));

		ThrowIfFailed(
			m_device->CreateCommandList(
				0,
				D3D12_COMMAND_LIST_TYPE_DIRECT,
				gui_commandAllocator.Get(),
				nullptr,
				IID_PPV_ARGS(gui_commandList.ReleaseAndGetAddressOf())
			));
		gui_commandList->Close();
	}

	{
		m_commandAllocator->Reset();
		m_commandList->Reset(m_commandAllocator.Get(), nullptr);

		if (world)
		{
			world->Initialize(m_width - guiWidth, m_height, this, m_device, m_commandList.Get());
		}


		m_commandList->Close();
		ID3D12CommandList* commands[] = { m_commandList.Get() };
		m_commandQueue->ExecuteCommandLists(ARRAYSIZE(commands), commands);

		FlushCommands();
	}

	BuildFrameResources();

	saveThread = std::thread([&] {
		if (world == nullptr)
		{
			std::cout << "World 가 Null입니다.\n";
			return;
		}
		while (world->isRunning) {
			std::unique_lock<std::mutex> lock(capture_mtx);
			captureCv.wait(lock, [&] {return !world->isRunning || saveReady || runDLReady; });

			if (!world->isRunning)
			{
				break;
			}

			if (saveReady)
			{
				saveReady = false;
				lock.unlock();
				SaveTextureCPU();
			}
			//else if (runDLReady)
			//{
			//	runDLReady = false;
			//	lock.unlock();
			//	RunDLModel();
			//}
		}
		});
	// BOOKMARK
	renderThread = std::thread([&] {
		if (world == nullptr)
		{
			std::cout << "World 가 Null입니다 (renderThread).\n";
			return;
		}
		while (world->isRunning) {
			std::unique_lock<std::mutex> lock(g_mtx);
			cv.wait(lock, [&] { return !world->isRunning || frameReady; });

			if (!world->isRunning)
			{
				break;
			}
			if (resize)
			{
				resize = false;
				FlushResourceCommands();
				OnResize();
			}
			if (clearFlag)
			{
				clearFlag = false;
				FlushResourceCommands();
				ClearTexture();
			}
			if (captureDirty)
			{
				captureDirty = false;
				FlushResourceCommands();
				//SaveTextureGPU(saveTextureName, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, saveMipLevel, true);
				SaveTextureGPU("BackBuffer", D3D12_RESOURCE_STATE_PRESENT);
			}
			if (runDLDirty)
			{
				runDLDirty = false;
				FlushResourceCommands();
				SaveTextureGPU(m_computeTextureName, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, saveMipLevel, false);
			}

			frameReady = false;
			lock.unlock();

			m_frameQueue->Pop(r_packet);
			r_currentResourceIndex = r_packet.frameId % m_frameResourceCount;
			r_currentFrameResource = m_frameResources[r_currentResourceIndex].get();

			r_currentFrameResource->UpdateGlobalConstantBuffer(r_packet.gc);
			r_currentFrameResource->UpdatePBGlobalConstantBuffer(r_packet.pbgc);

			//DrainRenderCommands
			for (size_t i = 0; i < actorCount; i++)
			{
				if (m_renderAddCmdQueue[i]->TryPop(r_addCmd))
				{
					m_scene->Apply(r_addCmd);
				}
				if (m_renderUpdateCmdQueue[i]->TryPop(r_updateCmd))
				{
					m_scene->Apply(r_updateCmd);
				}
			}

			for (auto& [type, proxies] : m_scene->m_proxies)
			{
				for (auto& [id, proxy] : proxies)
				{
					if (proxy.addDirtyFlags[r_currentResourceIndex])
					{
						//std::cout << "Add In RenderThread - " << id << ", Index : " << r_currentResourceIndex << '\n';
						r_currentFrameResource->AddLocalConstantBuffer(id, proxy, type);
						proxy.addDirtyFlags[r_currentResourceIndex] = false;
						r_idToName[id] = proxy.name;
						r_nameToId[proxy.name] = id;
						r_idMax = max(r_idMax, id);
					}
					if (proxy.updateDirtyFlags[r_currentResourceIndex])
					{
						//std::cout << "Add In RenderThread - " << id << ", Index : " << r_currentResourceIndex << '\n';
						r_currentFrameResource->UpdateLocalConstantBuffer(id, proxy, type);
						proxy.updateDirtyFlags[r_currentResourceIndex] = false;
						if (id == 0)
						{
							auto position = proxy.constant.model.Transpose().Translation();
							r_currentFrameResource->UpdateCubeGCView(position);
						}
					}
				}
			}
			//RenderWithText();
			//RenderWithCompute();
			DrawingWithMouse();
		}});

		return true;
}

bool RenderEngine::InitGUI(HWND wnd)
{
	IMGUI_CHECKVERSION();
	g_imguiCtx = ImGui::CreateContext();
	//ImGui::SetCurrentContext(m_imguiCtx);

	ImGuiIO& io = ImGui::GetIO();
	io.IniFilename = nullptr;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	// io.Fonts->TexID = (ImTextureID)m_guiFont->GetSpriteSheet().ptr;

	ImGui::StyleColorsLight();
	const char* fontPath = "Fonts/Hack-Regular.ttf";
	float fontSize = 15.0f;
	// 폰트 로드 
	io.Fonts->AddFontFromFileTTF(fontPath, fontSize);

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.NumDescriptors = 1;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	m_device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_guiFontHeap));

	// Setup Platform/Renderer backends
	ImGui_ImplWin32_Init(wnd);

	ImGui_ImplDX12_Init(m_device, m_swapChainBufferCount, Renderer::backBufferFormat,
		m_guiFontHeap.Get(),
		m_guiFontHeap->GetCPUDescriptorHandleForHeapStart(),
		m_guiFontHeap->GetGPUDescriptorHandleForHeapStart());

	guiPenColor[0] = 1.f;
	guiPenColor[1] = 1.f;
	guiPenColor[2] = 1.f;
	guiPenRadius = 15.f;

	guiLocalConstant = {};


	g_imguiReady.store(true, std::memory_order_release);
	return true;
}

void RenderEngine::RequestResize(int newWidth, int newHeight)
{
	m_width = newWidth;
	m_height = newHeight;
	{
		std::lock_guard<std::mutex> lock(g_mtx);
		resize = true;
	}
}

void RenderEngine::RequestCapture()
{
	std::lock_guard<std::mutex> lock(g_mtx);
	captureDirty = true;
}

void RenderEngine::RequestRunDL()
{
	std::lock_guard<std::mutex> lock(g_mtx);
	runDLDirty = true;
}
// BOOKMARK
void RenderEngine::OnResize()
{
	if (m_swapChain == nullptr) return;

	// swapchain 버퍼 리셋
	for (int i = 0; i < m_swapChainBufferCount; i++)
	{
		m_swapChainResources[i].Reset();
	}

	// swapchain 버퍼 크기 조정
	m_swapChain->ResizeBuffers(m_swapChainBufferCount,
		m_width,
		m_height,
		DXGI_FORMAT_UNKNOWN,
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);

	// 버퍼에 대한 RTV 재생성
	CD3DX12_CPU_DESCRIPTOR_HANDLE handle(m_swapChainRTVHeap->GetCPUDescriptorHandleForHeapStart());
	for (int i = 0; i < m_swapChainBufferCount; i++)
	{
		m_swapChain->GetBuffer(i, IID_PPV_ARGS(m_swapChainResources[i].ReleaseAndGetAddressOf()));
		m_device->CreateRenderTargetView(m_swapChainResources[i].Get(), nullptr, handle);

		handle.Offset(1, m_rtvDescriptorSize);
	}

	// DepthBuffer 재생성
	CreateMainDepthBuffer();
	CreateTextureBuffers();

	m_currentBackBufferIndex = m_swapChain->GetCurrentBackBufferIndex();

	m_viewport = CD3DX12_VIEWPORT((FLOAT)m_guiWidth, 0.F, (FLOAT)(m_width - m_guiWidth), (FLOAT)m_height);
	m_hdrViewport = CD3DX12_VIEWPORT((FLOAT)0.F, 0.F, (FLOAT)(m_width), (FLOAT)m_height);
	m_scissorRect = CD3DX12_RECT((LONG)0, 0, (LONG)(m_width), (LONG)m_height);

}
// BOOKMARK
void RenderEngine::RegisterPrimitive(PrimitiveComponent* primitive)
{
	m_primitives.push_back(primitive);

	static uint32_t id = 0;

	CmdAddPrimitive add;
	if (StaticMeshComponent* staticMeshComp = dynamic_cast<StaticMeshComponent*>(primitive)) {
		add.mesh = staticMeshComp->GetMeshPtr();
		add.meshType = MT_primitive;
	}
	else if (PointCloudComponent* pointCloudComp = dynamic_cast<PointCloudComponent*>(primitive))
	{
		add.mesh = pointCloudComp->GetMeshPtr();
		add.meshType = MT_pointCloud;
	}
	else if (DotComponent* pointCloudComp = dynamic_cast<DotComponent*>(primitive))
	{
		add.mesh = pointCloudComp->GetMeshPtr();
		add.meshType = MT_finalize;
	}
	else if (CollisionComponent* collisionComp = dynamic_cast<CollisionComponent*>(primitive))
	{
		add.mesh = collisionComp->GetMeshPtr();
		add.meshType = MT_collision;
	}
	else if (CubeMapComponent* cubeMapComp = dynamic_cast<CubeMapComponent*>(primitive))
	{
		add.mesh = cubeMapComp->GetMeshPtr();
		add.meshType = MT_cubeMap;
	}
	else if (LightComponent* lightComp = dynamic_cast<LightComponent*>(primitive))
	{
		add.mesh = lightComp->GetMeshPtr();
		add.meshType = MT_primitive;
		m_lightInfos.push_back(lightComp->GetLightInfo());
	}
	else if (SkinnedMeshComponent* skinnedComp = dynamic_cast<SkinnedMeshComponent*>(primitive))
	{
		CmdAddSkinnedMesh skinnedAdd;
		skinnedAdd.name = primitive->GetName();
		skinnedAdd.id = id++;
		skinnedAdd.textureName = primitive->GetTextureName();
		skinnedAdd.psoName = primitive->GetPSOName();
		skinnedAdd.constant = primitive->GetLocalConstant();
		skinnedAdd.mesh = skinnedComp->GetMeshPtr();
		skinnedAdd.meshType = MT_skinnedMesh;
		skinnedAdd.skinnedlc = skinnedComp->GetSkinnedLocalConstant();
		m_renderAddCmdQueue[skinnedAdd.id]->Push(std::move(skinnedAdd));
		return;
	}
	add.name = primitive->GetName();
	add.id = id++;
	add.textureName = primitive->GetTextureName();
	add.psoName = primitive->GetPSOName();
	add.constant = primitive->GetLocalConstant();
	m_renderAddCmdQueue[add.id]->Push(std::move(add));
}

void RenderEngine::CreateCommandObjects()
{
	ThrowIfFailed(
		m_device->CreateCommandAllocator(
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(&m_commandAllocator)
		));

	ThrowIfFailed(
		m_device->CreateCommandList(
			0,
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			m_commandAllocator.Get(),
			nullptr,
			IID_PPV_ARGS(&m_commandList)
		));

	D3D12_COMMAND_QUEUE_DESC queueDesc;
	ZeroMemory(&queueDesc, sizeof(queueDesc));
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	ThrowIfFailed(
		m_device->CreateCommandQueue(
			&queueDesc,
			IID_PPV_ARGS(&m_commandQueue))
	);

	m_commandList->Close();
}

void RenderEngine::CreateSwapChain(IDXGIFactory7* factory, HWND wnd)
{
	ComPtr<IDXGISwapChain1> swapChain;

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = m_swapChainBufferCount;
	swapChainDesc.Width = m_width;
	swapChainDesc.Height = m_height;
	swapChainDesc.Format = Renderer::backBufferFormat;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.SampleDesc.Count = 1;

	ThrowIfFailed(factory->CreateSwapChainForHwnd(
		m_commandQueue.Get(),
		wnd,
		&swapChainDesc,
		nullptr,
		nullptr,
		&swapChain
	));

	ThrowIfFailed(swapChain.As(&m_swapChain));

	m_currentBackBufferIndex = m_swapChain->GetCurrentBackBufferIndex();
}

void RenderEngine::CreateMainDepthBuffer()
{
	utility->CreateTextureBuffer(m_depthStencilBuffer, m_width, m_height,
		Renderer::dsBufferFormat, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL,
		D3D12_RESOURCE_STATE_DEPTH_WRITE, 0, L"mainDepthBuffer");
	CD3DX12_CPU_DESCRIPTOR_HANDLE handle(m_DSVHeap->GetCPUDescriptorHandleForHeapStart());

	utility->CreateResourceView(m_depthStencilBuffer, Renderer::dsBufferFormat, false, handle, DescriptorType::DSV);
}

void RenderEngine::CreateDepthBuffers()
{
	CreateMainDepthBuffer();

	utility->CreateTextureBuffer(m_depthOnlyBuffer, depthOnlyWidth, depthOnlyHeight,
		Renderer::dsOnlyFormat, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL,
		D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE, 0, L"depthOnyBuffer");

	CD3DX12_CPU_DESCRIPTOR_HANDLE handle(m_DSVHeap->GetCPUDescriptorHandleForHeapStart(), 1, m_dsvDescriptorSize);
	utility->CreateResourceView(m_depthOnlyBuffer, Renderer::dsOnlyDsvFormat, false, handle, DescriptorType::DSV);
	//utility->CreateResourceView(m_depthOnlyBuffer, Renderer::dsOnlySrvFormat, false, handle, DescriptorType::SRV);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = Renderer::dsOnlySrvFormat;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = m_depthOnlyBuffer->GetDesc().MipLevels;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.f;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;

	m_textureLoader->AddTexture(m_depthOnlyBuffer, srvDesc, depthOnlyTextureName, false);

	m_depthOnlyViewport = CD3DX12_VIEWPORT((FLOAT)0.F, 0.F, (FLOAT)(depthOnlyWidth), (FLOAT)depthOnlyHeight);
	m_depthOnlyRect = CD3DX12_RECT((LONG)0, 0, (LONG)(depthOnlyWidth), (LONG)depthOnlyHeight);
}

void RenderEngine::CreateTextureBuffers()
{
	// Compute Shader에서 사용할 버퍼 생성
	{
		computeTextureDIMX = m_width - m_guiWidth;
		computeTextureDIMY = m_height;
		D3D12_RESOURCE_FLAGS flag = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS | D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		utility->CreateTextureBuffer(m_computeBuffer, computeTextureDIMX, computeTextureDIMY, m_computeBufferFormat, flag, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, 0, L"computeBuffer");

		utility->CreateResourceView(m_computeBuffer, m_computeBufferFormat, false, m_UAVHeap->GetCPUDescriptorHandleForHeapStart(), DescriptorType::UAV);
		utility->CreateResourceView(m_computeBuffer, m_computeBufferFormat, false, m_UAVCPUHeap->GetCPUDescriptorHandleForHeapStart(), DescriptorType::UAV);
		m_textureLoader->AddTexture(m_computeBuffer, m_computeTextureName);

	}

	// HDR 버퍼 생성
	{
		D3D12_RESOURCE_FLAGS hdrFlag = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		utility->CreateTextureBuffer(m_hdrBuffer, m_width, m_height, Renderer::hdrFormat, hdrFlag, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, 0, L"hdrBuffer");
		utility->CreateResourceView(m_hdrBuffer, Renderer::hdrFormat, false, m_hdrRtvHeap->GetCPUDescriptorHandleForHeapStart(), DescriptorType::RTV);
		m_textureLoader->AddTexture(m_hdrBuffer, m_hdrTextureName);
	}
}

// BOOKMARK
void RenderEngine::UpdateGUI()
{
	ImGui::SetWindowSize(ImVec2((float)m_guiWidth, (float)m_height));
	ImGui::SetWindowPos(ImVec2(0.f, 0.f), ImGuiCond_FirstUseEver);

	auto it = m_scene->m_proxies[MeshType::MT_primitive].find(r_selecteId);
	if (it != m_scene->m_proxies[MeshType::MT_primitive].end())
	{
		PrimitiveProxy proxy = it->second;
		guiLocalConstant = proxy.constant;
	}
	Vector3 loc = guiLocalConstant.model.Transpose().Translation();
	gui_location = { loc.x, loc.y, loc.z };

	int fps = int(1 / r_packet.deltaTime);
	std::string fpsStr = "FPS : " + std::to_string(fps);
	ImGui::Text(fpsStr.c_str());
	Vector3 dir = r_currentFrameResource->GetGlobalConstant().cameraDir;
	//std::string str = "x : " + std::to_string(dir.x) + "y : " + std::to_string(dir.y) + "z : " + std::to_string(dir.z);
	//ImGui::Text(str.c_str());

	ImGui::Checkbox("Change Mode", &test);
	ImGui::Checkbox("Render PointCloud", &gui_renderPointCloud);
	if (ImGui::Checkbox("WireFrame Mode", &gui_wireFrameMode))
	{
		if (gui_wireFrameMode)
		{
			currentPbrPSO = wirePbrPSO;
		}
		else
			currentPbrPSO = pbrPSO;
	}

	ImGui::NewLine();
	ImGui::PushItemWidth(m_guiWidth - 30.f);

	ImGui::Text("Pen Radius");
	ImGui::SliderFloat("##Pen Radius", &guiPenRadius, 0.f, 20.f);

	/*ImGui::TextUnformatted("save miplevel");
	ImGui::SliderInt("save miplevel", &saveMipLevel, 0, 5);*/
	ImGui::SeparatorText("Local Constant");

	ImGui::Text(r_idToName[r_selecteId].c_str());
	ImGui::SliderInt("##id", &r_selecteId, 0, r_idMax);
	ImGui::Text("position");
	if (ImGui::DragFloat3("##loc", gui_location.data(), 0.1f, -100.f, 100.f))
	{
		guiLocalConstant.model = guiLocalConstant.model.Transpose();

		guiLocalConstant.model.m[3][0] = gui_location[0];
		guiLocalConstant.model.m[3][1] = gui_location[1];
		guiLocalConstant.model.m[3][2] = gui_location[2];

		guiLocalConstant.modelInvTranspose = guiLocalConstant.model.Invert();
		guiLocalConstant.model = guiLocalConstant.model.Transpose();

		CmdUpdateActorConstant cmd;
		cmd.id = r_selecteId;
		cmd.lc = guiLocalConstant;
		m_renderToMainCmdQueue->Push(std::move(cmd));
	}
	ImGui::Text("Height Scale");
	if (ImGui::SliderFloat("##Height Scale", &guiLocalConstant.heightScale, 0.f, 1.f))
	{
		CmdUpdateActorConstant cmd;
		cmd.id = r_selecteId;
		cmd.lc = guiLocalConstant;
		m_renderToMainCmdQueue->Push(std::move(cmd));
	}
	ImGui::Text("Roughness");
	if (ImGui::SliderFloat("##Roughness", &guiLocalConstant.roughness, 0, 1))
	{
		CmdUpdateActorConstant cmd;
		cmd.id = r_selecteId;
		cmd.lc = guiLocalConstant;
		m_renderToMainCmdQueue->Push(std::move(cmd));
	}
	ImGui::Text("Matallic");
	if (ImGui::SliderFloat("##Matallic", &guiLocalConstant.metallic, 0.f, 1.f))
	{
		CmdUpdateActorConstant cmd;
		cmd.id = r_selecteId;
		cmd.lc = guiLocalConstant;
		m_renderToMainCmdQueue->Push(std::move(cmd));
	}

	ImGui::PopItemWidth();
	if (ImGui::Button("Spawn Actor")) {
		CmdAddActor cmd;
		cmd.name = "test";
		m_renderToMainCmdQueue->Push(std::move(cmd));

	}
	//if (ImGui::Button("Run DL")) {
	//	RequestRunDL();
	//	printRet = true;

	//}
	//if (printRet)
	//{
	//	//ImGui::SameLine();
	//	int ret = dlRet.load(std::memory_order_acquire);
	//	std::string retStr = "DL prediction : ";
	//	retStr += std::to_string(ret);
	//	ImGui::Text(retStr.c_str());
	//}
	if (ImGui::Button("Save backbuffer")) {
		RequestCapture();
	}
	ImGui::SameLine();
	if (ImGui::Button("Clear")) {
		clearFlag = true;
	}

	//if (ImGui::Button("Open Picker"))
	//	ImGui::OpenPopup("PickerPopup");

	//if (ImGui::BeginPopup("PickerPopup"))
	//{
	//	// 큰 컬러 피커(휠/사각형)
	//	ImGui::ColorPicker3("##Picker", (float*)&guiPenColor,
	//		ImGuiColorEditFlags_Float |
	//		ImGuiColorEditFlags_AlphaBar |
	//		ImGuiColorEditFlags_DisplayRGB);

	//	ImGui::EndPopup();
	//}
}

void RenderEngine::BuildFrameResources()
{
	m_frameResources.resize(m_frameResourceCount);
	for (int i = 0; i < m_frameResourceCount; i++)
	{
		m_frameResources[i] = std::make_shared<FrameResource>();
		if (world)
		{
			m_frameResources[i]->Initialize(m_device, 512, 512, 0, mainWnd, m_lightInfos);
		}
	}
}

void RenderEngine::AddTextProxy(SceneComponent* component)
{
	//if (StaticMeshComponent* staticMeshComp = dynamic_cast<StaticMeshComponent*>(component))
	//{
	//	//std::cout << "Mesh exist\n";
	//	TextProxy proxy;
	//	proxy.mesh = staticMeshComp->GetMesh();
	//	currentFrameResource->textProxyBuffer.push_back(proxy);
	//}

	//std::vector<std::shared_ptr<SceneComponent>> child;
	//component->GetChildrenComponents(child);
	//for (size_t i = 0; i < child.size(); i++)
	//{
	//	AddTextProxy(child[i].get());
	//}
}

void  RenderEngine::UpdateMousePosition()
{
	GetCursorPos(&currMousPt);
	ScreenToClient(mainWnd, &currMousPt);
	pMouseinputStateHelper->UpdateMousePos(currMousPt.x, currMousPt.y);
	pMouseinputStateHelper->UpdatePrevMousePos(prevMousePt.x, prevMousePt.y);
}

void RenderEngine::UpdateGlobalConstantBuffer(const ViewProjInfo& viewProjInfo)
{
	packet.gc.cameraDir = ToVector4(viewProjInfo.viewDirection, 0.f);
	packet.gc.cameraPos = ToVector4(viewProjInfo.viewLocation, 0.f);
	packet.gc.view = viewProjInfo.view.Transpose();
	packet.gc.proj = viewProjInfo.proj.Transpose();

	for (size_t i = 0; i < m_lightInfos.size(); i++)
	{
		packet.gc.lights[i] = *(m_lightInfos[i].get());
	}
}

void RenderEngine::UpdatePBGlobalConstantBuffer(const int& guiWidth, const MouseInputState& mouseInputState)
{
	packet.pbgc.mouseX = float(mouseInputState.mouseX - guiWidth);
	packet.pbgc.mouseY = float(mouseInputState.mouseY);
	packet.pbgc.prevMouseX = float(mouseInputState.prevMouseX - guiWidth);
	packet.pbgc.prevMouseY = float(mouseInputState.prevMouseY);
	packet.pbgc.lMouseClickDown = mouseInputState.lmbDown ? 1 : 0;

	packet.pbgc.penColor = Vector3(guiPenColor);
	packet.pbgc.penRadius = guiPenRadius;
}
// BOOKMARK
void RenderEngine::Tick(float deltaTime)
{
	// drain render input
	while (m_renderToMainCmdQueue->TryPop(g_cmd))
	{
		ApplyGameCommand(g_cmd);
	}

	// Global Constants
	if (world)
	{
		packet.frameId = m_frameId++;
		packet.deltaTime = deltaTime;
		UpdateGlobalConstantBuffer(world->GetViewProjInfo());
		{
			UpdateMousePosition();
			UpdatePBGlobalConstantBuffer(m_guiWidth, pMouseinputStateHelper->GetInputState());
			prevMousePt = currMousPt;
		}

		m_frameQueue->Push(std::move(packet));
	}

	// Local Constants
	int id = m_frameId % m_frameResourceCount;

	for (int i = 0; i < m_primitives.size(); i++)
	{
		PrimitiveComponent* prim = m_primitives[i];
		if (prim->IsUpdateConstant())
		{
			MeshType mt = GetMeshType(prim);
			if (mt == MT_skinnedMesh)
			{
				std::string name = prim->GetName();
				CmdUpdateSkinnedMesh update;
				update.id = i;
				update.constant = prim->GetLocalConstant();
				if (SkinnedMeshComponent* skinnedComp = dynamic_cast<SkinnedMeshComponent*>(prim))
				{
					update.skinnedlc = skinnedComp->GetSkinnedLocalConstant();
				}
				update.meshType = mt;
				m_renderUpdateCmdQueue[i]->Push(std::move(update));
			}
			else
			{
				std::string name = prim->GetName();
				CmdUpdatePrimitive update;
				update.id = i;
				update.constant = prim->GetLocalConstant();
				update.meshType = mt;
				m_renderUpdateCmdQueue[i]->Push(std::move(update));
			}
		}
		{
			std::lock_guard<std::mutex> lock(g_mtx);
			frameReady = true;
		}
		// render thread 호출
		cv.notify_one();

	}
}

void RenderEngine::RenderMeshes(const std::string& psoName, ID3D12GraphicsCommandList* commandList, MeshType meshType, RenderPassType rpType, int gcbIdx)
{
	ID3D12DescriptorHeap* heaps[] = {
		m_textureLoader->GetHeap()
	};
	commandList->SetDescriptorHeaps(1, heaps);

	for (auto& [type, proxies] : m_scene->m_proxies)
	{
		if (type == meshType)
		{
			for (auto& [id, proxy] : proxies)
			{
				auto it = r_currentFrameResource->m_localData.find(id);
				auto& ld = it->second;

				// cubemap - 0, albedo - 1, player cubemap - 2, local - 3, gobal - 4
				// phong 과 pbr 구분 목적 psoName검사
				if (type == MT_primitive && (psoName.find(ld.psoName) != std::string::npos))
				{
					if (rpType == RenderPassType::RPT_Default)
					{
						commandList->SetGraphicsRootDescriptorTable(0, m_textureLoader->GetGPUHandle(m_cubeMapTextureName));
						commandList->SetGraphicsRootDescriptorTable(1, m_textureLoader->GetGPUHandle(ld.textureName));
						commandList->SetGraphicsRootDescriptorTable(2, m_textureLoader->GetGPUHandle(playerCubeMapTextureName));
						commandList->SetGraphicsRootDescriptorTable(3, m_textureLoader->GetGPUHandle(depthOnlyTextureName));
						commandList->SetGraphicsRootConstantBufferView(4, ld.localCB->GetGPUVirtualAddress());
						commandList->SetGraphicsRootConstantBufferView(5, r_currentFrameResource->GetGCBGPUAddress());
					}
					else if (rpType == RenderPassType::RPT_CubeMapPass)
					{
						/*commandList->SetGraphicsRootDescriptorTable(0, m_textureLoader->GetGPUHandle(m_cubeMapTextureName));
						commandList->SetGraphicsRootDescriptorTable(1, m_textureLoader->GetGPUHandle(ld.textureName));
						commandList->SetGraphicsRootDescriptorTable(3, m_textureLoader->GetGPUHandle(depthOnlyTextureName));
						commandList->SetGraphicsRootConstantBufferView(4, ld.localCB->GetGPUVirtualAddress());
						commandList->SetGraphicsRootConstantBufferView(5, r_currentFrameResource->GetCubeGCBGPUAddress(gcbIdx));*/

						commandList->SetGraphicsRootDescriptorTable(0, m_textureLoader->GetGPUHandle(m_cubeMapTextureName));
						commandList->SetGraphicsRootDescriptorTable(1, m_textureLoader->GetGPUHandle(ld.textureName));
						commandList->SetGraphicsRootConstantBufferView(2, ld.localCB->GetGPUVirtualAddress());
						commandList->SetGraphicsRootConstantBufferView(3, r_currentFrameResource->GetCubeGCBGPUAddress(gcbIdx));
					}
					else if (rpType == RenderPassType::RPT_DepthOnlyPass)
					{
						commandList->SetGraphicsRootDescriptorTable(0, m_textureLoader->GetGPUHandle(ld.textureName));
						commandList->SetGraphicsRootConstantBufferView(1, ld.localCB->GetGPUVirtualAddress());
						commandList->SetGraphicsRootConstantBufferView(2, r_currentFrameResource->GetLightGCBGPUAddress(gcbIdx));
					}
					proxy.mesh->Render_(commandList);
				}
				else if (type == MT_pointCloud)
				{
					if (rpType == RenderPassType::RPT_Default)
					{
						commandList->SetGraphicsRootConstantBufferView(0, ld.localCB->GetGPUVirtualAddress());
						commandList->SetGraphicsRootConstantBufferView(1, r_currentFrameResource->GetGCBGPUAddress());
					}
					else if (rpType == RenderPassType::RPT_CubeMapPass)
					{
						commandList->SetGraphicsRootConstantBufferView(0, ld.localCB->GetGPUVirtualAddress());
						commandList->SetGraphicsRootConstantBufferView(1, r_currentFrameResource->GetCubeGCBGPUAddress(gcbIdx));
					}
					proxy.mesh->RenderPoints(commandList);

				}
				else if (type == MT_finalize)
				{
					commandList->SetGraphicsRootDescriptorTable(0, m_textureLoader->GetGPUHandle(ld.textureName));
					proxy.mesh->RenderDot(commandList);
				}
				else if (type == MT_cubeMap)
				{
					if (rpType == RenderPassType::RPT_Default)
					{
						commandList->SetGraphicsRootDescriptorTable(0, m_textureLoader->GetGPUHandle(ld.textureName));
						commandList->SetGraphicsRootConstantBufferView(1, r_currentFrameResource->GetGCBGPUAddress());
					}
					else if (rpType == RenderPassType::RPT_CubeMapPass)
					{
						commandList->SetGraphicsRootDescriptorTable(0, m_textureLoader->GetGPUHandle(ld.textureName));
						commandList->SetGraphicsRootConstantBufferView(1, r_currentFrameResource->GetCubeGCBGPUAddress(gcbIdx));
					}
					proxy.mesh->CubeMapRender(commandList);
				}
				else if (type == MT_skinnedMesh)
				{
					if (rpType == RenderPassType::RPT_Default) {
						commandList->SetGraphicsRootDescriptorTable(0, m_textureLoader->GetGPUHandle(m_cubeMapTextureName));
						commandList->SetGraphicsRootDescriptorTable(1, m_textureLoader->GetGPUHandle(ld.textureName));
						commandList->SetGraphicsRootDescriptorTable(2, m_textureLoader->GetGPUHandle(playerCubeMapTextureName));
						commandList->SetGraphicsRootDescriptorTable(3, m_textureLoader->GetGPUHandle(depthOnlyTextureName));
						commandList->SetGraphicsRootConstantBufferView(4, ld.localCB->GetGPUVirtualAddress());
						commandList->SetGraphicsRootConstantBufferView(5, r_currentFrameResource->GetGCBGPUAddress());
						commandList->SetGraphicsRootConstantBufferView(6, ld.skinnedLCB->GetGPUVirtualAddress());
					}
					else if (rpType == RenderPassType::RPT_DepthOnlyPass)
					{
						commandList->SetGraphicsRootDescriptorTable(0, m_textureLoader->GetGPUHandle(ld.textureName));
						commandList->SetGraphicsRootConstantBufferView(1, ld.localCB->GetGPUVirtualAddress());
						commandList->SetGraphicsRootConstantBufferView(2, r_currentFrameResource->GetLightGCBGPUAddress(gcbIdx));
						commandList->SetGraphicsRootConstantBufferView(3, ld.skinnedLCB->GetGPUVirtualAddress());
					}
					proxy.mesh->Render_(commandList);
				}
				else if (type == MT_collision)
				{
					if (rpType == RenderPassType::RPT_Default) 
					{
						commandList->SetGraphicsRootConstantBufferView(0, ld.localCB->GetGPUVirtualAddress());
						commandList->SetGraphicsRootConstantBufferView(1, r_currentFrameResource->GetGCBGPUAddress());

					}
					proxy.mesh->RenderDot(commandList);

				}
			}

		}
	}
}

void RenderEngine::Render(const std::string& psoName, int idx, MeshType meshType, bool isFinal, bool clear)
{
	PIXBeginEvent(m_commandQueue.Get(), PIX_COLOR(255, 0, 0), psoName.c_str());

	using namespace Renderer;

	if (idx == 0)
	{
		if (r_currentFrameResource->m_currentFence != 0 &&
			m_fence->GetCompletedValue() < r_currentFrameResource->m_currentFence)
		{
			HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
			m_fence->SetEventOnCompletion(r_currentFrameResource->m_currentFence, eventHandle);

			WaitForSingleObject(eventHandle, INFINITE);
			CloseHandle(eventHandle);
		}
		r_currentFrameResource->m_currentFence = ++m_currentFence;
	}

	GraphicsPSO pso;
	if (m_PSOs.find(psoName) != m_PSOs.end())
	{
		pso = m_PSOs[psoName];
	}
	else
	{
		pso = m_PSOs["defaultPSO"];
	}
	ID3D12GraphicsCommandList* commandList = r_currentFrameResource->GetCommandList(idx);
	r_currentFrameResource->ResetAllocator(idx);
	ThrowIfFailed(commandList->Reset(r_currentFrameResource->GetAllocator(idx), pso.GetPSO()));
	commandList->SetPipelineState(pso.GetPSO());
	commandList->SetGraphicsRootSignature(pso.GetRootSignature()->GetSignature());


	commandList->RSSetScissorRects(1, &m_scissorRect);

	if (meshType == MT_finalize) {
		commandList->RSSetViewports(1, &m_viewport);

		commandList->ResourceBarrier(
			1,
			&CD3DX12_RESOURCE_BARRIER::Transition(
				GetCurrentSwapChainResource(),
				D3D12_RESOURCE_STATE_PRESENT,
				D3D12_RESOURCE_STATE_RENDER_TARGET
			));
		if (clear)
		{
			commandList->ClearRenderTargetView(GetCurrentRtvCpuHandle(), rtvClearColor.data(), 0, nullptr);
			commandList->ClearDepthStencilView(GetDSVCpuHandle(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.f, 0, 0, nullptr);
		}
		commandList->OMSetRenderTargets(1, &GetCurrentRtvCpuHandle(), TRUE, &GetDSVCpuHandle());
	}
	else
	{
		commandList->RSSetViewports(1, &m_hdrViewport);

		commandList->ResourceBarrier(
			1,
			&CD3DX12_RESOURCE_BARRIER::Transition(
				m_hdrBuffer.Get(),
				D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
				D3D12_RESOURCE_STATE_RENDER_TARGET
			));
		if (clear)
		{
			commandList->ClearRenderTargetView(m_hdrRtvHeap->GetCPUDescriptorHandleForHeapStart(), blackClearColor.data(), 0, nullptr);
			commandList->ClearDepthStencilView(GetDSVCpuHandle(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.f, 0, 0, nullptr);
		}
		commandList->OMSetRenderTargets(1, &m_hdrRtvHeap->GetCPUDescriptorHandleForHeapStart(), TRUE, &GetDSVCpuHandle());
	}

	RenderMeshes(psoName, commandList, meshType, RPT_Default);

	if (meshType == MT_finalize)
	{
		commandList->ResourceBarrier(
			1,
			&CD3DX12_RESOURCE_BARRIER::Transition(
				GetCurrentSwapChainResource(),
				D3D12_RESOURCE_STATE_RENDER_TARGET,
				D3D12_RESOURCE_STATE_PRESENT
			));
	}
	else
	{
		commandList->ResourceBarrier(
			1,
			&CD3DX12_RESOURCE_BARRIER::Transition(
				m_hdrBuffer.Get(),
				D3D12_RESOURCE_STATE_RENDER_TARGET,
				D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
			));
	}

	commandList->Close();

	ID3D12CommandList* commands[] = { commandList };
	{
		std::lock_guard<std::mutex> lock(queue_mtx);
		m_commandQueue->ExecuteCommandLists(ARRAYSIZE(commands), commands);

		if (isFinal)
		{
			ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), m_currentFence));
			// text 업데이트를 위해 graphcics memory 사용 시 commit 해줘야 Graphics 메모리를 재사용한다
			m_graphicsMemory->Commit(m_commandQueue.Get());
			ThrowIfFailed(m_swapChain->Present(1, 0));
			m_currentBackBufferIndex = (m_currentBackBufferIndex + 1) % m_swapChainBufferCount;
		}
	}
	PIXEndEvent(m_commandQueue.Get());
}

void RenderEngine::RenderGUI(bool isFinal)
{
	std::lock_guard<std::mutex> lock(Graphics::g_imguiMutex);
	ImGui::SetCurrentContext(g_imguiCtx);

	ImGui_ImplWin32_NewFrame();
	ImGui_ImplDX12_NewFrame();
	ImGui::NewFrame();

	ImGuiWindowFlags flags = 0;

	ImGui::Begin("GUI", nullptr, flags);
	UpdateGUI();

	ImGui::End();
	ImGui::Render();

	ID3D12CommandAllocator* commandAllocator = r_currentFrameResource->GetGUIAllocator();
	ID3D12GraphicsCommandList* commandList = r_currentFrameResource->GetGUICommandList();

	ThrowIfFailed(commandAllocator->Reset());
	ThrowIfFailed(commandList->Reset(commandAllocator, nullptr));

	commandList->ResourceBarrier(1,
		&CD3DX12_RESOURCE_BARRIER::Transition(
			GetCurrentSwapChainResource(),
			D3D12_RESOURCE_STATE_PRESENT,
			D3D12_RESOURCE_STATE_RENDER_TARGET
		));

	commandList->OMSetRenderTargets(1, &GetCurrentRtvCpuHandle(), false, nullptr);
	ID3D12DescriptorHeap* pHeaps[] = { m_guiFontHeap.Get() };
	commandList->SetDescriptorHeaps(static_cast<UINT>(std::size(pHeaps)), pHeaps);

	ImDrawData* dd = ImGui::GetDrawData();
	if (dd->Valid)
		ImGui_ImplDX12_RenderDrawData(dd, commandList);

	commandList->ResourceBarrier(1,
		&CD3DX12_RESOURCE_BARRIER::Transition(
			GetCurrentSwapChainResource(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PRESENT
		));

	commandList->Close();
	ID3D12CommandList* commands[] = { commandList };
	{
		std::lock_guard<std::mutex> lock(queue_mtx);
		m_commandQueue->ExecuteCommandLists(ARRAYSIZE(commands), commands);

		if (isFinal)
		{
			ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), m_currentFence));
			// text 업데이트를 위해 graphcics memory 사용 시 commit 해줘야 Graphics 메모리를 재사용한다
			m_graphicsMemory->Commit(m_commandQueue.Get());
			ThrowIfFailed(m_swapChain->Present(1, 0));
			m_currentBackBufferIndex = (m_currentBackBufferIndex + 1) % m_swapChainBufferCount;
		}
	}
}

// TODO : 사용할 리소스 지정하기
void RenderEngine::Compute(const std::string& cpsoName, int idx, bool isFinal, D3D12_RESOURCE_STATES prevState)
{
	using namespace Renderer;

	if (idx == 0)
	{
		if (r_currentFrameResource->m_currentFence != 0 &&
			m_fence->GetCompletedValue() < r_currentFrameResource->m_currentFence)
		{
			HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
			m_fence->SetEventOnCompletion(r_currentFrameResource->m_currentFence, eventHandle);

			WaitForSingleObject(eventHandle, INFINITE);
			CloseHandle(eventHandle);
		}
		r_currentFrameResource->m_currentFence = ++m_currentFence;
	}
	ComputePSO pso;
	if (m_CPSOs.find(cpsoName) != m_CPSOs.end())
	{
		pso = m_CPSOs[cpsoName];
	}
	else
	{
		pso = m_CPSOs["defaultCPSO"];
	}
	ID3D12GraphicsCommandList* commandList = r_currentFrameResource->GetCommandList(idx);
	r_currentFrameResource->ResetAllocator(idx);
	ThrowIfFailed(commandList->Reset(r_currentFrameResource->GetAllocator(idx), pso.GetPSO()));

	commandList->ResourceBarrier(
		1,
		&CD3DX12_RESOURCE_BARRIER::Transition(
			m_computeBuffer.Get(),
			prevState,
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS
		));

	commandList->SetComputeRootSignature(pso.GetRootSignature()->GetSignature());

	ID3D12DescriptorHeap* heaps[] = {
		m_UAVHeap.Get()
	};
	commandList->SetDescriptorHeaps(1, heaps);

	commandList->SetComputeRootDescriptorTable(0, m_UAVHeap->GetGPUDescriptorHandleForHeapStart());
	commandList->SetComputeRootConstantBufferView(1, r_currentFrameResource->GetPBGCBGPUAddress());
	commandList->Dispatch((UINT)ceil((computeTextureDIMX) / 32.f), (UINT)ceil(computeTextureDIMY / 32.f), 1);

	commandList->ResourceBarrier(
		1,
		&CD3DX12_RESOURCE_BARRIER::Transition(
			m_computeBuffer.Get(),
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
			prevState
		));

	commandList->Close();

	ID3D12CommandList* commands[] = { commandList };
	{
		std::lock_guard<std::mutex> lock(queue_mtx);
		m_commandQueue->ExecuteCommandLists(ARRAYSIZE(commands), commands);

		if (isFinal)
		{
			ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), m_currentFence));
			m_graphicsMemory->Commit(m_commandQueue.Get());
			ThrowIfFailed(m_swapChain->Present(1, 0));
			m_currentBackBufferIndex = (m_currentBackBufferIndex + 1) % m_swapChainBufferCount;
		}
	}
}

void RenderEngine::RenderCube(const std::string& psoName, const std::string& proxyPsoName, int idx, MeshType meshType, bool isFinal, bool clear)
{
	PIXBeginEvent(m_commandQueue.Get(), PIX_COLOR(255, 0, 0), psoName.c_str());

	using namespace Renderer;

	if (idx == 0)
	{
		if (r_currentFrameResource->m_currentFence != 0 &&
			m_fence->GetCompletedValue() < r_currentFrameResource->m_currentFence)
		{
			HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
			m_fence->SetEventOnCompletion(r_currentFrameResource->m_currentFence, eventHandle);

			WaitForSingleObject(eventHandle, INFINITE);
			CloseHandle(eventHandle);
		}
		r_currentFrameResource->m_currentFence = ++m_currentFence;
	}

	GraphicsPSO pso;
	if (m_PSOs.find(psoName) != m_PSOs.end())
	{
		pso = m_PSOs[psoName];
	}
	else
	{
		pso = m_PSOs["defaultPSO"];
	}
	ID3D12GraphicsCommandList* commandList = r_currentFrameResource->GetCommandList(idx);
	r_currentFrameResource->ResetAllocator(idx);
	ThrowIfFailed(commandList->Reset(r_currentFrameResource->GetAllocator(idx), pso.GetPSO()));

	commandList->SetPipelineState(pso.GetPSO());
	commandList->SetGraphicsRootSignature(pso.GetRootSignature()->GetSignature());

	commandList->RSSetScissorRects(1, &m_cubeScissorRect);
	commandList->RSSetViewports(1, &m_cubeViewport);


	commandList->ResourceBarrier(
		1,
		&CD3DX12_RESOURCE_BARRIER::Transition(
			m_cubeMap.Get(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_RENDER_TARGET
		));

	for (int i = 0; i < 6; i++)
	{
		if (clear)
		{
			commandList->ClearRenderTargetView(GetCubeMapRtvCpuHandle(i), cubeRtvClearColor.data(), 0, nullptr);
			commandList->ClearDepthStencilView(GetCubeMapDsvCpuHandle(i), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.f, 0, 0, nullptr);

		}
		commandList->OMSetRenderTargets(1, &GetCubeMapRtvCpuHandle(i), TRUE, &GetCubeMapDsvCpuHandle(i));

		RenderMeshes(proxyPsoName, commandList, meshType, RPT_CubeMapPass, i);
		//RenderMeshes(proxyPsoName, commandList, meshType, , i);

	}
	commandList->ResourceBarrier(
		1,
		&CD3DX12_RESOURCE_BARRIER::Transition(
			m_cubeMap.Get(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
		));

	commandList->Close();

	ID3D12CommandList* commands[] = { commandList };
	{
		std::lock_guard<std::mutex> lock(queue_mtx);
		m_commandQueue->ExecuteCommandLists(ARRAYSIZE(commands), commands);

		if (isFinal)
		{
			ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), m_currentFence));
			// text 업데이트를 위해 graphcics memory 사용 시 commit 해줘야 Graphics 메모리를 재사용한다
			m_graphicsMemory->Commit(m_commandQueue.Get());
			ThrowIfFailed(m_swapChain->Present(1, 0));
			m_currentBackBufferIndex = (m_currentBackBufferIndex + 1) % m_swapChainBufferCount;
		}
	}
	PIXEndEvent(m_commandQueue.Get());
}


void RenderEngine::DepthOnlyPass(const std::string& psoName, const std::string& proxyPsoName, int idx, MeshType meshType, bool isFinal, bool clear)
{
	PIXBeginEvent(m_commandQueue.Get(), PIX_COLOR(255, 0, 0), psoName.c_str());

	using namespace Renderer;

	if (idx == 0)
	{
		if (r_currentFrameResource->m_currentFence != 0 &&
			m_fence->GetCompletedValue() < r_currentFrameResource->m_currentFence)
		{
			HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
			m_fence->SetEventOnCompletion(r_currentFrameResource->m_currentFence, eventHandle);

			WaitForSingleObject(eventHandle, INFINITE);
			CloseHandle(eventHandle);
		}
		r_currentFrameResource->m_currentFence = ++m_currentFence;
	}

	GraphicsPSO pso;
	if (m_PSOs.find(psoName) != m_PSOs.end())
	{
		pso = m_PSOs[psoName];
	}
	else
	{
		pso = m_PSOs["defaultPSO"];
	}

	ID3D12GraphicsCommandList* commandList = r_currentFrameResource->GetCommandList(idx);
	r_currentFrameResource->ResetAllocator(idx);
	ThrowIfFailed(commandList->Reset(r_currentFrameResource->GetAllocator(idx), pso.GetPSO()));

	commandList->SetPipelineState(pso.GetPSO());
	commandList->SetGraphicsRootSignature(pso.GetRootSignature()->GetSignature());

	commandList->RSSetScissorRects(1, &m_depthOnlyRect);
	commandList->RSSetViewports(1, &m_depthOnlyViewport);

	commandList->ResourceBarrier(
		1,
		&CD3DX12_RESOURCE_BARRIER::Transition(
			m_depthOnlyBuffer.Get(),
			D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_DEPTH_WRITE
		));
	if (clear)
	{
		commandList->ClearDepthStencilView(GetDSVCpuHandle(1), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.f, 0, 0, nullptr);
	}
	commandList->OMSetRenderTargets(0, nullptr, TRUE, &GetDSVCpuHandle(1));
	RenderMeshes(proxyPsoName, commandList, meshType, RPT_DepthOnlyPass, 0);

	commandList->ResourceBarrier(
		1,
		&CD3DX12_RESOURCE_BARRIER::Transition(
			m_depthOnlyBuffer.Get(),
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE
		));

	commandList->Close();

	ID3D12CommandList* commands[] = { commandList };
	{
		std::lock_guard<std::mutex> lock(queue_mtx);
		m_commandQueue->ExecuteCommandLists(ARRAYSIZE(commands), commands);

		if (isFinal)
		{
			ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), m_currentFence));
			// text 업데이트를 위해 graphcics memory 사용 시 commit 해줘야 Graphics 메모리를 재사용한다
			m_graphicsMemory->Commit(m_commandQueue.Get());
			ThrowIfFailed(m_swapChain->Present(1, 0));
			m_currentBackBufferIndex = (m_currentBackBufferIndex + 1) % m_swapChainBufferCount;
		}
	}
	PIXEndEvent(m_commandQueue.Get());
}
// FrameResource의 텍스트 렌더용 텍스쳐 업데이트
void RenderEngine::UpdateTexts()
{
	/*GraphicsPSO pso = m_PSOs["defaultPSO"];
	ID3D12CommandAllocator* commandAllocator = r_currentFrameResource->GetTextAllocator();
	ID3D12GraphicsCommandList* commandList = r_currentFrameResource->GetTextCommandList();
	ID3D12DescriptorHeap* m_textRtvHeap = r_currentFrameResource->GetTextRtvHeap();
	commandAllocator->Reset();
	commandList->Reset(commandAllocator, pso.GetPSO());
	commandList->SetPipelineState(pso.GetPSO());
	commandList->SetGraphicsRootSignature(pso.GetRootSignature()->GetSignature());
	UINT i = 0;
	for (auto& tr : r_currentFrameResource->textResources)
	{
		UINT64 width = tr.textureWidth;
		UINT64 height = tr.textureHeight;
		D3D12_VIEWPORT viewport = CD3DX12_VIEWPORT(0.F, 0.F, (FLOAT)width, (FLOAT)height);
		D3D12_RECT scissorRect = CD3DX12_RECT(0, 0, (LONG)width, (LONG)height);
		commandList->RSSetScissorRects(1, &scissorRect);
		commandList->RSSetViewports(1, &viewport);

		commandList->ClearRenderTargetView(r_currentFrameResource->GetTextRrvCPUHandle(i), DirectX::Colors::White, 0, nullptr);
		commandList->OMSetRenderTargets(
			1,
			&r_currentFrameResource->GetTextRrvCPUHandle(i),
			FALSE,
			nullptr);

		ID3D12DescriptorHeap* heaps[] = { m_fontSrvHeap.Get() };
		commandList->SetDescriptorHeaps(static_cast<UINT>(std::size(heaps)), heaps);
		spriteBatch->Begin(commandList);

		spriteBatch->SetViewport(viewport);
		std::wstring output = r_currentFrameResource->textProxyBuffer[i].str;
		DirectX::SimpleMath::Vector2 textSize = font->MeasureString(output.c_str());

		// 중앙 정렬을 위해 origin을 텍스트의 중앙으로 설정
		DirectX::SimpleMath::Vector2 origin = textSize / 2.f;
		DirectX::SimpleMath::Vector2 m_fontPos = DirectX::SimpleMath::Vector2(width / 2.f, height / 2.f);

		DirectX::XMVECTORF32 color = DirectX::Colors::Black;

		font->DrawString(spriteBatch.get(), output.c_str(),
			m_fontPos, color, 0.f, origin, 1);

		spriteBatch->End();
		i++;
	}

	commandList->Close();
	ID3D12CommandList* commands[] = { commandList };
	{
		std::lock_guard<std::mutex> lock(queue_mtx);
		m_commandQueue->ExecuteCommandLists(ARRAYSIZE(commands), commands);
	}
	*/
}

void RenderEngine::CreateTextures()
{
	texturePath = "Build/";
	fallbackPath = "Build/Fallback/";

	DDSPath = "Textures/DDS/";
	fallbackDDSPath = "Textures/Fallback/";

	m_textureLoader = std::make_shared<TextureLoader>(texturePath, m_device);
	m_textureLoader->InitHeap(100);
	m_textureLoader->LoadIdx();
	m_textureLoader->LoadTextures(m_commandQueue);

	//m_fallbackLoader = std::make_shared<TextureLoader>(fallbackPath, m_device);
	//m_fallbackLoader->InitHeap(30);
	//m_fallbackLoader->LoadIdx();
}

void RenderEngine::CreateCubeMap()
{
	cubeMapPos = Vector3(-1.5f, 0.8f, -1.5f);

	D3D12_RESOURCE_DESC cubeRD = {};
	cubeRD.Width = cubeWidth;
	cubeRD.Height = cubeHeight;
	cubeRD.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	cubeRD.DepthOrArraySize = 6;
	cubeRD.MipLevels = 1;
	cubeRD.SampleDesc = { 1,0 };
	cubeRD.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	cubeRD.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	cubeRD.Format = Renderer::hdrFormat;

	D3D12_CLEAR_VALUE cv = {};
	cv.Color[0] = 0.f;
	cv.Color[1] = 0.f;
	cv.Color[2] = 0.f;
	cv.Color[3] = 1.f;
	cv.Format = Renderer::hdrFormat;

	ThrowIfFailed(m_device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES::CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&cubeRD,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		&cv,
		IID_PPV_ARGS(m_cubeMap.ReleaseAndGetAddressOf())
	));
	m_cubeMap->SetName(L"CubeMap");
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NumDescriptors = 6;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.NodeMask = 0;

	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeapDesc.NumDescriptors = 6;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.NodeMask = 0;

	ThrowIfFailed(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(m_cubeMapRtvHeap.ReleaseAndGetAddressOf())));
	ThrowIfFailed(m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(m_cubeMapDsvHeap.ReleaseAndGetAddressOf())));

	CD3DX12_CPU_DESCRIPTOR_HANDLE handle(m_cubeMapRtvHeap->GetCPUDescriptorHandleForHeapStart());
	CD3DX12_CPU_DESCRIPTOR_HANDLE dHandle(m_cubeMapDsvHeap->GetCPUDescriptorHandleForHeapStart());

	for (int i = 0; i < 6; i++)
	{
		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
		rtvDesc.Format = Renderer::hdrFormat;
		rtvDesc.Texture2DArray.ArraySize = 1;
		rtvDesc.Texture2DArray.FirstArraySlice = i;
		rtvDesc.Texture2DArray.MipSlice = 0;
		rtvDesc.Texture2DArray.PlaneSlice = 0;
		m_device->CreateRenderTargetView(m_cubeMap.Get(), &rtvDesc, handle);
		if (i != 5)
			handle.Offset(1, m_rtvDescriptorSize);

		D3D12_RESOURCE_DESC dDesc = {};
		dDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		dDesc.Format = Renderer::dsBufferFormat;
		dDesc.MipLevels = 0;
		dDesc.DepthOrArraySize = 1;
		dDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
		dDesc.Width = cubeWidth;
		dDesc.Height = cubeHeight;
		dDesc.SampleDesc = { 1,0 };
		D3D12_CLEAR_VALUE cValue = {};
		cValue.DepthStencil.Depth = 1.f;
		cValue.DepthStencil.Stencil = 0;
		cValue.Format = Renderer::dsBufferFormat;

		m_device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&dDesc,
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&cValue,
			IID_PPV_ARGS(m_cubeDepthBuffer[i].ReleaseAndGetAddressOf())
		);
		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Texture2D.MipSlice = 0;
		dsvDesc.Format = Renderer::dsBufferFormat;
		dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
		m_device->CreateDepthStencilView(m_cubeDepthBuffer[i].Get(), &dsvDesc, dHandle);

		if (i != 5)
			dHandle.Offset(1, m_dsvDescriptorSize);

	}
	m_cubeViewport = CD3DX12_VIEWPORT((FLOAT)0.F, 0.F, (FLOAT)(cubeWidth), (FLOAT)cubeHeight);
	m_cubeScissorRect = CD3DX12_RECT((LONG)0, 0, (LONG)(cubeWidth), (LONG)cubeHeight);
	cubeRtvClearColor = { 0.F, 0.F, 0.F, 1.F };

	m_textureLoader->AddTexture(m_cubeMap, playerCubeMapTextureName, true);
}

void RenderEngine::CreateFonts()
{
	m_graphicsMemory.reset();

	m_graphicsMemory = std::make_unique<DirectX::GraphicsMemory>(m_device);

	DirectX::ResourceUploadBatch upload(m_device);
	upload.Begin();

	DirectX::RenderTargetState rtStateForTextRT(backBufferFormat, dsBufferFormat);

	font = std::make_shared<DirectX::SpriteFont>(
		m_device, upload,
		L"Fonts/comic_64.spritefont",
		m_fontSrvHeap->GetCPUDescriptorHandleForHeapStart(),
		m_fontSrvHeap->GetGPUDescriptorHandleForHeapStart());

	DirectX::SpriteBatchPipelineStateDescription pd(rtStateForTextRT);
	spriteBatch = std::make_shared<DirectX::SpriteBatch>(m_device, upload, pd);

	auto fut = upload.End(m_commandQueue.Get());
	fut.wait();
}

void RenderEngine::RenderWithText()
{
	// 현재 Render함수는 호출 될 때마다 commandList Allcator를 reset하고 있기 때문에
	// 한 프레임에 두 번 호출 될 수 없다
	/*Render(renderPSO, 0, false, false, true);
	UpdateTexts();
	Render(textPSO, 1, true, true, false);*/
}

void RenderEngine::RenderWithCompute()
{
	Compute(computePSO, 0, false/*isFinal*/, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	//Render("phongPSO", 1/*sequence*/, MT_primitive, false/*isFinal*/, true/*clear RT*/);
	//Render("pointCloudPSO", 2, RT_PointCloud, true/*isFinal*/, false/*clear RT*/);

	//RenderGUI(true);
}
// BOOKMARK
void RenderEngine::DrawingWithMouse()
{
	int i = 0;
	// cubemap에 환경 cubemap, mesh 렌더링
	DepthOnlyPass(depthOnlyPbrPSO, pbrPSO, i++, MT_primitive, false/*isFinal*/, true/*clear RT*/);
	DepthOnlyPass(skinnedDepthOnlyPbrPSO, skinnedMeshPSO, i++, MT_skinnedMesh, false/*isFinal*/, false/*clear RT*/);

	//RenderCube(cubeMapPSO, phongPSO, i++, MT_cubeMap, false/*isFinal*/, true/*clear RT*/);
	//RenderCube(genCubeMapPSO, phongPSO, i++, MT_primitive, false/*isFinal*/, false/*clear RT*/);
	//RenderCube(genPBRCubeMapPSO, pbrPSO, i++, MT_primitive, false/*isFinal*/, false/*clear RT*/);
	//if(gui_renderPointCloud)
	//	RenderCube(pointCloudPSO, pointCloudPSO, i++, MT_pointCloud, false/*isFinal*/, false/*clear RT*/);
	//Compute(computePSO, i++, false/*isFinal*/, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	//Render(phongPSO, i++/*sequence*/, MT_primitive, false/*isFinal*/, true/*clear RT*/);
	Render(currentPbrPSO, i++/*sequence*/, MT_primitive, false/*isFinal*/, true/*clear RT*/);
	Render(skinnedMeshPSO, i++/*sequence*/, MT_skinnedMesh, false/*isFinal*/, false/*clear RT*/);
	Render(collisionPSO, i++/*sequence*/, MT_collision, false/*isFinal*/, false/*clear RT*/);
	//if (gui_renderPointCloud)
	//	Render(pointCloudPSO, i++, MT_pointCloud, false/*isFinal*/, false/*clear RT*/);
	Render(cubeMapPSO, i++, MT_cubeMap, false/*isFinal*/, false/*clear RT*/);

	bool renderGUI = true;
	Render("renderTexturePSO", i++, MT_finalize, !renderGUI/*isFinal*/, true/*clear RT*/);
	if (renderGUI)
		RenderGUI(true);
}

D3D12_CPU_DESCRIPTOR_HANDLE RenderEngine::GetCurrentRtvCpuHandle() const
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_swapChainRTVHeap->GetCPUDescriptorHandleForHeapStart(), m_currentBackBufferIndex, m_rtvDescriptorSize);
}

D3D12_CPU_DESCRIPTOR_HANDLE RenderEngine::GetCubeMapRtvCpuHandle(int i) const
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_cubeMapRtvHeap->GetCPUDescriptorHandleForHeapStart(), i, m_rtvDescriptorSize);
}

D3D12_CPU_DESCRIPTOR_HANDLE RenderEngine::GetCubeMapDsvCpuHandle(int i) const
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_cubeMapDsvHeap->GetCPUDescriptorHandleForHeapStart(), i, m_dsvDescriptorSize);
}

D3D12_CPU_DESCRIPTOR_HANDLE RenderEngine::GetDSVCpuHandle(int idx) const
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_DSVHeap->GetCPUDescriptorHandleForHeapStart(), idx, m_dsvDescriptorSize);
}

ID3D12Resource* RenderEngine::GetCurrentSwapChainResource() const
{
	return m_swapChainResources[m_currentBackBufferIndex].Get();
}

void RenderEngine::FlushCommands()
{
	m_currentBufferFence++;

	ThrowIfFailed(m_commandQueue->Signal(m_createBufferfence.Get(), m_currentBufferFence));

	if (m_createBufferfence->GetCompletedValue() < m_currentBufferFence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
		m_createBufferfence->SetEventOnCompletion(m_currentBufferFence, eventHandle);

		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
}

void RenderEngine::FlushResourceCommands()
{
	m_currentFence++;

	ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), m_currentFence));
	if (m_fence->GetCompletedValue() < m_currentFence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
		m_fence->SetEventOnCompletion(m_currentFence, eventHandle);

		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
}

void RenderEngine::SaveTextureGPU(const std::string& name, D3D12_RESOURCE_STATES state, UINT16 miplevel, bool saveCPU)
{
	std::cout << "SaveTextureGPU\n";
	ID3D12Resource* t;

	if (name == "BackBuffer")
	{
		t = GetCurrentSwapChainResource();
	}
	else
		t = m_textureLoader->GetTexture(name);

	D3D12_RESOURCE_DESC desc = t->GetDesc();

	UINT subresource = 0;

	if (miplevel != 0)
	{
		GenerateMips(t);
		UINT16 mipLevels = desc.MipLevels;

		subresource = D3D12CalcSubresource(
			/*MipSlice*/ miplevel,
			/*ArraySlice*/ 0,
			/*PlaneSlice*/ 0,
			/*MipLevels*/ mipLevels,
			/*ArraySize*/ 1
		);
	}

	UINT numRows = 0;
	UINT64 rowSize = 0;
	UINT64 requiredSize = 0;

	D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint;
	m_device->GetCopyableFootprints(&desc, subresource, 1, 0, &footprint, &numRows, &rowSize, &requiredSize);

	m_device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(requiredSize),
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(m_saveBuffer.ReleaseAndGetAddressOf()));

	m_commandAllocator->Reset();
	m_commandList->Reset(m_commandAllocator.Get(), nullptr);

	imageInfo.rowPitch = footprint.Footprint.RowPitch;
	imageInfo.rowSize = rowSize;
	imageInfo.numRows = numRows;
	imageInfo.name = name;
	imageInfo.width = footprint.Footprint.Width;
	imageInfo.height = footprint.Footprint.Height;
	imageInfo.bufferPixelCount = requiredSize;
	imageInfo.format = desc.Format;

	m_commandList->ResourceBarrier(1,
		&CD3DX12_RESOURCE_BARRIER::Transition(
			t,
			state,
			D3D12_RESOURCE_STATE_COPY_SOURCE));

	CD3DX12_TEXTURE_COPY_LOCATION dst(m_saveBuffer.Get(), footprint);
	D3D12_TEXTURE_COPY_LOCATION src{};
	src.pResource = t;
	src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	src.SubresourceIndex = subresource;

	m_commandList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);

	m_commandList->ResourceBarrier(1,
		&CD3DX12_RESOURCE_BARRIER::Transition(
			t,
			D3D12_RESOURCE_STATE_COPY_SOURCE,
			state
		));

	m_commandList->Close();
	ID3D12CommandList* commands[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(ARRAYSIZE(commands), commands);

	if (saveCPU)
	{
		std::lock_guard<std::mutex> lock(capture_mtx);
		saveReady = true;
	}
	else
	{
		std::lock_guard<std::mutex> lock(capture_mtx);
		runDLReady = true;
	}
	captureCv.notify_one();
}

void RenderEngine::SaveTextureCPU()
{
	std::cout << "SaveTextureCPU\n";

	CD3DX12_RANGE range(0, 0);

	uint32_t pixelSize = (uint32_t)(imageInfo.rowSize * imageInfo.numRows);
	uint32_t pixelCount = pixelSize;
	if (imageInfo.format == DXGI_FORMAT_R16G16B16A16_FLOAT)
	{
		pixelCount /= 2;
	}
	std::vector<uint16_t> imagef16(pixelCount);
	std::vector<uint8_t> image(pixelCount);

	if (imageInfo.format == DXGI_FORMAT_R16G16B16A16_FLOAT)
	{
		uint16_t* mapped = nullptr;
		m_saveBuffer->Map(0, &range, reinterpret_cast<void**>(&mapped));
		/*for (UINT y = 0; y < imageInfo.numRows / 2; y++) {
			memcpy(imagef16.data() + y * imageInfo.rowSize,
				mapped + y * imageInfo.rowPitch,
				imageInfo.rowSize);
		}*/
		memcpy(imagef16.data(), mapped, imagef16.size() * 2);
		for (size_t i = 0; i < image.size(); i++)
		{
			double c = std::clamp(fp16_ieee_to_fp32_value(imagef16[i]), 0.f, 1.f);
			if ((i + 1) % 4 != 0)
			{
				c = std::pow(c, 1 / 2.2);
			}
			image[i] = std::clamp((int)(c * 255.f), 0, 255);
		}
	}
	else
	{
		uint8_t* mapped = nullptr;
		if (m_saveBuffer == nullptr)
		{
			return;
		}
		m_saveBuffer->Map(0, &range, reinterpret_cast<void**>(&mapped));
		for (UINT y = 0; y < imageInfo.numRows; y++) {
			memcpy(image.data() + y * imageInfo.rowSize,
				mapped + y * imageInfo.rowPitch,
				imageInfo.rowSize);
		}
	}
	//memcpy(image.data(), pSaveBuffer, copyBufferSize);

	m_saveBuffer->Unmap(0, nullptr);

	std::string filename = imageInfo.name + utility->MakeTimestamp() + ".png";
	std::string fileFullPath = imageFilePath + filename;
	if (imageInfo.format == DXGI_FORMAT_R16G16B16A16_FLOAT)
		stbi_write_png(fileFullPath.c_str(), (int)imageInfo.width, (int)imageInfo.height, 4, image.data(), (int)(imageInfo.rowSize / 2));
	else
		stbi_write_png(fileFullPath.c_str(), (int)imageInfo.width, (int)imageInfo.height, 4, image.data(), (int)imageInfo.rowSize);
	std::cout << imageInfo.name << " 가 성공적으로 저장되었습니다.\n";

	std::string cmd = "start \"\" \"" + imageFilePath + "\"";
	system(cmd.c_str());
}

void RenderEngine::RunDLModel()
{
	std::cout << "RunDLModel\n";

	CD3DX12_RANGE range(0, 0);
	std::vector<uint8_t> image(imageInfo.rowSize * imageInfo.numRows);
	uint8_t* mapped = nullptr;
	m_saveBuffer->Map(0, &range, reinterpret_cast<void**>(&mapped));

	for (UINT y = 0; y < imageInfo.numRows; y++) {
		memcpy(image.data() + y * imageInfo.rowSize,
			mapped + y * imageInfo.rowPitch,
			imageInfo.rowSize);
	}
	m_saveBuffer->Unmap(0, nullptr);

	//int ret = dlModel->Run(image);
	//std::cout << "Result : " << ret << '\n';
	//dlRet.store(ret, std::memory_order_release);
}

void RenderEngine::ApplyGameCommand(const GameCmd& cmd)
{
	std::visit([&](auto&& c) {return ApplyImpl(c); }, cmd);
}

void RenderEngine::ApplyImpl(const CmdAddActor& c)
{
	if (world)
	{
		auto testActor = utility->CreateActor(
			"test",
			world->modelLoader->GetMeshes("cube"),
			"pavement_03_albedo",
			{ 0.f,5.f,0.f },
			world.get());
		std::cout << "Main Spawn Actor\n";
		world->SpawnActor(testActor);
	}

}
// TODO : MeshType도 받아오기
void RenderEngine::ApplyImpl(const CmdUpdateActorConstant& c)
{
	CmdUpdatePrimitive update;
	update.id = (UINT)c.id;
	//m_primitives[c.id]->SetLocalConstant(c.lc);
	m_primitives[c.id]->SetCubeMapMipLevel(c.lc.cubeMapMipLevel);
	m_primitives[c.id]->SetHeightScale(c.lc.heightScale);
	m_primitives[c.id]->SetLocation(c.lc.model.Transpose().Translation());
	m_primitives[c.id]->SetRoughness(c.lc.roughness);
	m_primitives[c.id]->SetMetallic(c.lc.metallic);
	//update.constant = m_primitives[c.id]->GetLocalConstant();
	//update.meshType = MT_primitive;

	//m_renderCmdQueue->Push(update);
}

MeshType RenderEngine::GetMeshType(PrimitiveComponent* primitive)
{
	if (StaticMeshComponent* staticMeshComp = dynamic_cast<StaticMeshComponent*>(primitive)) {
		return MT_primitive;
	}
	else if (PointCloudComponent* pointCloudComp = dynamic_cast<PointCloudComponent*>(primitive))
	{
		return MT_pointCloud;
	}
	else if (DotComponent* pointCloudComp = dynamic_cast<DotComponent*>(primitive))
	{
		return MT_pointCloud;
	}
	else if (CubeMapComponent* cubeMapComp = dynamic_cast<CubeMapComponent*>(primitive))
	{
		return MT_cubeMap;
	}
	else if (CollisionComponent* cubeMapComp = dynamic_cast<CollisionComponent*>(primitive))
	{
		return MT_collision;
	}
	else if (LightComponent* lightComp = dynamic_cast<LightComponent*>(primitive))
	{
		return MT_light;
	}
	else if (SkinnedMeshComponent* skinnedComp = dynamic_cast<SkinnedMeshComponent*>(primitive))
	{
		return MT_skinnedMesh;
	}
	return MT_primitive;
}

void RenderEngine::ClearTexture()
{
	m_commandAllocator->Reset();
	m_commandList->Reset(m_commandAllocator.Get(), nullptr);

	m_commandList->ResourceBarrier(
		1,
		&CD3DX12_RESOURCE_BARRIER::Transition(
			m_computeBuffer.Get(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS
		)
	);
	ID3D12DescriptorHeap* heaps[] =
	{
		m_UAVHeap.Get()
	};
	m_commandList->SetDescriptorHeaps(1, heaps);
	m_commandList->ClearUnorderedAccessViewFloat(
		m_UAVHeap->GetGPUDescriptorHandleForHeapStart(),
		m_UAVCPUHeap->GetCPUDescriptorHandleForHeapStart(),
		m_computeBuffer.Get(), computeClearColor,
		0, nullptr
	);
	m_commandList->ResourceBarrier(
		1,
		&CD3DX12_RESOURCE_BARRIER::Transition(
			m_computeBuffer.Get(),
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE

		)
	);
	m_commandList->Close();
	ID3D12CommandList* commands[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(ARRAYSIZE(commands), commands);

	//FlushCommands();
}

void RenderEngine::Quit()
{
	{
		std::lock_guard<std::mutex> lock(g_mtx);
		frameReady = true;
	}
	{
		std::lock_guard<std::mutex> lock(capture_mtx);
		saveReady = true;
	}

	cv.notify_all();
	captureCv.notify_all();

	renderThread.join();
	saveThread.join();

	FlushResourceCommands();

}

void RenderEngine::GenerateMips(ID3D12Resource* tex)
{
	DirectX::ResourceUploadBatch upload(m_device);
	upload.Begin(D3D12_COMMAND_LIST_TYPE_DIRECT);

	upload.GenerateMips(tex);

	auto finish = upload.End(m_commandQueue.Get());
	finish.wait();
}