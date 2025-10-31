#pragma once

#include "d3d12.h"
#include "wrl.h"
#include <vector>
#include "directxtk12/SimpleMath.h"

#include "Proxy.h"
#include "Constants.h"

struct TextResource {
	UINT64 textureWidth;
	UINT64 textureHeight;

	Microsoft::WRL::ComPtr<ID3D12Resource> text;
};

class FrameResource {
public:
	FrameResource() {};

public:
	void Initialize(Microsoft::WRL::ComPtr<ID3D12Device5>& device, const UINT& width = 2048, const UINT& height = 2048, const UINT& textCount = 0);
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
	D3D12_CPU_DESCRIPTOR_HANDLE GetTextSrvCPUHandle(int idx) const { return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_textSrvHeap->GetCPUDescriptorHandleForHeapStart(), idx, srvIncrementSize); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetTextSrvGPUHandle(int idx) const { return CD3DX12_GPU_DESCRIPTOR_HANDLE(m_textSrvHeap->GetGPUDescriptorHandleForHeapStart(), idx, srvIncrementSize); }
	D3D12_CPU_DESCRIPTOR_HANDLE GetTextRrvCPUHandle(int idx) const {return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_textRtvHeap->GetCPUDescriptorHandleForHeapStart(), idx, rtvIncrementSize);}

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
	std::vector<TextResource> textResources;

private:
	
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_textSrvHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_textRtvHeap;

	UINT srvIncrementSize = 0;
	UINT rtvIncrementSize = 0;
};