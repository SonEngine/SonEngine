#include "TextureLoader.h"

#include "Directxtk12/DDSTextureLoader.h"
#include "directxtk12/ResourceUploadBatch.h"

#include "Renderer.h"
#include "Utility.h"

namespace fs = std::filesystem;


std::ostream& operator<<(std::ostream& out, const TextureInfo& info)
{
	out << ", Offset : " << info.offset << " , size : " << info.size;
	out << '\n';

	return out;
}

TextureLoader::TextureLoader()
	:m_device(nullptr)
{
}

TextureLoader::TextureLoader(std::string path, ID3D12Device5* device)
	:folder(path),
	m_device(device)
{
	count = 0;
	binPath = folder + "textures.bin";
	idxPath = folder + "textures.idx";
}

void TextureLoader::InitHeap(UINT heapSize)
{
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.NumDescriptors = heapSize;
	m_heapSize = heapSize;

	m_device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(heap.GetAddressOf()));
}

void TextureLoader::LoadIdx()
{
	std::ifstream idx(idxPath.c_str(), std::ios::binary);

	idx.read(reinterpret_cast<char*>(&count), sizeof(count));

	std::cout << "dds count : " << count << '\n';

	for (uint32_t i = 0; i < count; i++)
	{
		TextureInfo info;
		uint32_t nameLen;

		idx.read(reinterpret_cast<char*>(&nameLen), sizeof(nameLen));
		std::string filename(nameLen, ' ');

		idx.read(filename.data(), nameLen);
		idx.read(reinterpret_cast<char*>(&info.offset), sizeof(info.offset));
		idx.read(reinterpret_cast<char*>(&info.size), sizeof(info.size));

		//std::cout << "File name : " << filename << info;
		textureMap[filename] = info;
		nameMap[i] = filename;
		idxMap[filename] = i;
		filenames.push_back(filename);
	}
}

void TextureLoader::LoadTextures(Microsoft::WRL::ComPtr<ID3D12CommandQueue>& commandQueue)
{
	std::ifstream bin(binPath, std::ios::binary);

	CD3DX12_CPU_DESCRIPTOR_HANDLE handle(heap->GetCPUDescriptorHandleForHeapStart());
	srvOffset = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	DirectX::ResourceUploadBatch resourceUpload(m_device);
	resourceUpload.Begin();

	for (uint32_t i = 0; i < count; i++)
	{
		std::string filename = nameMap[i];
		
		bool isCubeMap = filename.find("CubeMap") != std::string::npos;

		TextureInfo info = textureMap[filename];
		uint64_t size = info.size;
		std::vector<uint8_t> texture(size);
		bin.seekg(info.offset);
		bin.read(reinterpret_cast<char*>(texture.data()), size);

		// 1. texture2d를 만든다
		// 2. heap에 view를 만든다
		
		Microsoft::WRL::ComPtr<ID3D12Resource> t;
		ThrowIfFailed(DirectX::CreateDDSTextureFromMemoryEx(
			m_device,
			resourceUpload,
			reinterpret_cast<const uint8_t*>(texture.data()),
			size,
			0,
			D3D12_RESOURCE_FLAG_NONE,
			DirectX::DX12::DDS_LOADER_FORCE_SRGB,
			t.GetAddressOf()));

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = t->GetDesc().Format;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Texture2D.MipLevels = t->GetDesc().MipLevels;
		srvDesc.Texture2D.ResourceMinLODClamp = 0.f;
		if(isCubeMap)
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		else
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		
		m_device->CreateShaderResourceView(t.Get(), &srvDesc, handle);
		textures.push_back(t);

		handle.Offset(1, srvOffset);
	}
	auto uploadResourcesFinished = resourceUpload.End(commandQueue.Get());
	uploadResourcesFinished.wait();
}

void TextureLoader::AddTexture(Microsoft::WRL::ComPtr<ID3D12Resource> & texture, std::string & filename, bool isCubeMap)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = texture->GetDesc().Format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	if (isCubeMap)
	{
		srvDesc.TextureCube.MipLevels = texture->GetDesc().MipLevels;
		srvDesc.TextureCube.MostDetailedMip = 0;
		srvDesc.TextureCube.ResourceMinLODClamp = 0.f;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	}
	else
	{
		srvDesc.Texture2D.MipLevels = texture->GetDesc().MipLevels;
		srvDesc.Texture2D.ResourceMinLODClamp = 0.f;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	}	

	auto it = idxMap.find(filename);
	if (it != idxMap.end())
	{
		int idx = it->second;
	
		CD3DX12_CPU_DESCRIPTOR_HANDLE handle(heap->GetCPUDescriptorHandleForHeapStart(), idx, srvOffset);
		m_device->CreateShaderResourceView(texture.Get(), &srvDesc, handle);

		textures[idx] = texture;
	}
	else if (count <= m_heapSize)
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE handle(heap->GetCPUDescriptorHandleForHeapStart(), count, srvOffset);
		m_device->CreateShaderResourceView(texture.Get(), &srvDesc, handle);
		idxMap[filename] = count;
		textures.push_back(texture);
		count++;
	}
	else
	{
		std::cout << "Failed to AddTexture : 힙이 가득 찾습니다\n";
		return;
	}
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureLoader::GetGPUHandle(const int & idx) const
{
	CD3DX12_GPU_DESCRIPTOR_HANDLE handle(heap->GetGPUDescriptorHandleForHeapStart(), idx, srvOffset);
	return handle;
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureLoader::GetGPUHandle(const std::string & filename) const
{
	uint32_t idx = 0;
	auto it = idxMap.find(filename);
	if (it != idxMap.end())
	{
		idx = it->second;
	}
	CD3DX12_GPU_DESCRIPTOR_HANDLE handle(heap->GetGPUDescriptorHandleForHeapStart(), idx, srvOffset);
	return handle;
}

ID3D12Resource* TextureLoader::GetTexture(const std::string& filename) const
{
	const std::string test;
	uint32_t idx = 0;
	auto it = idxMap.find(filename);
	if (it != idxMap.end())
	{
		idx = it->second;
	}
	return textures[idx].Get();
}
