#include "FrameResource.h"
#include "GraphicsCommon.h"
#include "DirectXColors.h"
#include <iostream>

FrameResource::FrameResource()
{
	kEyeDir =
	{
		Vector4{  1,  0,  0, 0 }, Vector4{ -1,  0,  0, 0 },
		Vector4{  0,  1,  0, 0 }, Vector4{  0, -1,  0, 0 },
		Vector4{  0,  0,  1, 0 }, Vector4{  0,  0, -1, 0 }
	};
	kUpDir =
	{
		Vector4{ 0, 1, 0, 0 }, Vector4{ 0, 1, 0, 0 },
		Vector4{ 0, 0, -1, 0 }, Vector4{ 0, 0,  1, 0 },
		Vector4{ 0, 1, 0, 0 }, Vector4{ 0, 1,  0, 0 }
	};

}

void FrameResource::Initialize(ID3D12Device5* device, const UINT& width, const UINT& height, const UINT& textCount, HWND mainHwnd)
{
	hwnd = mainHwnd;
	if (Graphics::utility == nullptr)
	{
		std::cout << "Failed FrameResource::Initialize -> Graphics::utility == nullptr\n";
		return;
	}
	srvIncrementSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	rtvIncrementSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	Graphics::utility->CreateConstantBuffer(
		sizeof(PhongGlobalConstant),
		m_phongGCBuffer,
		reinterpret_cast<void**>(&pPhongGCB)
	);

	Graphics::utility->CreateConstantBuffer(
		sizeof(PBGlobalConstant),
		m_pbGCBuffer,
		reinterpret_cast<void**>(&pPBGCB)
	);

	for (size_t i = 0; i < commandCount; i++)
	{
		CreateCommand(device, m_commandAllocator[i], m_commandList[i]);
	}

	CreateCommand(device, m_textCommandAllocator, m_textCommandList);
	CreateCommand(device, m_guiCommandAllocator, m_guiCommandList);


	float fov = DirectX::XM_PIDIV2;
	DirectX::SimpleMath::Matrix projMatrix = DirectX::XMMatrixPerspectiveFovLH(
		fov, 1.f, 0.1f, 1000.f);

	m_pCubeGC.resize(6);
	for (int i = 0; i < 6; i++)
	{

		Graphics::utility->CreateConstantBuffer(
			sizeof(PhongGlobalConstant),
			m_cubeMapGCB[i],
			reinterpret_cast<void**>(&m_pCubeGC[i])
		);

		DirectX::SimpleMath::Matrix viewMatrix = XMMatrixLookToLH(Vector3(0, 0, 0), kEyeDir[i], kUpDir[i]);

		m_cubePhongGC[i].proj = projMatrix.Transpose();
		m_cubePhongGC[i].view = viewMatrix.Transpose();

		m_cubePhongGC[i].proj;
		memcpy(m_pCubeGC[i], &m_cubePhongGC[i], sizeof(PhongGlobalConstant));
	}
}

void FrameResource::AddLocalConstantBuffer(uint32_t id, const PrimitiveProxy& proxy)
{
	Microsoft::WRL::ComPtr<ID3D12Resource> buffer;
	void* pLB;
	Graphics::utility->CreateConstantBuffer(
		sizeof(LocalConstant),
		buffer,
		reinterpret_cast<void**>(&pLB)
	);

	LocalData ld;
	ld.localCB = buffer;
	ld.textureName = proxy.textureName;
	ld.psoName = proxy.psoName;
	m_localData[id] = ld;
	m_pCBs[id] = pLB;

	memcpy(pLB, &proxy.constant, sizeof(LocalConstant));
}

void FrameResource::UpdateLocalConstantBuffer(const LocalConstant& lc, uint32_t id)
{
	auto pLB = m_pCBs[id];
	memcpy(pLB, &lc, sizeof(LocalConstant));
}

void FrameResource::UpdateCubeGCView(const DirectX::SimpleMath::Vector3& loc)
{
	for (int i = 0; i < 6; i++)
	{
		DirectX::SimpleMath::Matrix viewMatrix = XMMatrixLookToLH(loc, kEyeDir[i], kUpDir[i]);
		m_cubePhongGC[i].view = viewMatrix.Transpose();

		memcpy(m_pCubeGC[i], &m_cubePhongGC[i], sizeof(PhongGlobalConstant));
	}
}

void FrameResource::UpdateGlobalConstantBuffer(const PhongGlobalConstant& pgc)
{
	phongGC = pgc;
	memcpy(pPhongGCB, &phongGC, sizeof(PhongGlobalConstant));
}

void FrameResource::UpdatePBGlobalConstantBuffer(const PBGlobalConstant& pbgc)
{
	pbGC = pbgc;
	memcpy(pPBGCB, &pbGC, sizeof(PBGlobalConstant));
}

void FrameResource::ResetAllocator(int idx)
{
	m_commandAllocator[idx]->Reset();
}

void FrameResource::CreateCommand(ID3D12Device5* device, Microsoft::WRL::ComPtr<ID3D12CommandAllocator>& alloc, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList)
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
