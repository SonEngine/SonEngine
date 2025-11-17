#include "MultiThreadApp.h"
#include "RootSignature.h"
#include "PipelineState.h"
#include "GeometryGenerator.h"

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

	m_physXEngine = std::make_shared<PhysXEngine>();
	m_physXEngine->Initialize();

	if (world) { world->InitializePhysics(m_physXEngine.get()); }
	
	m_renderEngine = std::make_shared<RenderEngine>(m_device.Get());
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
}

// main thread
void Core::MultiThreadApp::Update(float deltaTime)
{
	PIXBeginEvent(0, L"Game Update");

	if (world)
	{
		world->Tick(deltaTime);
		world->SyncKinematicToPhysX();

	}
	m_physXEngine->Tick(deltaTime);
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
