#include "SimpleApp.h"
#include "RootSignature.h"
#include "PipelineState.h"
#include "Vertex.h"
#include "StaticMesh.h"
#include "GeometryGenerater.h"

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
}

Core::SimpleApp::SimpleApp(const int width, const int height)
	:BaseApp(width, height)
{
	m_aspectRatio = width / (float)height;
}

Core::SimpleApp::~SimpleApp()
{
	m_camera.reset();
}

bool Core::SimpleApp::InitDirectX()
{
	m_camera = std::make_shared<Camera>();
	m_camera->SetSpeed(3.f);
	m_camera->SetRotateSpeed(0.5f);
	m_camera->SetPosition(Vector3(0, 1, 0.f));
	m_directionLight = std::make_shared<Light>();

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
	CreateSwapChain();

	utility = std::make_shared<GraphicsUtils::Utility>(m_device.Get(), m_commandList.Get());

	m_cbvSrvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	m_dsvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	utility->CreateDescriptorHeap(m_swapChainBufferCount, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, m_swapChainRtvHeap);
	utility->CreateDescriptorHeap(1, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, m_texturesHeap, 0, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);

	// Create SwapChain RTVs

	CD3DX12_CPU_DESCRIPTOR_HANDLE handle(m_swapChainRtvHeap->GetCPUDescriptorHandleForHeapStart());
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

	BuildGeometry();
	BuildConstantBuffers();
	CreateTextures();

	rtvClearColor = { 0.53F, 0.81F, 0.92F, 1.0F };



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

	ImGui_ImplDX12_Init(m_device.Get(), m_swapChainBufferCount, backbufferFormat,
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


	CD3DX12_CPU_DESCRIPTOR_HANDLE handle(m_swapChainRtvHeap->GetCPUDescriptorHandleForHeapStart());
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
	if (isFocused && isFPSMode)
	{
		// set cursor pos center
		GetWindowRect(m_mainWnd, &windowRect);
		int x = (windowRect.right + windowRect.left) / 2;
		int y = (windowRect.bottom + windowRect.top) / 2;
		SetCursorPos(x, y);

		// gui test
		localConstant.model.m[3][2] = m_zValue;

		// update camera
		m_camera->UpdateCameraRotation(mouseDeltaX, mouseDeltaY);
		m_camera->UpdateCameraPosition(m_inputHelper.ExecuteCommands(deltaTime, m_camera.get()));

		// update consatant
		globalConstant.view = m_camera->GetViewMatrix();

		phongGC.view = globalConstant.view;
		phongGC.cameraPos = ToVector4(m_camera->GetPosition(), 0.f);
		phongGC.cameraDir = ToVector4(m_camera->GetFrontDirection(), 0.f);
		phongGC.DirectionLightPos = ToVector4(m_directionLight->GetPosition(), 0.f);
		phongGC.DirectionLightDir = ToVector4(m_directionLight->GetFrontDirection(), 0.f);

		memcpy(pLocalConstant, &localConstant, sizeof(LocalConstant));
		memcpy(pGlobalConstant, &globalConstant, sizeof(GlobalConstant));
		memcpy(pPhongCB, &phongGC, sizeof(PhongGlobalConstant));

		// reset mouse
		mouseDeltaX = 0;
		mouseDeltaY = 0;
	}
}

void Core::SimpleApp::UpdateGUI(float deltaTime)
{
	std::string str = "FPS : ";
	str += std::to_string(int(1 / deltaTime));
	ImGui::Text(str.c_str());

	ImGui::Separator();
	ImGui::Text("PSO Mode");
	ImGui::SameLine();
	if (ImGui::BeginCombo("##pso combo", psoNames[selectedPSOIdx].c_str()))
	{
		for (int i = 0; i < psoNames.size(); i++)
		{
			bool isSelected = (selectedPSOIdx == i);
			if (ImGui::Selectable(psoNames[i].c_str(), isSelected)) {
				selectedPSOIdx = i;
				renderPSO = psoNames[i];
			}
			if (isSelected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	//TODO 선택 기능 추가
	if (selectedMesh != nullptr)
	{
		ImGui::Separator();
		ImGui::Text("current mesh texture");
		ImGui::SameLine();
		if (ImGui::BeginCombo("##texture", selectedMesh->GetAlbedoTextureName().c_str()))
		{
			for (int i = 0; i < m_textureLoader->filenames.size(); i++)
			{
				bool isSelected = (selectedMesh->GetAlbedoTextureName() == m_textureLoader->filenames[i]);
				if (ImGui::Selectable(m_textureLoader->filenames[i].c_str(), isSelected)) {
					selectedMesh->SetAlbedoTexture(m_textureLoader->filenames[i]);
				}
				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
	}

}

void Core::SimpleApp::Render(float deltaTime)
{
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
	m_commandList->OMSetRenderTargets(1, &GetCurrentRtvCpuHandle(), TRUE, nullptr);

	
	ID3D12DescriptorHeap* heaps[] = {
		m_textureLoader->GetHeap()
	};

	m_commandList->SetDescriptorHeaps(1, heaps);
	//m_commandList->SetGraphicsRootDescriptorTable(0, m_textureLoader->GetGPUHandle(1));

	m_commandList->SetGraphicsRootConstantBufferView(1, m_localCB->GetGPUVirtualAddress());

	if (renderPSO == "defaultPSO")
	{
		m_commandList->SetGraphicsRootConstantBufferView(2, m_globalCB->GetGPUVirtualAddress());
		mesh->Render(m_commandList.Get(), m_textureLoader.get());
	}
	else if (renderPSO == "phongPSO")
	{
		m_commandList->SetGraphicsRootConstantBufferView(2, m_phongGCB->GetGPUVirtualAddress());
		for (auto & mesh : phongMeshes)
		{
			mesh->Render(m_commandList.Get(), m_textureLoader.get());
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
	swapChainDesc.Format = backbufferFormat;
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

void Core::SimpleApp::BuildGeometry()
{
	m_commandAllocator->Reset();
	m_commandList->Reset(m_commandAllocator.Get(), nullptr);

	mesh = std::make_shared<StaticMesh>();
	mesh->Initialize(m_device.Get(), m_commandList.Get(), GeometryGenerator::MakeSimpleCube(2, 2, 2));
	mesh->SetAlbedoTexture("8k_earth_albedo");

	//std::shared_ptr<StaticMesh> plane = std::make_shared<StaticMesh>();
	//plane->Initialize(m_device.Get(), m_commandList.Get(), GeometryGenerator::MakePlane(4, 4, 2));
	std::shared_ptr<StaticMesh> sphere = std::make_shared<StaticMesh>();
	sphere->Initialize(m_device.Get(), m_commandList.Get(), GeometryGenerator::MakeSphere(100, 1));
	sphere->SetAlbedoTexture("8k_earth_albedo");
	phongMeshes.push_back(sphere);

	m_commandList->Close();

	ID3D12CommandList* commands[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(ARRAYSIZE(commands), commands);

	FlushCommands();
}

void Core::SimpleApp::BuildConstantBuffers()
{

	utility->CreateConstantBuffer(
		sizeof(LocalConstant),
		m_localCB,
		reinterpret_cast<void**>(&pLocalConstant)
	);

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

	memcpy(
		pLocalConstant,
		&localConstant,
		sizeof(LocalConstant)
	);


	m_fovRadians = XMConvertToRadians(m_fovDegrees);
	globalConstant.proj = XMMatrixPerspectiveFovLH(
		m_fovRadians, m_aspectRatio, m_nearZ, m_farZ);
	globalConstant.view = m_camera->GetViewMatrix();

	memcpy(
		pGlobalConstant,
		&globalConstant,
		sizeof(GlobalConstant)
	);

	m_fovRadians = XMConvertToRadians(m_fovDegrees);
	phongGC.proj = XMMatrixPerspectiveFovLH(
		m_fovRadians, m_aspectRatio, m_nearZ, m_farZ);
	phongGC.view = m_camera->GetViewMatrix();

	phongGC.cameraPos = ToVector4(m_camera->GetPosition(), 0.f);
	phongGC.cameraDir = ToVector4(m_camera->GetFrontDirection(), 0.f);
	phongGC.DirectionLightPos = ToVector4(m_directionLight->GetPosition(), 0.f);
	phongGC.DirectionLightDir = ToVector4(m_directionLight->GetFrontDirection(), 0.f);

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

	m_textureLoader = std::make_shared<TextureLoader>(texturePath);
	m_fallbackLoader = std::make_shared<TextureLoader>(fallbackPath);

	m_textureLoader->LoadIdx(m_device);
	m_fallbackLoader->LoadIdx(m_device);
	m_textureLoader->LoadTextures(m_device, m_commandQueue);

	//ResourceUploadBatch resourceUpload(m_device.Get());
	//resourceUpload.Begin();

	////ThrowIfFailed(CreateDDSTextureFromFile(m_device.Get(), resourceUpload, L"Textures/earth.dds", m_texture.GetAddressOf()));
	//ThrowIfFailed(CreateDDSTextureFromFileEx(m_device.Get(), resourceUpload, L"Textures/earth_Albedo.dds", 0,
	//	D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE,
	//	DDS_LOADER_MIP_AUTOGEN | DDS_LOADER_FORCE_SRGB,
	//	m_texture.ReleaseAndGetAddressOf()));
	//D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = utility->CreateSRVDesc(m_texture.Get());

	//CD3DX12_CPU_DESCRIPTOR_HANDLE textureHandle(m_texturesHeap->GetCPUDescriptorHandleForHeapStart());
	//m_device->CreateShaderResourceView(m_texture.Get(), &srvDesc, textureHandle);

	//auto uploadResourcesFinished = resourceUpload.End(m_commandQueue.Get());

	//uploadResourcesFinished.wait();

}

D3D12_CPU_DESCRIPTOR_HANDLE Core::SimpleApp::GetCurrentRtvCpuHandle() const
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_swapChainRtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
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