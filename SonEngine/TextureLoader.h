#pragma once

#include <string>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>
#include <map>
#include <unordered_map>
#include "wrl.h"
#include "d3d12.h"

struct TextureInfo {
	uint64_t offset;
	uint64_t size;
};

class TextureLoader {
public:
	TextureLoader(std::string path);

	void InitHeap(Microsoft::WRL::ComPtr<ID3D12Device5>& device);
	void LoadIdx(Microsoft::WRL::ComPtr<ID3D12Device5>& device);
	void LoadTextures(Microsoft::WRL::ComPtr<ID3D12Device5>& device,
		Microsoft::WRL::ComPtr<ID3D12CommandQueue>& commandQueue);

public:
	ID3D12DescriptorHeap* GetHeap()const { return heap.Get(); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(const int& idx) const;
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(const std::string& filename) const;
	std::vector<std::string> filenames;

private:
	std::string folder;
	std::string binPath;
	std::string idxPath;

	uint32_t count;

	std::unordered_map<std::string, TextureInfo> textureMap;
	std::unordered_map<uint32_t, std::string> nameMap;
	std::map<std::string, uint32_t> idxMap;

	

	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> textures;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> heap;

	UINT srvOffset = 0;
};