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
	void Initialize(Microsoft::WRL::ComPtr<ID3D12Device5>& device, UINT64 width = 2048, UINT64 height = 2048);
	void UpdateGlobalConstantBuffer(const DirectX::SimpleMath::Vector3& viewDirection, const DirectX::SimpleMath::Vector3& viewLocation, const DirectX::SimpleMath::Matrix& view, const DirectX::SimpleMath::Matrix& proj);
	void ResetAllocator(int idx);

public:
	D3D12_GPU_VIRTUAL_ADDRESS GetGCBGPUAddress() const { return m_phongGCBuffer->GetGPUVirtualAddress(); }

public:
	ID3D12CommandAllocator* GetAllocator(int idx) const { return m_commandAllocator[idx].Get(); }
	ID3D12GraphicsCommandList* GetCommandList(int idx) const { return m_commandList[idx].Get(); }
	
	ID3D12CommandAllocator* GetTextAllocator() const { return m_textCommandAllocator.Get(); }
	ID3D12GraphicsCommandList* GetTextCommandList() const { return m_textCommandList.Get(); }

	ID3D12DescriptorHeap* GetTextSrvHeap() const { return m_textSrvHeap.Get(); }
	ID3D12DescriptorHeap* GetTextRtvHeap() const { return m_textRtvHeap.Get(); }

private:
	PhongGlobalConstant phongGC;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_phongGCBuffer;
	void* pPhongGCB = nullptr;

private:
	const static int commandCount = 2;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_commandAllocator[commandCount];
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList[commandCount];
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_textCommandAllocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_textCommandList;
public:
	UINT64 m_currentFence = 0;
	bool proxyDirty = true;
	std::vector<Proxy> proxyBuffer;
	std::vector<TextProxy> textProxyBuffer;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_text;

private:
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_textSrvHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_textRtvHeap;

	DXGI_FORMAT textureFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	UINT64 textureWidth;
	UINT64 textureHeight;
};