#include "VideoApp.h"
#include "RootSignature.h"
#include "PipelineState.h"
#include "Vertex.h"
#include "StaticMesh.h"

#include <fstream>

#include "Directxtk12/DDSTextureLoader.h"
#include "directxtk12/ResourceUploadBatch.h"

using Microsoft::WRL::ComPtr;
using namespace GraphicsUtils;
using namespace Graphics;
using namespace Renderer;
using namespace DirectX;

void SaveFrameAsBMP(const std::string& filename, uint8_t* data, int width, int height, int pitch) {
	std::ofstream file(filename, std::ios::binary);
	if (!file) return;

	uint32_t fileSize = 54 + height * pitch;
	uint8_t bmpHeader[54] = {
		'B','M',
		static_cast<uint8_t>(fileSize), static_cast<uint8_t>(fileSize >> 8),
		static_cast<uint8_t>(fileSize >> 16), static_cast<uint8_t>(fileSize >> 24),
		0,0,0,0, 54,0,0,0, 40,0,0,0,
		static_cast<uint8_t>(width), static_cast<uint8_t>(width >> 8),
		static_cast<uint8_t>(width >> 16), static_cast<uint8_t>(width >> 24),
		static_cast<uint8_t>(-height), static_cast<uint8_t>((-height) >> 8),
		static_cast<uint8_t>((-height) >> 16), static_cast<uint8_t>((-height) >> 24),
		1,0, 32,0,
		0,0,0,0, 0,0,0,0, 0,0,0,0,
		0,0,0,0, 0,0,0,0, 0,0,0,0
	};
	file.write(reinterpret_cast<char*>(bmpHeader), 54);
	for (int y = 0; y < height; ++y) {
		file.write(reinterpret_cast<char*>(data + y * pitch), pitch);
	}
	file.close();
}

Core::VideoApp::VideoApp()
	:BaseApp()
{
}

Core::VideoApp::VideoApp(const int width, const int height)
	:BaseApp(width, height)
{
}

Core::VideoApp::~VideoApp()
{

}

bool Core::VideoApp::InitDirectX()
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

	m_utility = new Utility(m_device.Get(), m_commandList.Get());

	m_cbvSrvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	m_dsvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	m_utility->CreateDescriptorHeap(m_swapChainBufferCount, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, m_swapChainRtvHeap);
	m_utility->CreateDescriptorHeap(3, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, m_texturesHeap, 0, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);

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

	m_commandAllocator->Reset();
	m_commandList->Reset(m_commandAllocator.Get(), nullptr);

	BuildGeometry();
	BuildConstantBuffers();
	CreateTextures();

	m_commandList->Close();

	ID3D12CommandList* commands[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(ARRAYSIZE(commands), commands);

	FlushCommands();


	return true;
}

bool Core::VideoApp::InitGUI()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	// io.Fonts->TexID = (ImTextureID)m_guiFont->GetSpriteSheet().ptr;

	ImGui::StyleColorsLight();
	const char* fontPath = "Fonts/Hack-Bold.ttf";
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

void Core::VideoApp::OnResize()
{
	if (m_swapChain == nullptr) return;

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

void Core::VideoApp::Update(float deltaTime)
{
	time += deltaTime;

	//if (time > 1 / 30.f)
	//{
	//	
	//	time = 0;
	//	while (av_read_frame(fmtCtx, &pkt) >= 0) {
	//	
	//		if (pkt.stream_index == videoStreamIdx) {
	//			avcodec_send_packet(codecCtx, &pkt);
	//			if (avcodec_receive_frame(codecCtx, frame) == 0) {

	//				m_commandAllocator->Reset();
	//				m_commandList->Reset(m_commandAllocator.Get(), nullptr);

	//				currentFrameIndex++;
	//				sws_scale(swsCtx, frame->data, frame->linesize, 0, codecCtx->height, rgbFrame->data, rgbFrame->linesize);
	//				rgbFrame->format = AV_PIX_FMT_BGRA;

	//				avSubresource.pData = rgbBuffer.data();
	//				avSubresource.RowPitch = codecCtx->width * 4;
	//				avSubresource.SlicePitch = avSubresource.RowPitch * codecCtx->height;

	//				UpdateSubresources(m_commandList.Get(), m_gpuTexture.Get(), m_uploadBuffer.Get(), 0, 0, 1, &avSubresource);

	//				m_commandList->Close();

	//				ID3D12CommandList* cmds[] = { m_commandList.Get() };
	//				m_commandQueue->ExecuteCommandLists(_countof(cmds), cmds);

	//				FlushCommands();

	//				//std::cout << currentFrameIndex << '\n';
	//				break;
	//			}
	//		}
	//	}
	//}

	
}

void Core::VideoApp::UpdateGUI(float deltaTime)
{
}

void Core::VideoApp::Render(float deltaTime)
{
	RenderScene();
}

void  Core::VideoApp::RenderScene()
{
	m_commandAllocator->Reset();
	GraphicsPSO pso = sm_PSOs[0];
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

	FLOAT black[4] = { 0.F, 0.F, 0.F , 1.F };
	m_commandList->ClearRenderTargetView(GetCurrentRtvCpuHandle(), black, 0, nullptr);
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

	ThrowIfFailed(m_swapChain->Present(1, 0));
	m_frameIndex = (m_frameIndex + 1) % m_swapChainBufferCount;


	FlushCommands();
}

void Core::VideoApp::RenderGUI(float deltaTime)
{
}

bool Core::VideoApp::FinDirectX()
{
	return true;
}

void Core::VideoApp::Finalize(float deltaTime)
{

}

void Core::VideoApp::CreateCommandObjects()
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

void Core::VideoApp::CreateSwapChain()
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

void Core::VideoApp::BuildGeometry()
{

	mesh = std::make_shared<StaticMesh>();
	mesh->Initialize(m_device.Get(), m_commandList.Get());


}

void Core::VideoApp::BuildConstantBuffers()
{

	m_utility->CreateConstantBuffer(sizeof(LocalConstant), m_localCB, reinterpret_cast<void**>(&pLocalConstant));
	m_utility->CreateConstantBuffer(sizeof(GlobalConstant), m_globalCB, reinterpret_cast<void**>(&pGlobalConstant));

	//localConstant.model.m[3][0] = 1 / 60.f;
	//localConstant.model = localConstant.model.Transpose();
	memcpy(pLocalConstant, &localConstant, sizeof(LocalConstant));


}

void Core::VideoApp::CreateTextures()
{
	/*avformat_network_init();

	avformat_open_input(&fmtCtx, "videos/test.mp4", nullptr, nullptr);
	avformat_find_stream_info(fmtCtx, nullptr);

	videoStreamIdx = av_find_best_stream(fmtCtx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);*/

	//AVCodecParameters* codecPar = fmtCtx->streams[videoStreamIdx]->codecpar;
	//const AVCodec* codec = avcodec_find_decoder(codecPar->codec_id);
	//codecCtx = avcodec_alloc_context3(codec);
	//avcodec_parameters_to_context(codecCtx, codecPar);
	//avcodec_open2(codecCtx, codec, nullptr);

	//// BGRA 변환용 SWS 컨텍스트
	//swsCtx = sws_getContext(
	//	codecCtx->width, codecCtx->height, codecCtx->pix_fmt,
	//	codecCtx->width, codecCtx->height, AV_PIX_FMT_BGRA,
	//	SWS_BILINEAR, nullptr, nullptr, nullptr);

	//frame = av_frame_alloc();
	//rgbFrame = av_frame_alloc();
	//int numBytes = av_image_get_buffer_size(AV_PIX_FMT_BGRA, codecCtx->width, codecCtx->height, 1);
	//rgbBuffer.resize(numBytes);
	//av_image_fill_arrays(rgbFrame->data, rgbFrame->linesize, rgbBuffer.data(), AV_PIX_FMT_BGRA, codecCtx->width, codecCtx->height, 1);

	//while (av_read_frame(fmtCtx, &pkt) >= 0) {
	//	if (pkt.stream_index == videoStreamIdx) {
	//		avcodec_send_packet(codecCtx, &pkt);
	//		if (avcodec_receive_frame(codecCtx, frame) == 0) {
	//			
	//			sws_scale(swsCtx, frame->data, frame->linesize, 0, codecCtx->height, rgbFrame->data, rgbFrame->linesize);
	//			rgbFrame->format = AV_PIX_FMT_BGRA;  				
	//			//SaveFrameAsBMP("debug_frame0.bmp", rgbBuffer.data(), codecCtx->width, codecCtx->height, rgbFrame->linesize[0]);


	//			D3D12_RESOURCE_DESC texDesc = {};
	//			texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	//			texDesc.Width = codecCtx->width;
	//			texDesc.Height = codecCtx->height;
	//			texDesc.DepthOrArraySize = 1;
	//			texDesc.MipLevels = 1;
	//			texDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	//			texDesc.SampleDesc.Count = 1;
	//			texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	//			texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	//			m_device->CreateCommittedResource(
	//				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
	//				D3D12_HEAP_FLAG_NONE,
	//				&texDesc,
	//				D3D12_RESOURCE_STATE_COPY_DEST,
	//				nullptr,
	//				IID_PPV_ARGS(&m_gpuTexture));

	//			UINT64 uploadSize = 0;
	//			D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint;
	//			UINT numRows;
	//			UINT64 rowSize, totalSize;
	//			m_device->GetCopyableFootprints(&texDesc, 0, 1, 0, &footprint, &numRows, &rowSize, &totalSize);

	//			m_device->CreateCommittedResource(
	//				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
	//				D3D12_HEAP_FLAG_NONE,
	//				&CD3DX12_RESOURCE_DESC::Buffer(totalSize),
	//				D3D12_RESOURCE_STATE_GENERIC_READ,
	//				nullptr,
	//				IID_PPV_ARGS(&m_uploadBuffer));

	//			
	//			avSubresource.pData = rgbBuffer.data();
	//			avSubresource.RowPitch = codecCtx->width * 4;
	//			avSubresource.SlicePitch = avSubresource.RowPitch * codecCtx->height;

	//			UpdateSubresources(m_commandList.Get(), m_gpuTexture.Get(), m_uploadBuffer.Get(), 0, 0, 1, &avSubresource);

	//			// SRV 생성
	//			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	//			srvDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	//			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	//			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	//			srvDesc.Texture2D.MipLevels = 1;

	//			CD3DX12_CPU_DESCRIPTOR_HANDLE heapHandle(m_texturesHeap->GetCPUDescriptorHandleForHeapStart());
	//			m_device->CreateShaderResourceView(m_gpuTexture.Get(), &srvDesc, heapHandle);
	//			

	//			break;
	//			
	//		}
	//	}
	//}

	ComPtr<ID3D12VideoDevice> videoDevice;
    if (FAILED(m_device->QueryInterface(IID_PPV_ARGS(&videoDevice))))
    {
        throw std::runtime_error("ID3D12VideoDevice 얻기 실패");
    }

    // Decode configuration 구조체 구성
    D3D12_VIDEO_DECODE_CONFIGURATION config = {};
    config.DecodeProfile = D3D12_VIDEO_DECODE_PROFILE_H264;
    config.BitstreamEncryption = D3D12_BITSTREAM_ENCRYPTION_TYPE_NONE;
    config.InterlaceType = D3D12_VIDEO_FRAME_CODED_INTERLACE_TYPE_NONE;

    // Decoder descriptor 구성
    D3D12_VIDEO_DECODER_DESC desc = {};
    desc.NodeMask = 0;
    desc.Configuration = config;

    // 디코더 생성
    ComPtr<ID3D12VideoDecoder> decoder;
    HRESULT hr = videoDevice->CreateVideoDecoder(&desc, IID_PPV_ARGS(&decoder));
    if (FAILED(hr))
    {
        throw std::runtime_error("CreateVideoDecoder 실패");
    }

}


D3D12_CPU_DESCRIPTOR_HANDLE Core::VideoApp::GetCurrentRtvCpuHandle() const
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_swapChainRtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
}

ID3D12Resource* Core::VideoApp::GetCurrentSwapChainResource() const
{
	return m_swapChainResources[m_frameIndex].Get();
}

void Core::VideoApp::FlushCommands()
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
