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

Core::SimpleApp::SimpleApp()
	:BaseApp(),
	m_eyePosition(Vector3(0, 0, -3)),
	m_eyeDirection(Vector3(0, 0, 1)),
	m_upDirection(Vector3(0, 1, 0)),
	m_rightDirection(Vector3(1, 0, 0))
{
}

Core::SimpleApp::SimpleApp(const int width, const int height)
	:BaseApp(width, height),
	m_eyePosition(Vector3(0, 0, -3)),
	m_eyeDirection(Vector3(0, 0, 1)),
	m_upDirection(Vector3(0, 1, 0)),
	m_rightDirection(Vector3(1, 0, 0))
{
	m_aspectRatio = width / (float)height;
}

Core::SimpleApp::~SimpleApp()
{

}

bool Core::SimpleApp::InitDirectX()
{
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

	Graphics::InitializeCommonState(m_device);
	Renderer::Initialize(m_device);

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

	ImGui_ImplDX12_Init(m_device.Get(), m_swapChainBufferCount, m_backbufferFormat,
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
	// local
	localConstant.model.m[3][2] = m_zValue;
	
	// global
	m_eyePosition += m_inputHelper.ExecuteCommands(deltaTime, m_eyeDirection, m_upDirection, m_rightDirection);

	globalConstant.view = XMMatrixLookToLH(m_eyePosition, m_eyeDirection, m_upDirection);
	
	if (bMouseFlag)
	{
		bMouseFlag = false;
		GetCursorPos(&mousePos);
	}
	memcpy(pLocalConstant, &localConstant, sizeof(LocalConstant));
	memcpy(pGlobalConstant, &globalConstant, sizeof(GlobalConstant));
}

void Core::SimpleApp::UpdateGUI(float deltaTime)
{
	ImGui::DragFloat("zValue", &m_zValue, 1.f, 0.f, 10.f);
	std::string str = "x : ";
	str += std::to_string(mousePos.x);
	str += ", y : ";
	str += std::to_string(mousePos.y);
	ImGui::Text(str.c_str());
}

void Core::SimpleApp::Render(float deltaTime)
{
	RenderScene();
}

void  Core::SimpleApp::RenderScene()
{
	m_commandAllocator->Reset();
	m_commandList->Reset(m_commandAllocator.Get(), sm_PSOs[0].GetPSO());

	m_commandList->RSSetScissorRects(1, &m_scissorRect);
	m_commandList->RSSetViewports(1, &m_viewport);

	m_commandList->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_commandList->SetPipelineState(sm_PSOs[0].GetPSO());
	m_commandList->SetGraphicsRootSignature(sm_PSOs[0].GetRootSignature()->GetSignature());


	m_commandList->ResourceBarrier(
		1,
		&CD3DX12_RESOURCE_BARRIER::Transition(
			GetCurrentSwapChainResource(),
			D3D12_RESOURCE_STATE_PRESENT,
			D3D12_RESOURCE_STATE_RENDER_TARGET
		));

	m_commandList->ClearRenderTargetView(GetCurrentRtvCpuHandle(), rtvClearColor.data(), 0, nullptr);
	m_commandList->OMSetRenderTargets(1, &GetCurrentRtvCpuHandle(), TRUE, nullptr);

	m_commandList->SetGraphicsRootConstantBufferView(1, m_localCB->GetGPUVirtualAddress());
	m_commandList->SetGraphicsRootConstantBufferView(2, m_globalCB->GetGPUVirtualAddress());

	ID3D12DescriptorHeap* heaps[] = {
		m_texturesHeap.Get()
	};

	m_commandList->SetDescriptorHeaps(1, heaps);
	m_commandList->SetGraphicsRootDescriptorTable(0, m_texturesHeap->GetGPUDescriptorHandleForHeapStart());

	mesh->Render(m_commandList.Get());

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
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
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

	//localConstant.model.m[3][0] = 1/60.f;
	//localConstant.model = localConstant.model.Transpose();

	memcpy(
		pLocalConstant,
		&localConstant,
		sizeof(LocalConstant)
	);

	m_fovRadians = XMConvertToRadians(m_fovDegrees);
	globalConstant.proj = XMMatrixPerspectiveFovLH(
		m_fovRadians, m_aspectRatio, m_nearZ, m_farZ);

	memcpy(
		pGlobalConstant,
		&globalConstant,
		sizeof(LocalConstant)
	);


}

void Core::SimpleApp::CreateTextures()
{

	ResourceUploadBatch resourceUpload(m_device.Get());
	resourceUpload.Begin();

	ThrowIfFailed(CreateDDSTextureFromFile(m_device.Get(), resourceUpload, L"Textures/bricks.dds", m_texture.GetAddressOf()));

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = m_texture->GetDesc().Format;

	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = m_texture->GetDesc().MipLevels;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.f;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;


	CD3DX12_CPU_DESCRIPTOR_HANDLE textureHandle(m_texturesHeap->GetCPUDescriptorHandleForHeapStart());
	m_device->CreateShaderResourceView(m_texture.Get(), &srvDesc, textureHandle);

	auto uploadResourcesFinished = resourceUpload.End(m_commandQueue.Get());

	uploadResourcesFinished.wait();

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