#include "FrameResource.h"
#include "GraphicsCommon.h"
#include <iostream>

void FrameResource::Initialize(Microsoft::WRL::ComPtr<ID3D12Device5>& device) {

	if (Graphics::utility == nullptr)
	{
		std::cout << "Failed FrameResource::Initialize -> Graphics::utility == nullptr\n";
	}
	Graphics::utility->CreateConstantBuffer(
		sizeof(GlobalConstant),
		m_phongGCBuffer,
		reinterpret_cast<void**>(&pPhongGCB)
	);

	ThrowIfFailed(
		device->CreateCommandAllocator(
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(m_commandAllocator.ReleaseAndGetAddressOf())
		));
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

void FrameResource::ResetAllocator()
{
	m_commandAllocator->Reset();
}
