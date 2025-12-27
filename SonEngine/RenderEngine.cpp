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

#include "Directxtk12/DDSTextureLoader.h"
#include "directxtk12/ResourceUploadBatch.h"

#include <pix3.h>

using Microsoft::WRL::ComPtr;
using namespace GraphicsUtils;
using namespace Graphics;
using namespace Renderer;

RenderEngine::RenderEngine(ID3D12Device5* device)
	:m_device(device)
{
}

RenderEngine::~RenderEngine()
{
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

bool RenderEngine::Initialize(int width, int height, int guiWidth, IDXGIFactory7* factory, HWND wnd, MouseInputStateHelper* mouseInputState)
{
	dlModel = std::make_shared<DLModel>();
	dlModel->Initialize("DL/models/mlp.pt");

	m_boundedQueue = std::make_shared<BoundedQueue>(m_frameResourceCount);
	pMouseinputStateHelper = mouseInputState;
	m_guiWidth = guiWidth;
	m_width = width;
	m_height = height;
	mainWnd = wnd;

	m_viewport = CD3DX12_VIEWPORT((FLOAT)m_guiWidth, 0.F, (FLOAT)(m_width - m_guiWidth), (FLOAT)m_height);
	m_scissorRect = CD3DX12_RECT((LONG)0, 0, (LONG)(m_width), (LONG)m_height);

	rtvClearColor = { 0.53F, 0.81F, 0.92F, 1.0F };

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
	utility->CreateDescriptorHeap(m_dsBufferCount, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, m_DSVHeap);
	utility->CreateDescriptorHeap(1, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, m_fontSrvHeap, 0, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);

	//textCount = 9;
	//cpuTexts.resize(textCount);

	// DescriptorHeap 생성
	utility->CreateDescriptorHeap(m_swapChainBufferCount, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, m_swapChainRTVHeap);
	utility->CreateDescriptorHeap(m_dsBufferCount, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, m_DSVHeap);
	utility->CreateDescriptorHeap(1, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, m_fontSrvHeap, 0, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);

	// Compute Shader에서 사용할 버퍼 생성
	{
		computeTextureDIMX = m_width - m_guiWidth;
		computeTextureDIMY = m_height;
		D3D12_RESOURCE_FLAGS flag = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS | D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		utility->CreateTextureBuffer(m_computeBuffer, computeTextureDIMX, computeTextureDIMY, m_computeBufferFormat, flag, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, 0);

		utility->CreateDescriptorHeap(1, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, m_UAVHeap, 0, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
		utility->CreateDescriptorHeap(1, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, m_UAVCPUHeap, 0, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
		utility->CreateDescriptorHeap(1, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, m_SRVHeap, 0, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);

		utility->CreateResourceView(m_computeBuffer, m_computeBufferFormat, false, m_UAVHeap->GetCPUDescriptorHandleForHeapStart(), DescriptorType::UAV);
		utility->CreateResourceView(m_computeBuffer, m_computeBufferFormat, false, m_UAVCPUHeap->GetCPUDescriptorHandleForHeapStart(), DescriptorType::UAV);
	}

	CreateSwapChain(factory, wnd);
	CreateDepthBuffer();
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
	m_textureLoader->AddTexture(m_computeBuffer, m_computeTextureName);
	BuildFrameResources();

	computeClearColor[0] = 0.f;
	computeClearColor[1] = 0.f;
	computeClearColor[2] = 0.f;
	computeClearColor[3] = 1.f;

	{
		m_commandAllocator->Reset();
		m_commandList->Reset(m_commandAllocator.Get(), nullptr);

		if (world)
		{
			world->Initialize(m_width, m_height, this, m_device, m_commandList.Get());
		}


		m_commandList->Close();
		ID3D12CommandList* commands[] = { m_commandList.Get() };
		m_commandQueue->ExecuteCommandLists(ARRAYSIZE(commands), commands);

		FlushCommands();
	}
	ClearTexture();

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
			else if (runDLReady)
			{
				runDLReady = false;
				lock.unlock();
				RunDLModel();
			}
		}
		});

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
				SaveTextureGPU(m_computeTextureName, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, saveMipLevel, true);
				//SaveTextureGPU("BackBuffer", D3D12_RESOURCE_STATE_PRESENT);
			}
			if (runDLDirty)
			{
				runDLDirty = false;
				FlushResourceCommands();
				SaveTextureGPU(m_computeTextureName, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, saveMipLevel, false);
			}

			frameReady = false;
			lock.unlock();

			m_boundedQueue->Pop(r_packet);
			r_currentResourceIndex = r_packet.frameId % m_frameResourceCount;
			r_currentFrameResource = m_frameResources[r_currentResourceIndex].get();

			r_currentFrameResource->UpdateGlobalConstantBuffer(r_packet.gc);
			r_currentFrameResource->UpdatePBGlobalConstantBuffer(r_packet.pbgc);

			//RenderWithText();
			//RenderWithCompute();
			DrawingWithMouse();
		}});

		return true;
}

bool RenderEngine::InitGUI(HWND wnd)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.IniFilename = nullptr;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
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
	guiPenRadius = 10.f;

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
	CreateDepthBuffer();

	m_currentBackBufferIndex = m_swapChain->GetCurrentBackBufferIndex();

	m_viewport = CD3DX12_VIEWPORT((FLOAT)m_guiWidth, 0.F, (FLOAT)(m_width - m_guiWidth), (FLOAT)m_height);

	m_scissorRect = CD3DX12_RECT((LONG)0, 0, (LONG)(m_width), (LONG)m_height);

}

void RenderEngine::RegisterPrimitive(PrimitiveComponent* primitive)
{
	m_primitives.push_back(primitive);
	for (auto& fr : m_frameResources)
		fr->proxyDirty = true;
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

void RenderEngine::CreateDepthBuffer()
{
	D3D12_RESOURCE_DESC rDesc = {};
	rDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	rDesc.Format = Renderer::dsBufferFormat;
	rDesc.MipLevels = 0;
	rDesc.DepthOrArraySize = 1;
	rDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	rDesc.Width = m_width;
	rDesc.Height = m_height;
	rDesc.SampleDesc = { 1,0 };

	D3D12_CLEAR_VALUE cValue = {};
	cValue.DepthStencil.Depth = 1.f;
	cValue.DepthStencil.Stencil = 0;
	cValue.Format = Renderer::dsBufferFormat;

	m_device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&rDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&cValue,
		IID_PPV_ARGS(m_depthStencilBuffer.ReleaseAndGetAddressOf())
	);

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;
	dsvDesc.Format = Renderer::dsBufferFormat;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

	D3D12_CPU_DESCRIPTOR_HANDLE handle = m_DSVHeap->GetCPUDescriptorHandleForHeapStart();
	m_device->CreateDepthStencilView(m_depthStencilBuffer.Get(), &dsvDesc, handle);
}

void RenderEngine::UpdateGUI()
{
	ImGui::SetWindowSize(ImVec2((float)m_guiWidth, (float)m_height), ImGuiCond_FirstUseEver);
	ImGui::SetWindowPos(ImVec2(0.f, 0.f), ImGuiCond_FirstUseEver);

	ImGui::Checkbox("Change Mode", &test);

	ImGui::NewLine();

	ImGui::SliderFloat("Pen Radius", &guiPenRadius, 0.f, 10.f);

	ImGui::SliderInt("miplevel", &saveMipLevel, 0, 5);
	
	if (ImGui::Button("Run DL")) {
		RequestRunDL();
		printRet = true;
		
	}
	if (printRet)
	{
		//ImGui::SameLine();
		int ret = dlRet.load(std::memory_order_acquire);
		std::string retStr = "DL prediction : ";
		retStr += std::to_string(ret);
		ImGui::Text(retStr.c_str());
	}
	if (ImGui::Button("Save")) {
		RequestCapture();
	}
	ImGui::SameLine();
	if (ImGui::Button("Clear")) {
		clearFlag = true;
	}

	if (ImGui::Button("Open Picker"))
		ImGui::OpenPopup("PickerPopup");

	if (ImGui::BeginPopup("PickerPopup"))
	{
		// 큰 컬러 피커(휠/사각형)
		ImGui::ColorPicker3("##Picker", (float*)&guiPenColor,
			ImGuiColorEditFlags_Float |
			ImGuiColorEditFlags_AlphaBar |
			ImGuiColorEditFlags_DisplayRGB);

		ImGui::EndPopup();
	}


}

void RenderEngine::BuildFrameResources()
{
	m_frameResources.resize(m_frameResourceCount);
	for (int i = 0; i < m_frameResourceCount; i++)
	{
		m_frameResources[i] = std::make_shared<FrameResource>();
		// TODO textActor 개수 추가
		m_frameResources[i]->Initialize(m_device, 512, 512, 0, mainWnd);
	}
}
void RenderEngine::PostActorChanges()
{
	//	if (!m_addActors.empty())
	//{
	//	for (auto& pFr : m_frameResources)
	//	{
	//		pFr->proxyDirty = true;
	//	}
	//	//std::cout << m_addActors.size() << "개 추가\n";
	//	for (auto& a : m_addActors)
	//	{
	//		m_actors.push_back(a);
	//	}
	//	m_addActors.clear();
}

void RenderEngine::BuildRenderProxy()
{
	for (auto& pFr : m_frameResources)
	{
		if (pFr->proxyDirty)
		{
			pFr->proxyDirty = false;
			pFr->proxyBuffer.clear();
			pFr->textProxyBuffer.clear();

			for (auto primitive : m_primitives)
			{
				AddProxy(primitive, pFr.get());
			}
		}
	}

}

// 새로운 compoenet 추가 시 Utility.inl에 h파일 추가
void RenderEngine::AddProxy(SceneComponent* component, FrameResource* fr)
{
	if (StaticMeshComponent* staticMeshComp = dynamic_cast<StaticMeshComponent*>(component))
	{
		//std::cout << "Mesh exist\n";
		Proxy proxy;
		proxy.mesh = staticMeshComp->GetMesh();
		fr->proxyBuffer.push_back(proxy);
	}
	else if (PointCloudComponent* pointCloudComp = dynamic_cast<PointCloudComponent*>(component))
	{
		//std::cout << "Mesh exist\n";
		Proxy proxy;
		proxy.mesh = pointCloudComp->GetMesh();
		fr->pcProxyBuffer.push_back(proxy);
	}
	else if (DotComponent* pointCloudComp = dynamic_cast<DotComponent*>(component))
	{
		//std::cout << "Mesh exist\n";
		Proxy proxy;
		proxy.mesh = pointCloudComp->GetMesh();
		fr->dotProxyBuffer.push_back(proxy);
	}
	/*std::vector<std::shared_ptr<SceneComponent>> child;
	component->GetChildrenComponents(child);
	for (size_t i = 0; i < child.size(); i++)
	{
		AddProxy(child[i].get());
	}*/
}

void RenderEngine::AddTextProxy(SceneComponent* component)
{
	if (StaticMeshComponent* staticMeshComp = dynamic_cast<StaticMeshComponent*>(component))
	{
		//std::cout << "Mesh exist\n";
		TextProxy proxy;
		proxy.mesh = staticMeshComp->GetMesh();
		currentFrameResource->textProxyBuffer.push_back(proxy);
	}

	std::vector<std::shared_ptr<SceneComponent>> child;
	component->GetChildrenComponents(child);
	for (size_t i = 0; i < child.size(); i++)
	{
		AddTextProxy(child[i].get());
	}
}

void  RenderEngine::UpdateMousePosition()
{
	GetCursorPos(&currMousPt);
	ScreenToClient(mainWnd, &currMousPt);
	pMouseinputStateHelper->UpdateMousePos(currMousPt.x, currMousPt.y);
	pMouseinputStateHelper->UpdatePrevMousePos(prevMousePt.x, prevMousePt.y);
}

void RenderEngine::UpdateGlobalConstantBuffer(const ViewProjInfo& viewProjInfo, const std::vector<LightInfo>& lightInfos)
{
	packet.gc.cameraDir = ToVector4(viewProjInfo.viewDirection, 0.f);
	packet.gc.cameraPos = ToVector4(viewProjInfo.viewLocation, 0.f);
	packet.gc.view = viewProjInfo.view.Transpose();
	packet.gc.proj = viewProjInfo.proj.Transpose();

	for (size_t i = 0; i < lightInfos.size(); i++)
	{
		packet.gc.lights[i].direction = lightInfos[0].direction;
		packet.gc.lights[i].location = lightInfos[0].location;
		packet.gc.lights[i].brightness = lightInfos[0].brightness;
	}

	//memcpy(pPhongGCB, &phongGC, sizeof(PhongGlobalConstant));
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

void RenderEngine::Tick(float deltaTime)
{
	if (world)
	{
		packet.frameId = m_frameId++;
		UpdateGlobalConstantBuffer(world->GetViewProjInfo(), world->GetLightInfos());
		{
			UpdateMousePosition();
			UpdatePBGlobalConstantBuffer(m_guiWidth, pMouseinputStateHelper->GetInputState());
			prevMousePt = currMousPt;
		}

		m_boundedQueue->Push(packet);
		BuildRenderProxy();
	}

	{
		std::lock_guard<std::mutex> lock(g_mtx);
		frameReady = true;
	}
	// render thread 호출
	cv.notify_one();

}

void RenderEngine::Render(const std::string& psoName, int idx, RenderType renderType, bool isFinal, bool clear)
{
	PIXBeginEvent(m_commandQueue.Get(), PIX_COLOR(255, 0, 0), "test");

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

	//m_viewport = CD3DX12_VIEWPORT((FLOAT)m_guiWidth, 0.F, (FLOAT)(m_width- m_guiWidth), (FLOAT)m_height);

	commandList->RSSetScissorRects(1, &m_scissorRect);
	commandList->RSSetViewports(1, &m_viewport);

	commandList->SetPipelineState(pso.GetPSO());
	commandList->SetGraphicsRootSignature(pso.GetRootSignature()->GetSignature());

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

	if (renderType == RT_Default)
	{
		commandList->SetGraphicsRootConstantBufferView(2, r_currentFrameResource->GetGCBGPUAddress());

		ID3D12DescriptorHeap* heaps[] = {
			m_textureLoader->GetHeap()
		};

		commandList->SetDescriptorHeaps(1, heaps);
		for (auto& proxy : r_currentFrameResource->proxyBuffer)
		{
			proxy.mesh->Render(commandList, m_textureLoader.get());
		}
	}
	else if (renderType == RT_TEXT)
	{
		commandList->SetGraphicsRootConstantBufferView(2, r_currentFrameResource->GetGCBGPUAddress());

		if (r_currentFrameResource->textProxyBuffer.size() > 0)
		{
			ID3D12DescriptorHeap* heaps[] = {
			r_currentFrameResource->GetTextSrvHeap()
			};

			commandList->SetDescriptorHeaps(1, heaps);
			UINT i = 0;
			for (auto& proxy : r_currentFrameResource->textProxyBuffer)
			{
				commandList->SetGraphicsRootDescriptorTable(0, r_currentFrameResource->GetTextSrvGPUHandle(i));
				proxy.mesh->Render(commandList);
				i++;
			}
		}
	}
	else if (renderType == RT_PointCloud)
	{
		commandList->SetGraphicsRootConstantBufferView(1, r_currentFrameResource->GetGCBGPUAddress());

		// Build Proxy 함수에서 등록된 메시가 PointCloud 일 경우 pcProxy에 등록됨
		for (auto& proxy : r_currentFrameResource->pcProxyBuffer)
		{
			proxy.mesh->RenderPoints(commandList, 0);
		}
	}
	else if (renderType == RT_Dot)
	{
		ID3D12DescriptorHeap* heaps[] = {
			m_textureLoader->GetHeap()
		};

		commandList->SetDescriptorHeaps(1, heaps);
		// Build Proxy 함수에서 등록된 메시가 PointCloud 일 경우 pcProxy에 등록됨
		for (auto& proxy : r_currentFrameResource->dotProxyBuffer)
		{
			proxy.mesh->RenderDot(commandList, m_textureLoader.get());
		}
	}
	commandList->ResourceBarrier(
		1,
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
	PIXEndEvent(m_commandQueue.Get());
}

void RenderEngine::RenderGUI(bool isFinal)
{
	ImGui_ImplWin32_NewFrame();
	ImGui_ImplDX12_NewFrame();
	ImGui::NewFrame();


	ImGuiWindowFlags flags = 0;

	ImGui::Begin("GUI", nullptr, flags);
	UpdateGUI();

	ImGui::End();
	ImGui::Render();

	//r_currentFrameResource = m_frameResources[r_currentResourceIndex].get();
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
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);

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

// FrameResource의 텍스트 렌더용 텍스쳐 업데이트
void RenderEngine::UpdateTexts()
{
	GraphicsPSO pso = m_PSOs["defaultPSO"];
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
}

void RenderEngine::CreateTextures()
{
	texturePath = "Build/";
	fallbackPath = "Build/Fallback/";

	DDSPath = "Textures/DDS/";
	fallbackDDSPath = "Textures/Falback/";

	m_textureLoader = std::make_shared<TextureLoader>(texturePath, m_device);
	m_fallbackLoader = std::make_shared<TextureLoader>(fallbackPath, m_device);

	m_textureLoader->InitHeap(20);
	m_textureLoader->LoadIdx();
	m_fallbackLoader->InitHeap(20);
	m_fallbackLoader->LoadIdx();
	m_textureLoader->LoadTextures(m_commandQueue);
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
	Render("phongPSO", 1/*sequence*/, RT_Default, false/*isFinal*/, true/*clear RT*/);
	Render("pointCloudPSO", 2, RT_PointCloud, true/*isFinal*/, false/*clear RT*/);

	//RenderGUI(true);
}

void RenderEngine::DrawingWithMouse()
{
	Compute(computePSO, 0, false/*isFinal*/, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	if (test)
		Render("pointCloudPSO", 1, RT_PointCloud, false/*isFinal*/, true/*clear RT*/);
	else
		Render("renderTexturePSO", 1, RT_Dot, false/*isFinal*/, true/*clear RT*/);
	RenderGUI(true);
}

D3D12_CPU_DESCRIPTOR_HANDLE RenderEngine::GetCurrentRtvCpuHandle() const
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_swapChainRTVHeap->GetCPUDescriptorHandleForHeapStart(), m_currentBackBufferIndex, m_rtvDescriptorSize);
}

D3D12_CPU_DESCRIPTOR_HANDLE RenderEngine::GetDSVCpuHandle() const
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_DSVHeap->GetCPUDescriptorHandleForHeapStart());
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

	GenerateMips(t);

	D3D12_RESOURCE_DESC desc = t->GetDesc();

	UINT16 mipLevels = desc.MipLevels;

	UINT subresource = D3D12CalcSubresource(
		/*MipSlice*/ miplevel,
		/*ArraySlice*/ 0,
		/*PlaneSlice*/ 0,
		/*MipLevels*/ mipLevels,
		/*ArraySize*/ 1
	);

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
	imageInfo.height = imageInfo.width = footprint.Footprint.Height;
	imageInfo.bufferPixelCount = requiredSize;

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
	std::vector<uint8_t> image(imageInfo.rowSize * imageInfo.numRows);
	uint8_t* mapped = nullptr;
	m_saveBuffer->Map(0, &range, reinterpret_cast<void**>(&mapped));
	//memcpy(image.data(), pSaveBuffer, copyBufferSize);

	for (UINT y = 0; y < imageInfo.numRows; y++) {
		memcpy(image.data() + y * imageInfo.rowSize,
			mapped + y * imageInfo.rowPitch,
			imageInfo.rowSize);
	}
	m_saveBuffer->Unmap(0, nullptr);

	//dlModel->Run(image);

	std::string filename = imageInfo.name + utility->MakeTimestamp() + ".png";
	std::string fileFullPath = imageFilePath + filename;
	stbi_write_png(fileFullPath.c_str(), (int)imageInfo.width, (int)imageInfo.height, 4, image.data(), (int)imageInfo.rowSize);
	std::cout << imageInfo.name << " 가 성공적으로 저장되었습니다.\n";

	std::string cmd = "start \"\" \"" + imageFilePath + "\"";
	system(cmd.c_str());
}

void RenderEngine::RunDLModel()
{
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

	int ret = dlModel->Run(image);
	dlRet.store(ret, std::memory_order_release);
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