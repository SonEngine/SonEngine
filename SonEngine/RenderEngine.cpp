#pragma warning(disable : 4996)

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

#include "RenderEngine.h"
#include "GraphicsCommon.h"

#include "RootSignature.h"
#include "PipelineState.h"
#include "GeometryGenerater.h"

#include "Directxtk12/DDSTextureLoader.h"
#include "directxtk12/ResourceUploadBatch.h"

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

bool RenderEngine::Initialize(int width,int height, IDXGIFactory7* factory, HWND wnd)
{
	m_width = width;
	m_height = height;

	m_viewport = CD3DX12_VIEWPORT(0.F, 0.F, (FLOAT)1280, (FLOAT)720);
	m_scissorRect = CD3DX12_RECT(0, 0, (LONG)1280, (LONG)720);

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
		D3D12_RESOURCE_FLAGS flag = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS | D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		utility->CreateTextureBuffer(m_computeBuffer, computeTextureDIM, computeTextureDIM, m_computeBufferFormat, flag, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

		utility->CreateDescriptorHeap(1, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, m_UAVHeap, 0, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
		utility->CreateDescriptorHeap(1, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, m_SRVHeap, 0, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);

		utility->CreateResourceView(m_computeBuffer, m_computeBufferFormat, false, m_UAVHeap->GetCPUDescriptorHandleForHeapStart(), DescriptorType::UAV);
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

	{
		m_commandAllocator->Reset();
		m_commandList->Reset(m_commandAllocator.Get(), nullptr);

		if (world)
		{
			world->Initialize(m_width, m_height);
		}

		BuildFrameResources();

		m_commandList->Close();
		ID3D12CommandList* commands[] = { m_commandList.Get() };
		m_commandQueue->ExecuteCommandLists(ARRAYSIZE(commands), commands);

		FlushCommands();
	}

	saveThread = std::thread([&] {
		if (world == nullptr)
		{
			std::cout << "World 가 Null입니다.\n";
			return;
		}
		while (world->isRunning) {
			std::unique_lock<std::mutex> lock(capture_mtx);
			captureCv.wait(lock, [&] {return !world->isRunning || saveReady; });

			if (!world->isRunning)
			{
				break;
			}
			saveReady = false;
			lock.unlock();

			SaveTextureCPU();
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
			//TODO : capture 
			//if (captureDirty)
			//{
			//	captureDirty = false;
			//	FlushResourceCommands();
			//	SaveTextureGPU(m_computeTextureName);
			//}
			
			frameReady = false;
			lock.unlock();

			//RenderWithText();
			RenderWithCompute();
		}});

	return true;
}

bool RenderEngine::InitGUI(HWND wnd)
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
	ImGui_ImplWin32_Init(wnd);

	ImGui_ImplDX12_Init(m_device, m_swapChainBufferCount, Renderer::backBufferFormat,
		m_guiFontHeap.Get(),
		m_guiFontHeap->GetCPUDescriptorHandleForHeapStart(),
		m_guiFontHeap->GetGPUDescriptorHandleForHeapStart());

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

	m_viewport = CD3DX12_VIEWPORT(0.F, 0.F, (FLOAT)m_width, (FLOAT)m_height);
	m_scissorRect = CD3DX12_RECT(0, 0, (LONG)m_width, (LONG)m_height);
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

void RenderEngine::UpdateGUI(float deltaTime)
{
	/*std::string str = "FPS : ";
	str += std::to_string(int(1 / deltaTime));
	ImGui::Text(str.c_str());*/
}

void RenderEngine::BuildFrameResources()
{
	m_frameResources.resize(m_frameResourceCount);
	for (int i = 0; i < m_frameResourceCount; i++)
	{
		m_frameResources[i] = std::make_shared<FrameResource>();
		// TODO textActor 개수 추가
		m_frameResources[i]->Initialize(m_device, 512, 512, 0);
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

void RenderEngine::BuildProxy(const std::vector<std::shared_ptr<Actor>>& actors, Actor* player)
{
	if (currentFrameResource->proxyDirty)
	{
		currentFrameResource->proxyDirty = false;
		currentFrameResource->proxyBuffer.clear();
		currentFrameResource->textProxyBuffer.clear();

		//std::cout << m_currentResourceIndex << "build 중\n";
		int id = 0;

		for (auto& actor : actors)
		{
			SceneComponent* root = actor->GetRootComponent();
			AddProxy(root);
		}
		/*for (auto& actor : m_textActors)
		{
			SceneComponent* root = actor->GetRootComponent();
			AddTextProxy(root);
		}*/
		SceneComponent* root = player->GetRootComponent();
		AddProxy(root);
	}

	//Update cpuTexts
	//cpuTexts[0] = std::to_wstring((int)m_timer.GetElapsedTime());
	//cpuTexts[1] = std::to_wstring((float)player->GetActorLocation().x);

	// Update textProxyBuffers
	for (size_t i = 0; i < cpuTexts.size(); i++)
	{
		if (currentFrameResource->textProxyBuffer.size() > i)
			currentFrameResource->textProxyBuffer[i].str = cpuTexts[i];

	}
}

void RenderEngine::AddProxy(SceneComponent* component)
{
	if (StaticMeshComponent* staticMeshComp = dynamic_cast<StaticMeshComponent*>(component))
	{
		//std::cout << "Mesh exist\n";
		Proxy proxy;
		proxy.mesh = staticMeshComp->GetMesh();
		currentFrameResource->proxyBuffer.push_back(proxy);
	}

	std::vector<std::shared_ptr<SceneComponent>> child;
	component->GetChildrenComponents(child);
	for (size_t i = 0; i < child.size(); i++)
	{
		AddProxy(child[i].get());
	}
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

void RenderEngine::Tick(float deltaTime)
{
	currentFrameResource = m_frameResources[m_currentResourceIndex].get();

	// currentFrameResource가 초기값이 아니면서,
	// 현재 사용하려는 리소스의 이전 명령이 아직 이행되지 않았을 경우 
	// 완료할 때까지 기다린다.
	if (currentFrameResource->m_currentFence != 0 &&
		m_fence->GetCompletedValue() < currentFrameResource->m_currentFence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
		m_fence->SetEventOnCompletion(currentFrameResource->m_currentFence, eventHandle);

		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}

	if (world)
	{
		currentFrameResource->UpdateGlobalConstantBuffer(world->GetViewProjInfo());
		BuildProxy(world->GetActors(), world->Getplayer());
	}
	//std::cout << "Main Thread : " << m_currentResourceIndex << std::endl;

	m_currentResourceIndex = (m_currentResourceIndex + 1) % m_frameResourceCount;
	{
		std::lock_guard<std::mutex> lock(g_mtx);
		frameReady = true;
	}

	cv.notify_one();
}

void RenderEngine::Render(const std::string& psoName, int idx, bool isText, bool isFinal, bool clear)
{
	using namespace Renderer;

	// N번을 update했을 때 N-1 번 프레임을 렌더링 
	// update의 경우 m_frameResourceCount만큼 미리 업데이트 가능
	r_currentFrameResource = m_frameResources[r_currentResourceIndex].get();

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
	if (isFinal)
	{
		r_currentResourceIndex = (r_currentResourceIndex + 1) % m_frameResourceCount;
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

	commandList->RSSetScissorRects(1, &m_scissorRect);
	commandList->RSSetViewports(1, &m_viewport);

	commandList->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

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


	commandList->SetGraphicsRootConstantBufferView(2, r_currentFrameResource->GetGCBGPUAddress());
	if (!isText)
	{
		ID3D12DescriptorHeap* heaps[] = {
			m_textureLoader->GetHeap()
		};

		commandList->SetDescriptorHeaps(1, heaps);
		for (auto& proxy : r_currentFrameResource->proxyBuffer)
		{
			proxy.mesh->Render(commandList, m_textureLoader.get());
		}
	}
	else
	{
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
}

// TODO : 사용할 리소스 지정하기
void RenderEngine::Compute(const std::string& cpsoName, int idx, bool isFinal, D3D12_RESOURCE_STATES prevState)
{
	using namespace Renderer;

	// N번을 update했을 때 N-1 번 프레임을 렌더링 
	// update의 경우 m_frameResourceCount만큼 미리 업데이트 가능
	r_currentFrameResource = m_frameResources[r_currentResourceIndex].get();

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
	if (isFinal)
	{
		r_currentResourceIndex = (r_currentResourceIndex + 1) % m_frameResourceCount;
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

	commandList->Dispatch((UINT)ceil(computeTextureDIM / 32.f), (UINT)ceil(computeTextureDIM / 32.f), 1);

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
	r_currentFrameResource = m_frameResources[r_currentResourceIndex].get();

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
	Render(renderPSO, 0, false, false, true);
	UpdateTexts();
	Render(textPSO, 1, true, true, false);
}

void RenderEngine::RenderWithCompute()
{
	Compute(computePSO, 0, false, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	Render(renderPSO, 1, false, true, true);
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

void RenderEngine::SaveTextureGPU(std::string& name)
{
	std::cout << "SaveTextureGPU\n";

	ID3D12Resource* t = m_textureLoader->GetTexture(name);
	D3D12_RESOURCE_DESC desc = t->GetDesc();
	UINT64 w = desc.Width;
	UINT64 h = desc.Height;

	imageInfo.bufferPixelCount = w * h * 4;
	UINT64 copyBufferSize = imageInfo.bufferPixelCount;

	UINT64 requiredSize = 0;
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint;
	m_device->GetCopyableFootprints(&desc, 0, 1, 0, &footprint, nullptr, nullptr, &requiredSize);

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
	imageInfo.rowSize = w * 4;
	imageInfo.numRows = h;
	imageInfo.name = name;
	imageInfo.width = w;
	imageInfo.height = h;

	m_commandList->ResourceBarrier(1,
		&CD3DX12_RESOURCE_BARRIER::Transition(
			t,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_COPY_SOURCE));

	CD3DX12_TEXTURE_COPY_LOCATION dst(m_saveBuffer.Get(), footprint);
	CD3DX12_TEXTURE_COPY_LOCATION src(t, 0);
	m_commandList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);

	m_commandList->ResourceBarrier(1,
		&CD3DX12_RESOURCE_BARRIER::Transition(
			t,
			D3D12_RESOURCE_STATE_COPY_SOURCE,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
		));

	m_commandList->Close();
	ID3D12CommandList* commands[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(ARRAYSIZE(commands), commands);

	{
		std::lock_guard<std::mutex> lock(capture_mtx);
		saveReady = true;
	}
	captureCv.notify_one();
}

void RenderEngine::SaveTextureCPU()
{
	std::cout << "SaveTextureCPU\n";

	CD3DX12_RANGE range(0, 0);
	std::vector<uint8_t> image(imageInfo.bufferPixelCount);
	uint8_t* mapped = nullptr;
	m_saveBuffer->Map(0, &range, reinterpret_cast<void**>(&mapped));
	//memcpy(image.data(), pSaveBuffer, copyBufferSize);

	for (UINT y = 0; y < imageInfo.numRows; y++) {
		memcpy(image.data() + y * imageInfo.rowSize,
			mapped + y * imageInfo.rowPitch,
			imageInfo.rowSize);
	}
	m_saveBuffer->Unmap(0, nullptr);

	std::string filename = imageInfo.name + ".png";
	std::string fileFullPath = imageFilePath + filename;
	stbi_write_png(fileFullPath.c_str(), (int)imageInfo.width, (int)imageInfo.height, 4, image.data(), imageInfo.rowSize);
	std::cout << imageInfo.name << " 가 성공적으로 저장되었습니다.\n";

	std::string cmd = "start \"\" \"" + imageFilePath + "\"";
	system(cmd.c_str());
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