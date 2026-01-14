#pragma once

#include "d3d12.h"
#include "wrl.h"
#include <vector>
#include <array>

#include "directxtk12/SimpleMath.h"

#include "Proxy.h"

#include "ViewProjInfo.h"
#include "PBRHLSLCompat.h"
#include "PaintBoardHLSLCompat.h"
#include "MouseInputState.h"

// 각 mesh별 data
struct LocalData {
	// local constant buffer
	Microsoft::WRL::ComPtr<ID3D12Resource> localCB;
	std::string textureName;
	std::string psoName;
};

struct TextResource {
	UINT64 textureWidth;
	UINT64 textureHeight;

	Microsoft::WRL::ComPtr<ID3D12Resource> text;
};

class FrameResource {
public:
	FrameResource();

public:
	//void Initialize(ID3D12Device5* device, const UINT& width, const UINT& height, const UINT& textCount, HWND mainHwnd);
	void Initialize(ID3D12Device5* device, const UINT& width, const UINT& height, const UINT& textCount, HWND mainHwnd, const std::vector<std::shared_ptr<PBRLightInfo>>& lightInfos);
	void AddLocalConstantBuffer(uint32_t id, const PrimitiveProxy& proxy);
	void UpdateLocalConstantBuffer(const LocalConstant& lc, uint32_t id);
	void UpdateCubeGCView(const DirectX::SimpleMath::Vector3& loc);
	void UpdateGlobalConstantBuffer(const PBRGlobalConstant& pgc);
	void UpdatePBGlobalConstantBuffer(const PBGlobalConstant& pbgc);
	//void UpdateGlobalConstantBuffer(const ViewProjInfo& viewProjInfo, const std::vector<PBRLightInfo>& lightInfos);
	void UpdatePBGlobalConstantBuffer(const int& guiWidth, const MouseInputState& mouseInputState);
	
	
	void ResetAllocator(int idx);

	void CreateCommand(ID3D12Device5* device, Microsoft::WRL::ComPtr<ID3D12CommandAllocator>& alloc, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList);

public:
	D3D12_GPU_VIRTUAL_ADDRESS GetGCBGPUAddress() const { return m_pbrGCBuffer->GetGPUVirtualAddress(); }
	D3D12_GPU_VIRTUAL_ADDRESS GetPBGCBGPUAddress() const { return m_pbGCBuffer->GetGPUVirtualAddress(); }
	D3D12_GPU_VIRTUAL_ADDRESS GetCubeGCBGPUAddress(int idx) const { return m_cubeMapGCB[idx]->GetGPUVirtualAddress(); }
	D3D12_GPU_VIRTUAL_ADDRESS GetLightGCBGPUAddress(int idx) const { return m_lightGCB[idx]->GetGPUVirtualAddress(); }

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
	PBRGlobalConstant pbrGC;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_pbrGCBuffer;
	void* pPbrGCB = nullptr;

	PBGlobalConstant pbGC;
	// paint board global constant buffer
	Microsoft::WRL::ComPtr<ID3D12Resource> m_pbGCBuffer;
	void* pPBGCB = nullptr;

	PBRGlobalConstant m_cubePhongGC[6];
	Microsoft::WRL::ComPtr<ID3D12Resource> m_cubeMapGCB[6];
	std::vector<void*> m_pCubeGC;

	PBRGlobalConstant m_lightGC[NUM_LIGHTS];
	Microsoft::WRL::ComPtr<ID3D12Resource> m_lightGCB[NUM_LIGHTS];
	std::vector<void*> m_pLightGC;


public:
	std::array<DirectX::SimpleMath::Vector4, 6> kEyeDir;
	std::array<DirectX::SimpleMath::Vector4, 6> kUpDir;

private:
	HWND hwnd;

private:
	const static int commandCount = 10;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_commandAllocator[commandCount];
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList[commandCount];
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_textCommandAllocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_textCommandList;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_guiCommandAllocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_guiCommandList;

public:
	UINT64 m_currentFence = 0;

	std::unordered_map<uint32_t, LocalData> m_localData;
	std::unordered_map<uint32_t, void*> m_pCBs;

private:
	
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_textSrvHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_textRtvHeap;

	UINT srvIncrementSize = 0;
	UINT rtvIncrementSize = 0;

};