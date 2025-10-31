#include "FrameResource.h"
#include "GraphicsCommon.h"
#include "DirectXColors.h"
#include <iostream>

void FrameResource::Initialize(Microsoft::WRL::ComPtr<ID3D12Device5>& device, UINT64 width, UINT64 height) {

	textureWidth = width;
	textureHeight = height;
	if (Graphics::utility == nullptr)
	{
		std::cout << "Failed FrameResource::Initialize -> Graphics::utility == nullptr\n";
	}
	Graphics::utility->CreateConstantBuffer(
		sizeof(GlobalConstant),
		m_phongGCBuffer,
		reinterpret_cast<void**>(&pPhongGCB)
	);
	for (size_t i = 0; i < commandCount; i++)
	{
		ThrowIfFailed(
			device->CreateCommandAllocator(
				D3D12_COMMAND_LIST_TYPE_DIRECT,
				IID_PPV_ARGS(m_commandAllocator[i].ReleaseAndGetAddressOf())
			));

		ThrowIfFailed(
			device->CreateCommandList(
				0,
				D3D12_COMMAND_LIST_TYPE_DIRECT,
				m_commandAllocator[i].Get(),
				nullptr,
				IID_PPV_ARGS(m_commandList[i].ReleaseAndGetAddressOf())
			));
		m_commandList[i]->Close();
	}

	ThrowIfFailed(
		device->CreateCommandAllocator(
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(m_textCommandAllocator.ReleaseAndGetAddressOf())
		));

	ThrowIfFailed(
		device->CreateCommandList(
			0,
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			m_textCommandAllocator.Get(),
			nullptr,
			IID_PPV_ARGS(m_textCommandList.ReleaseAndGetAddressOf())
		));
	m_textCommandList->Close();
	

	D3D12_RESOURCE_DESC texDesc = {};
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Width = 64*4;
	texDesc.Height = 64*4;
	texDesc.Format = textureFormat;
	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	texDesc.MipLevels = 1;
	texDesc.SampleDesc.Count = 1;
	texDesc.DepthOrArraySize = 1;
	texDesc.SampleDesc = { 1,0 };

	D3D12_CLEAR_VALUE clearValue;
	clearValue.Format = textureFormat;
	clearValue.Color[0] = 1.0f;
	clearValue.Color[1] = 1.0f;
	clearValue.Color[2] = 1.0f;
	clearValue.Color[3] = 1.0f;

	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		&clearValue,
		IID_PPV_ARGS(m_text.ReleaseAndGetAddressOf())
	));

	// -------------- Create text textures----------------------------------
	Graphics::utility->CreateDescriptorHeap(1, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, m_textRtvHeap, 0);
	Graphics::utility->CreateDescriptorHeap(1, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, m_textSrvHeap, 0, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = m_text->GetDesc().Format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = m_text->GetDesc().MipLevels;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.f;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;

	device->CreateShaderResourceView(m_text.Get(), &srvDesc, m_textSrvHeap->GetCPUDescriptorHandleForHeapStart());
	device->CreateRenderTargetView(m_text.Get(), nullptr, m_textRtvHeap->GetCPUDescriptorHandleForHeapStart());	
}

void FrameResource::UpdateGlobalConstantBuffer(
	const DirectX::SimpleMath::Vector3& viewDirection,
	const DirectX::SimpleMath::Vector3& viewLocation,
	const DirectX::SimpleMath::Matrix& view,
	const DirectX::SimpleMath::Matrix& proj
	)
{
	phongGC.viewDir = ToVector4(viewDirection, 0.f);
	phongGC.viewLoc = ToVector4(viewLocation, 0.f);
	phongGC.view = view;
	phongGC.proj = proj;

	memcpy(pPhongGCB, &phongGC, sizeof(PhongGlobalConstant));
}

void FrameResource::ResetAllocator(int idx)
{
	m_commandAllocator[idx]->Reset();
}
