#include "SimpleApp.h"
#include "RootSignature.h"
#include "PipelineState.h"
#include "Vertex.h"
#include "StaticMesh.h"
#include "Geometrygenerator.h"

#include "Directxtk12/DDSTextureLoader.h"
#include "directxtk12/ResourceUploadBatch.h"



using Microsoft::WRL::ComPtr;
using namespace GraphicsUtils;
using namespace Graphics;
using namespace Renderer;
using namespace DirectX;
using DirectX::SimpleMath::Vector3;
using namespace DirectX::SimpleMath;

Core::SimpleApp::SimpleApp()
	:BaseApp()
{
	m_aspectRatio = 1280.f / 720.f;
	rtvClearColor = { 0.53F, 0.81F, 0.92F, 1.0F };
	fontClearColor = { 0.F, 0.F, 0.F, 1.0F };
}

Core::SimpleApp::SimpleApp(const int width, const int height)
	:BaseApp(width, height)
{
	m_aspectRatio = width / (float)height;
	rtvClearColor = { 0.53F, 0.81F, 0.92F, 1.0F };
	fontClearColor = { 0.F, 0.F, 0.F, 1.0F };
}

Core::SimpleApp::~SimpleApp()
{
	m_camera.reset();
}

int Core::SimpleApp::Run()
{
	MSG msg = { };
	m_timer.Reset();

	while (msg.message != WM_QUIT) {
		if (PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			m_timer.Tick();
			float deltaTime = (float)m_timer.GetDeltaTime();

			Update(deltaTime);
			Render(deltaTime);
			RenderGUI(deltaTime);
			Finalize(deltaTime);
		}
	}
	return (int)msg.wParam;
}

bool Core::SimpleApp::InitDirectX()
{
	m_camera = std::make_shared<Camera>();
	m_camera->SetActorSpeed(3.f);
	m_camera->SetActorLocation(Vector3(0, 1, 0.f));
	m_directionLight = std::make_shared<Light>();
	m_directionLight->SetActorLocation(Vector3(0, 1, 0.f));

	UINT dxgiFactoryFlags = 0;

	// Enable the debug layer
#if defined(DEBUG) || defined(_DEBUG)
	{
		ComPtr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();
			// Enable additional debug layers.
			dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
		}
	}
#endif

	// Create Factory and Device 
	ComPtr<ID3D12Device5> device;
	ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&m_dxgiFactory)));

	HRESULT hardwareResult = D3D12CreateDevice(
		nullptr,
		D3D_FEATURE_LEVEL_11_0,
		IID_PPV_ARGS(&device));

	if (FAILED(hardwareResult))
	{
		ComPtr<IDXGIAdapter> pWarpAdapter;
		m_dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&pWarpAdapter));
		D3D12CreateDevice(
			pWarpAdapter.Get(),
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&device));;
	}

	ThrowIfFailed(device.As(&m_device));

	Graphics::InitializeCommonState(m_device);
	Renderer::Initialize(m_device);

	CreateCommandObjects();

	utility = std::make_shared<GraphicsUtils::Utility>(m_device.Get(), m_commandList.Get());

	m_cbvSrvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	m_dsvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	utility->CreateDescriptorHeap(m_swapChainBufferCount, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, m_swapChainRTVHeap);
	utility->CreateDescriptorHeap(m_dsBufferCount, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, m_DSVHeap);
	utility->CreateDescriptorHeap(1, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, m_textRtvHeap);
	utility->CreateDescriptorHeap(1, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, m_fontSrvHeap, 0, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
	utility->CreateDescriptorHeap(1, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, m_textSrvHeap, 0, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);

	CreateSwapChain();
	CreateDepthBuffer();

	// Create SwapChain RTVs
	CD3DX12_CPU_DESCRIPTOR_HANDLE handle(m_swapChainRTVHeap->GetCPUDescriptorHandleForHeapStart());
	for (int i = 0; i < m_swapChainBufferCount; i++)
	{
		m_swapChain->GetBuffer(i, IID_PPV_ARGS(m_swapChainResources[i].ReleaseAndGetAddressOf()));
		m_device->CreateRenderTargetView(m_swapChainResources[i].Get(), nullptr, handle);

		handle.Offset(1, m_rtvDescriptorSize);
	}

	m_viewport = CD3DX12_VIEWPORT(0.F, 0.F, (FLOAT)m_width, (FLOAT)m_height);
	m_scissorRect = CD3DX12_RECT(0, 0, (LONG)m_width, (LONG)m_height);

	m_currentFence = 0;
	m_device->CreateFence(m_currentFence, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));

	CreateTexts();

	BuildGeometry();
	BuildConstantBuffers();
	CreateTextures();

	RenderText("1");

	return true;
}

bool Core::SimpleApp::InitGUI()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
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
	ImGui_ImplWin32_Init(m_mainWnd);

	ImGui_ImplDX12_Init(m_device.Get(), m_swapChainBufferCount, backBufferFormat,
		m_guiFontHeap.Get(),
		m_guiFontHeap->GetCPUDescriptorHandleForHeapStart(),
		m_guiFontHeap->GetGPUDescriptorHandleForHeapStart());

	return true;
}

void Core::SimpleApp::OnResize()
{
	if (m_swapChain == nullptr) return;

	m_aspectRatio = m_width / (float)m_height;
	globalConstant.proj = XMMatrixPerspectiveFovLH(
		m_fovRadians, m_aspectRatio, m_nearZ, m_farZ);

	//m_projFlag = true;

	for (int i = 0; i < m_swapChainBufferCount; i++)
	{
		m_swapChainResources[i].Reset();
	}

	m_swapChain->ResizeBuffers(m_swapChainBufferCount,
		m_width,
		m_height,
		DXGI_FORMAT_UNKNOWN,
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);


	CreateDepthBuffer();

	CD3DX12_CPU_DESCRIPTOR_HANDLE handle(m_swapChainRTVHeap->GetCPUDescriptorHandleForHeapStart());
	for (int i = 0; i < m_swapChainBufferCount; i++)
	{
		m_swapChain->GetBuffer(i, IID_PPV_ARGS(m_swapChainResources[i].ReleaseAndGetAddressOf()));
		m_device->CreateRenderTargetView(m_swapChainResources[i].Get(), nullptr, handle);

		handle.Offset(1, m_rtvDescriptorSize);
	}
	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
	m_viewport = CD3DX12_VIEWPORT(0.F, 0.F, (FLOAT)m_width, (FLOAT)m_height);
	m_scissorRect = CD3DX12_RECT(0, 0, (LONG)m_width, (LONG)m_height);

	std::cout << m_width << ' ' << m_height << '\n';
}

void Core::SimpleApp::Update(float deltaTime)
{
	//if (isFocused && isFPSMode)
	//{
	//	// set cursor pos center
	//	GetWindowRect(m_mainWnd, &windowRect);
	//	int x = (windowRect.right + windowRect.left) / 2;
	//	int y = (windowRect.bottom + windowRect.top) / 2;
	//	SetCursorPos(x, y);

	//	// update camera
	//	m_camera->UpdateCameraRotation(mouseDeltaX, mouseDeltaY);
	//	m_camera->UpdateCameraLocation(m_inputHelper.ExecuteCommands(deltaTime, m_camera.get()));

	//	// update consatant
	//	globalConstant.view = m_camera->GetViewMatrix();

	//	phongGC.view = globalConstant.view;
	//	phongGC.viewLoc = ToVector4(m_camera->GetActorLocation(), 0.f);
	//	phongGC.viewDir = ToVector4(m_camera->GetActorFrontDir(), 0.f);
	//	phongGC.DirectionLightLoc = ToVector4(m_directionLight->GetActorLocation(), 0.f);
	//	phongGC.DirectionLightDir = ToVector4(m_directionLight->GetActorFrontDir(), 0.f);

	//	memcpy(pGlobalConstant, &globalConstant, sizeof(GlobalConstant));
	//	memcpy(pPhongCB, &phongGC, sizeof(PhongGlobalConstant));

	//	// reset mouse
	//	mouseDeltaX = 0;
	//	mouseDeltaY = 0;
	//}
}

void Core::SimpleApp::UpdateGUI(float deltaTime)
{
	std::string str = "FPS : ";
	str += std::to_string(int(1 / deltaTime));
	ImGui::Text(str.c_str());

}

void Core::SimpleApp::Render(float deltaTime)
{
	int time = (int)m_timer.GetElapsedTime();
	DrawString(std::to_string(time));
	RenderScene(renderPSO);
}

void  Core::SimpleApp::RenderScene(const std::string& psoName)
{
	GraphicsPSO pso;
	if (m_PSOs.find(psoName) != m_PSOs.end())
	{
		pso = m_PSOs[psoName];
	}
	else
	{
		pso = m_PSOs["defaultPSO"];
	}


	m_commandAllocator->Reset();
	m_commandList->Reset(m_commandAllocator.Get(), pso.GetPSO());

	m_commandList->RSSetScissorRects(1, &m_scissorRect);
	m_commandList->RSSetViewports(1, &m_viewport);

	m_commandList->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_commandList->SetPipelineState(pso.GetPSO());
	m_commandList->SetGraphicsRootSignature(pso.GetRootSignature()->GetSignature());


	m_commandList->ResourceBarrier(
		1,
		&CD3DX12_RESOURCE_BARRIER::Transition(
			GetCurrentSwapChainResource(),
			D3D12_RESOURCE_STATE_PRESENT,
			D3D12_RESOURCE_STATE_RENDER_TARGET
		));

	m_commandList->ClearRenderTargetView(GetCurrentRtvCpuHandle(), rtvClearColor.data(), 0, nullptr);
	m_commandList->ClearDepthStencilView(GetDSVCpuHandle(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.f, 0, 0, nullptr);
	m_commandList->OMSetRenderTargets(1, &GetCurrentRtvCpuHandle(), TRUE, &GetDSVCpuHandle());

	if (renderPSO == "defaultPSO")
	{

		ID3D12DescriptorHeap* heaps[] = {
		m_textSrvHeap.Get()
		};

		m_commandList->SetDescriptorHeaps(1, heaps);
		m_commandList->SetGraphicsRootDescriptorTable(0, m_textSrvHeap->GetGPUDescriptorHandleForHeapStart());

		m_commandList->SetGraphicsRootConstantBufferView(2, m_globalCB->GetGPUVirtualAddress());
		mesh->Render(m_commandList.Get());
	}
	else if (renderPSO == "phongPSO")
	{
		/*ID3D12DescriptorHeap* heaps[] = {
		m_textureLoader->GetHeap()
		};

		m_commandList->SetDescriptorHeaps(1, heaps);*/
		ID3D12DescriptorHeap* heaps[] = {
			m_textSrvHeap.Get()
		};

		m_commandList->SetDescriptorHeaps(1, heaps);
		m_commandList->SetGraphicsRootDescriptorTable(0, m_textSrvHeap->GetGPUDescriptorHandleForHeapStart());
		m_commandList->SetGraphicsRootConstantBufferView(2, m_globalCB->GetGPUVirtualAddress());

		for (auto& mesh : phongMeshes)
		{
			//mesh->Render(m_commandList.Get(), m_textureLoader.get());
			mesh->Render(m_commandList.Get());
		}
	}
	
	m_commandList->ResourceBarrier(
		1,
		&CD3DX12_RESOURCE_BARRIER::Transition(
			GetCurrentSwapChainResource(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PRESENT
		));

	m_commandList->Close();

	ID3D12CommandList* commands[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(ARRAYSIZE(commands), commands);

	FlushCommands();
}

void Core::SimpleApp::RenderGUI(float deltaTime)
{
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	ImGui::Begin("GUI");
	UpdateGUI(deltaTime);

	ImGui::End();
	ImGui::Render();

	ThrowIfFailed(m_commandAllocator->Reset());
	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), nullptr));

	m_commandList->ResourceBarrier(1,
		&CD3DX12_RESOURCE_BARRIER::Transition(
			GetCurrentSwapChainResource(),
			D3D12_RESOURCE_STATE_PRESENT,
			D3D12_RESOURCE_STATE_RENDER_TARGET
		));

	m_commandList->OMSetRenderTargets(1, &GetCurrentRtvCpuHandle(), false, nullptr);
	ID3D12DescriptorHeap* pHeaps[] = { m_guiFontHeap.Get() };
	m_commandList->SetDescriptorHeaps(static_cast<UINT>(std::size(pHeaps)), pHeaps);
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), m_commandList.Get());

	m_commandList->ResourceBarrier(1,
		&CD3DX12_RESOURCE_BARRIER::Transition(
			GetCurrentSwapChainResource(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PRESENT
		));

	m_commandList->Close();
	ID3D12CommandList* lists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(lists), lists);

	ThrowIfFailed(m_swapChain->Present(1, 0));
	m_frameIndex = (m_frameIndex + 1) % m_swapChainBufferCount;


	FlushCommands();
}

bool Core::SimpleApp::FinDirectX()
{
	return true;
}

void Core::SimpleApp::Finalize(float deltaTime)
{

}

void Core::SimpleApp::CreateCommandObjects()
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

void Core::SimpleApp::CreateSwapChain()
{
	ComPtr<IDXGISwapChain1> swapChain;

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = m_swapChainBufferCount;
	swapChainDesc.Width = m_width;
	swapChainDesc.Height = m_height;
	swapChainDesc.Format = backBufferFormat;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.SampleDesc.Count = 1;

	ThrowIfFailed(m_dxgiFactory->CreateSwapChainForHwnd(
		m_commandQueue.Get(),
		m_mainWnd,
		&swapChainDesc,
		nullptr,
		nullptr,
		&swapChain
	));

	ThrowIfFailed(swapChain.As(&m_swapChain));

	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
}

void Core::SimpleApp::CreateDepthBuffer()
{
	D3D12_RESOURCE_DESC rDesc = {};
	rDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	rDesc.Format = dsBufferFormat;
	rDesc.MipLevels = 0;
	rDesc.DepthOrArraySize = 1;
	rDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	rDesc.Width = m_width;
	rDesc.Height = m_height;
	rDesc.SampleDesc = { 1,0 };

	D3D12_CLEAR_VALUE cValue = {};
	cValue.DepthStencil.Depth = 1.f;
	cValue.DepthStencil.Stencil = 0;
	cValue.Format = dsBufferFormat;

	m_device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&rDesc,
		D3D12_RESOURCE_STATE_COMMON,
		&cValue,
		IID_PPV_ARGS(m_depthStencilBuffer.ReleaseAndGetAddressOf())
	);

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;
	dsvDesc.Format = dsBufferFormat;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

	D3D12_CPU_DESCRIPTOR_HANDLE handle = m_DSVHeap->GetCPUDescriptorHandleForHeapStart();
	m_device->CreateDepthStencilView(m_depthStencilBuffer.Get(), &dsvDesc, handle);

}

void Core::SimpleApp::BuildGeometry()
{
	m_commandAllocator->Reset();
	m_commandList->Reset(m_commandAllocator.Get(), nullptr);

	mesh = std::make_shared<StaticMesh>();
	mesh->Initialize<SimpleVertex,uint16_t>(m_device.Get(), m_commandList.Get(), GeometryGenerator::MakeSimpleRect(2, 2));
	mesh->SetAlbedoTexture("test_albedo");

		std::shared_ptr<StaticMesh> sphere3 = std::make_shared<StaticMesh>();
	sphere3->Initialize<Vertex, uint16_t>(m_device.Get(), m_commandList.Get(), GeometryGenerator::MakePlane(1, 1,1));
	sphere3->SetAlbedoTexture("8k_earth_albedo");
	sphere3->SetLocation(0, 0, 0);
	phongMeshes.push_back(sphere3);


	m_commandList->Close();

	ID3D12CommandList* commands[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(ARRAYSIZE(commands), commands);

	FlushCommands();
}

void Core::SimpleApp::BuildConstantBuffers()
{

	utility->CreateConstantBuffer(
		sizeof(GlobalConstant),
		m_globalCB,
		reinterpret_cast<void**>(&pGlobalConstant)
	);

	utility->CreateConstantBuffer(
		sizeof(PhongGlobalConstant),
		m_phongGCB,
		reinterpret_cast<void**>(&pPhongCB)
	);

	// Initalize Constant Buffers	

	m_fovRadians = XMConvertToRadians(m_fovDegrees);
	globalConstant.proj = XMMatrixPerspectiveFovLH(
		m_fovRadians, m_aspectRatio, m_nearZ, m_farZ);
	globalConstant.view = m_camera->GetViewMatrix();

	memcpy(
		pGlobalConstant,
		&globalConstant,
		sizeof(GlobalConstant)
	);

	phongGC.proj = XMMatrixPerspectiveFovLH(
		m_fovRadians, m_aspectRatio, m_nearZ, m_farZ);
	phongGC.view = m_camera->GetViewMatrix();

	phongGC.viewLoc = ToVector4(m_camera->GetActorLocation(), 0.f);
	phongGC.viewDir = ToVector4(m_camera->GetActorFrontDir(), 0.f);
	phongGC.DirectionLightLoc = ToVector4(m_directionLight->GetActorLocation(), 0.f);
	phongGC.DirectionLightDir = ToVector4(m_directionLight->GetActorFrontDir(), 0.f);

	memcpy(
		pPhongCB,
		&phongGC,
		sizeof(PhongGlobalConstant)
	);

}

void Core::SimpleApp::CreateTextures()
{
	texturePath = "Build/";
	fallbackPath = "Build/Fallback/";

	DDSPath = "Textures/DDS/";
	fallbackDDSPath = "Textures/Falback/";

	m_textureLoader = std::make_shared<TextureLoader>(texturePath, m_device.Get());
	m_fallbackLoader = std::make_shared<TextureLoader>(fallbackPath, m_device.Get());

	m_textureLoader->LoadIdx();
	m_fallbackLoader->LoadIdx();
	m_textureLoader->LoadTextures(m_commandQueue);
}

void Core::SimpleApp::CreateTexts()
{
	m_graphicsMemory.reset();

	m_graphicsMemory = std::make_unique<DirectX::GraphicsMemory>(m_device.Get());

	DirectX::ResourceUploadBatch upload(m_device.Get());
	upload.Begin();

	RenderTargetState rtStateForTextRT(backBufferFormat, dsBufferFormat);

	font = std::make_shared<SpriteFont>(
		m_device.Get(), upload,
		L"Fonts/default.spritefont",
		m_fontSrvHeap->GetCPUDescriptorHandleForHeapStart(),
		m_fontSrvHeap->GetGPUDescriptorHandleForHeapStart());

	DirectX::SpriteBatchPipelineStateDescription pd(rtStateForTextRT);
	spriteBatch = std::make_shared<SpriteBatch>(m_device.Get(), upload, pd);

	
	auto fut = upload.End(m_commandQueue.Get());
	fut.wait();
}

void Core::SimpleApp::RenderText(const std::string& str)
{
	D3D12_RESOURCE_DESC texDesc = {};
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Width = 512;
	texDesc.Height = 512;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	texDesc.MipLevels = 1;
	texDesc.SampleDesc.Count = 1;
	texDesc.DepthOrArraySize = 1;
	D3D12_CLEAR_VALUE clearValue = {};
	clearValue.Format = texDesc.Format;

	clearValue.Color[0] = 1.0f;
	clearValue.Color[1] = 1.0f;
	clearValue.Color[2] = 1.0f;
	clearValue.Color[3] = 1.0f;

	ThrowIfFailed(m_device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		&clearValue, 
		IID_PPV_ARGS(&m_textRT)));

	auto rtvHandle = m_textRtvHeap->GetCPUDescriptorHandleForHeapStart();
	m_device->CreateRenderTargetView(m_textRT.Get(), nullptr, rtvHandle);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = m_textRT->GetDesc().Format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = m_textRT->GetDesc().MipLevels;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.f;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;

	m_device->CreateShaderResourceView(m_textRT.Get(), &srvDesc, m_textSrvHeap->GetCPUDescriptorHandleForHeapStart());

	//DrawString(str);
	
}

void Core::SimpleApp::DrawString(const std::string& str)
{
	UINT64 width = m_textRT->GetDesc().Width;
	UINT64 height = m_textRT->GetDesc().Height;

	D3D12_VIEWPORT viewport = CD3DX12_VIEWPORT(0.F, 0.F, (FLOAT)width, (FLOAT)height);
	D3D12_RECT scissorRect = CD3DX12_RECT(0, 0, (LONG)width, (LONG)height);

	GraphicsPSO pso = m_PSOs["defaultPSO"];

	m_commandAllocator->Reset();
	m_commandList->Reset(m_commandAllocator.Get(), pso.GetPSO());

	m_commandList->RSSetScissorRects(1, &scissorRect);
	m_commandList->RSSetViewports(1, &viewport);

	m_commandList->SetPipelineState(pso.GetPSO());
	m_commandList->SetGraphicsRootSignature(pso.GetRootSignature()->GetSignature());
	m_commandList->ClearRenderTargetView(m_textRtvHeap->GetCPUDescriptorHandleForHeapStart(), DirectX::Colors::White, 0, nullptr);

	m_commandList->OMSetRenderTargets(
		1,
		&m_textRtvHeap->GetCPUDescriptorHandleForHeapStart(),
		FALSE,
		nullptr);


	ID3D12DescriptorHeap* heaps[] = { m_fontSrvHeap.Get() };
	m_commandList->SetDescriptorHeaps(static_cast<UINT>(std::size(heaps)), heaps);
	spriteBatch->Begin(m_commandList.Get());

	spriteBatch->SetViewport(viewport);

	DirectX::SimpleMath::Vector2 m_fontPos = DirectX::SimpleMath::Vector2(width/2.f, height/2.f);

	DirectX::XMVECTORF32 color = DirectX::Colors::Black;
	font->DrawString(spriteBatch.get(), str.c_str(),
		m_fontPos, color);

	spriteBatch->End();

	m_commandList->Close();
	ID3D12CommandList* lists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(lists), lists);


	FlushCommands();
}

D3D12_CPU_DESCRIPTOR_HANDLE Core::SimpleApp::GetCurrentRtvCpuHandle() const
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_swapChainRTVHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
}

D3D12_CPU_DESCRIPTOR_HANDLE Core::SimpleApp::GetDSVCpuHandle() const
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_DSVHeap->GetCPUDescriptorHandleForHeapStart());
}


ID3D12Resource* Core::SimpleApp::GetCurrentSwapChainResource() const
{
	return m_swapChainResources[m_frameIndex].Get();
}

void Core::SimpleApp::FlushCommands()
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