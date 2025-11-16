
#include "MultiThreadApp.h"
#include "RootSignature.h"
#include "PipelineState.h"
#include "GeometryGenerater.h"

#include "Directxtk12/DDSTextureLoader.h"
#include "directxtk12/ResourceUploadBatch.h"

#include <pix3.h>


using Microsoft::WRL::ComPtr;
using namespace GraphicsUtils;
using namespace Graphics;
using namespace Renderer;

Core::MultiThreadApp::MultiThreadApp()
	:BaseApp()
{
}

Core::MultiThreadApp::MultiThreadApp(const int width, const int height)
	:BaseApp(width, height)
{
}

Core::MultiThreadApp::~MultiThreadApp()
{
}

int Core::MultiThreadApp::Run()
{
	MSG msg = { };
	m_timer.Reset();

	while (world->isRunning) {
		if (PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE)) {
			if (msg.message == WM_QUIT)
			{
				world->isRunning = false;
				break;
			}

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			PIXBeginEvent(0, L"Frame Update");

			m_timer.Tick();
			deltaTime = (float)m_timer.GetDeltaTime();

			Update(deltaTime);

		}
	}
	world->isRunning = false;
	m_renderEngine->Quit();

	std::cout << "Run 함수 종료\n";
	return (int)msg.wParam;
}

bool Core::MultiThreadApp::InitDirectX()
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


	Graphics::InitializeCommonState(m_device);
	Renderer::Initialize(m_device);
	m_renderEngine = std::make_unique<RenderEngine>(m_device.Get());
	m_renderEngine->Initialize(m_width, m_height, m_dxgiFactory.Get(), m_mainWnd);

	return true;
}

bool Core::MultiThreadApp::InitGUI()
{
	if (m_renderEngine)
	{
		return m_renderEngine->InitGUI(m_mainWnd);
	}
	return false;
	//IMGUI_CHECKVERSION();
	//ImGui::CreateContext();
	//ImGuiIO& io = ImGui::GetIO();
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	//// io.Fonts->TexID = (ImTextureID)m_guiFont->GetSpriteSheet().ptr;

	//ImGui::StyleColorsLight();
	//const char* fontPath = "Fonts/Hack-Regular.ttf";
	//float fontSize = 15.0f;
	//// 폰트 로드 
	//io.Fonts->AddFontFromFileTTF(fontPath, fontSize);

	//D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	//heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	//heapDesc.NumDescriptors = 1;
	//heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	//m_device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_guiFontHeap));

	//// Setup Platform/Renderer backends
	//ImGui_ImplWin32_Init(m_mainWnd);

	//ImGui_ImplDX12_Init(m_device.Get(), m_swapChainBufferCount, Renderer::backBufferFormat,
	//	m_guiFontHeap.Get(),
	//	m_guiFontHeap->GetCPUDescriptorHandleForHeapStart(),
	//	m_guiFontHeap->GetGPUDescriptorHandleForHeapStart());

	return true;
}

// main thread
void Core::MultiThreadApp::Update(float deltaTime)
{
	PIXBeginEvent(0, L"Game Update");

	if (world)
	{
		world->Tick(deltaTime);
	}
	m_renderEngine->Tick(deltaTime);

}

void Core::MultiThreadApp::OnResize()
{
	if (world)
	{
		world->UpdateCamera(m_width, m_height);
	}
	if (m_renderEngine)
	{
		m_renderEngine->RequestResize(m_width, m_height);
	}
}
