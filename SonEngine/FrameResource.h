#pragma once

#include "d3d12.h"
#include "wrl.h"
#include <vector>
#include "directxtk12/SimpleMath.h"

#include "Proxy.h"
#include "Constants.h"
#include "ViewProjInfo.h"
#include "PhongHLSLCompat.h"
#include "PaintBoardHLSLCompat.h"
#include "MouseInputState.h"

struct TextResource {
	UINT64 textureWidth;
	UINT64 textureHeight;

	Microsoft::WRL::ComPtr<ID3D12Resource> text;
};

class FrameResource {
public:
	FrameResource() {};

public:
	void Initialize(ID3D12Device5* device, const UINT& width, const UINT& height, const UINT& textCount, HWND mainHwnd);
	void UpdateGlobalConstantBuffer(const PhongGlobalConstant& pgc);
	void UpdatePBGlobalConstantBuffer(const PBGlobalConstant& pbgc);
	void UpdateGlobalConstantBuffer(const ViewProjInfo& viewProjInfo, const std::vector<LightInfo>& lightInfos);
	void UpdatePBGlobalConstantBuffer(const int& guiWidth, const MouseInputState& mouseInputState);
	
	
	void ResetAllocator(int idx);

	void CreateCommand(ID3D12Device5* device, Microsoft::WRL::ComPtr<ID3D12CommandAllocator>& alloc, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList);

public:
	D3D12_GPU_VIRTUAL_ADDRESS GetGCBGPUAddress() const { return m_phongGCBuffer->GetGPUVirtualAddress(); }
	D3D12_GPU_VIRTUAL_ADDRESS GetPBGCBGPUAddress() const { return m_pbGCBuffer->GetGPUVirtualAddress(); }

public:
	ID3D12CommandAllocator* GetAllocator(int idx) const { return m_commandAllocator[idx].Get(); }
	ID3D12GraphicsCommandList* GetCommandList(int idx) const { return m_commandList[idx].Get(); }
	
	ID3D12CommandAllocator* GetTextAllocator() const { return m_textCommandAllocator.Get(); }
	ID3D12GraphicsCommandList* GetTextCommandList() const { return m_textCommandList.Get(); }
	ID3D12CommandAllocator* GetGUIAllocator() const { return m_guiCommandAllocator.Get(); }
	ID3D12GraphicsCommandList* GetGUICommandList() const { return m_guiCommandList.Get(); }

	ID3D12DescriptorHeap* GetTextSrvHeap() const { return m_textSrvHeap.Get(); }
	ID3D12DescriptorHeap* GetTextRtvHeap() const { return m_textRtvHeap.Get(); }
	D3D12_CPU_DESCRIPTOR_HANDLE GetTextSrvCPUHandle(int idx) const { return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_textSrvHeap->GetCPUDescriptorHandleForHeapStart(), idx, srvIncrementSize); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetTextSrvGPUHandle(int idx) const { return CD3DX12_GPU_DESCRIPTOR_HANDLE(m_textSrvHeap->GetGPUDescriptorHandleForHeapStart(), idx, srvIncrementSize); }
	D3D12_CPU_DESCRIPTOR_HANDLE GetTextRrvCPUHandle(int idx) const {return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_textRtvHeap->GetCPUDescriptorHandleForHeapStart(), idx, rtvIncrementSize);}

private:
	PhongGlobalConstant phongGC;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_phongGCBuffer;
	void* pPhongGCB = nullptr;

	PBGlobalConstant pbGC;
	// paint board global constant buffer
	Microsoft::WRL::ComPtr<ID3D12Resource> m_pbGCBuffer;
	void* pPBGCB = nullptr;


private:
	HWND hwnd;

private:
	const static int commandCount = 3;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_commandAllocator[commandCount];
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList[commandCount];
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_textCommandAllocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_textCommandList;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_guiCommandAllocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_guiCommandList;

public:
	UINT64 m_currentFence = 0;
	bool proxyDirty = true;
	std::vector<Proxy> proxyBuffer;
	std::vector<Proxy> pcProxyBuffer;
	std::vector<Proxy> dotProxyBuffer;
	std::vector<TextProxy> textProxyBuffer;
	std::vector<TextResource> textResources;

private:
	
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_textSrvHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_textRtvHeap;

	UINT srvIncrementSize = 0;
	UINT rtvIncrementSize = 0;

};