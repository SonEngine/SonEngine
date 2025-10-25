#pragma once

#include "d3d12.h"
#include "wrl.h"
#include <vector>
#include "directxtk12/SimpleMath.h"

#include "Proxy.h"
#include "Constants.h"

class FrameResource {
public:
	FrameResource() {};

public:
	void Initialize(Microsoft::WRL::ComPtr<ID3D12Device5>& device);
	void UpdateGlobalConstantBuffer(const DirectX::SimpleMath::Vector3& viewDirection, const DirectX::SimpleMath::Vector3& viewLocation, const DirectX::SimpleMath::Matrix& view, const DirectX::SimpleMath::Matrix& proj);
	void ResetAllocator();

public:
	D3D12_GPU_VIRTUAL_ADDRESS GetGCBGPUAddress() const { return m_phongGCBuffer->GetGPUVirtualAddress(); }

public:
	ID3D12CommandAllocator* GetAllocator() const { return m_commandAllocator.Get(); }
	ID3D12GraphicsCommandList* GetCommandList() const { return m_commandList.Get(); }

private:
	PhongGlobalConstant phongGC;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_phongGCBuffer;
	void* pPhongGCB = nullptr;

private:
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_commandAllocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;

public:
	UINT64 m_currentFence = 0;
	std::vector<Proxy> proxyBuffer;
};