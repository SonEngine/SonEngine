#include "FrameResource.h"
#include "GraphicsCommon.h"
#include "DirectXColors.h"
#include <iostream>

void FrameResource::Initialize(ID3D12Device5* device, const UINT& width, const UINT& height, const UINT& textCount)
{
	if (Graphics::utility == nullptr)
	{
		std::cout << "Failed FrameResource::Initialize -> Graphics::utility == nullptr\n";
		return;
	}
	srvIncrementSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	rtvIncrementSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	Graphics::utility->CreateConstantBuffer(
		sizeof(GlobalConstant),
		m_phongGCBuffer,
		reinterpret_cast<void**>(&pPhongGCB)
	);
	for (size_t i = 0; i < commandCount; i++)
	{
		CreateCommand(device, m_commandAllocator[i], m_commandList[i]);
	}

	CreateCommand(device, m_textCommandAllocator, m_textCommandList);
	CreateCommand(device, m_guiCommandAllocator, m_guiCommandList);
		

	if (textCount == 0)
	{
		return;
	}		

	Graphics::utility->CreateDescriptorHeap(textCount, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, m_textRtvHeap, 0);
	Graphics::utility->CreateDescriptorHeap(textCount, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, m_textSrvHeap, 0, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
	CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle(m_textSrvHeap->GetCPUDescriptorHandleForHeapStart());
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_textRtvHeap->GetCPUDescriptorHandleForHeapStart());
	
	for (UINT i = 0; i < textCount; i++)
	{
		TextResource resource;
		resource.textureWidth = width;
		resource.textureHeight = height;


		D3D12_RESOURCE_DESC texDesc = {};
		texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		texDesc.Width = width;
		texDesc.Height = height;
		texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		texDesc.MipLevels = 1;
		texDesc.SampleDesc.Count = 1;
		texDesc.DepthOrArraySize = 1;
		texDesc.SampleDesc = { 1,0 };

		D3D12_CLEAR_VALUE clearValue;
		clearValue.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
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
			IID_PPV_ARGS(resource.text.ReleaseAndGetAddressOf())
		));

		// -------------- Create text textures----------------------------------


		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = resource.text->GetDesc().Format;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Texture2D.MipLevels = resource.text->GetDesc().MipLevels;
		srvDesc.Texture2D.ResourceMinLODClamp = 0.f;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;

		device->CreateShaderResourceView(resource.text.Get(), &srvDesc, srvHandle);
		device->CreateRenderTargetView(resource.text.Get(), nullptr, rtvHandle);

		textResources.push_back(resource);

		srvHandle.Offset(1, srvIncrementSize);
		rtvHandle.Offset(1, rtvIncrementSize);
	}
		

}

void FrameResource::UpdateGlobalConstantBuffer(const ViewProjInfo& viewProjInfo, const std::vector<LightInfo> & lightInfos)
{
	phongGC.cameraDir = ToVector4(viewProjInfo.viewDirection, 0.f);
	phongGC.cameraPos = ToVector4(viewProjInfo.viewLocation, 0.f);
	phongGC.view = viewProjInfo.view.Transpose();
	phongGC.proj = viewProjInfo.proj.Transpose();

	for (size_t i = 0; i < lightInfos.size(); i++)
	{
		phongGC.lights[i].direction = lightInfos[0].direction;
		phongGC.lights[i].location = lightInfos[0].location;
		phongGC.lights[i].brightness = lightInfos[0].brightness;
	}
	
	memcpy(pPhongGCB, &phongGC, sizeof(PhongGlobalConstant));
}

void FrameResource::ResetAllocator(int idx)
{
	m_commandAllocator[idx]->Reset();
}

void FrameResource::CreateCommand(ID3D12Device5* device, Microsoft::WRL::ComPtr<ID3D12CommandAllocator> & alloc, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> & commandList)
{
	ThrowIfFailed(
		device->CreateCommandAllocator(
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(alloc.ReleaseAndGetAddressOf())
		));

	ThrowIfFailed(
		device->CreateCommandList(
			0,
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			alloc.Get(),
			nullptr,
			IID_PPV_ARGS(commandList.ReleaseAndGetAddressOf())
		));
	commandList->Close();
}
