#pragma once

#include "d3d12.h"
#include "directx/d3dx12.h"
#include "wrl.h"
#include <vector>

#include "GraphicsCommon.h"
#include "Mesh.h"
#include "TextureGPUResource.h"
#include "Constants.h"

class TextureLoader;

class StaticMesh {

public:

	StaticMesh();
	 
	template<typename V, typename I>
	void Initialize(ID3D12Device5* device, ID3D12GraphicsCommandList* commandList, Mesh<V,I>& mesh);

	void Render(ID3D12GraphicsCommandList* commandList, const TextureLoader* textureLoader);
	void Render(ID3D12GraphicsCommandList* commandList);
	
	void SetAlbedoTexture(const std::string& filename) { albedoTexture = filename; }
	
	void SetLocation(const float& x, const float& y, const float& z);
	void Translate(const float& delX, const float& delY, const float& delZ);

	void SetRotation(const DirectX::SimpleMath::Matrix& mat);

	std::string GetAlbedoTextureName() const { return albedoTexture; }

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexGpu;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexUpload;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_indexGpu;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_indexUpload;

	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW m_indexBufferView;

	std::vector<TextureGPUResource> GPUResources;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_texturesHeap;

	UINT m_indexCount = 0;

	std::string albedoTexture;

private:
	void* pLocalConstant = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_localCB;
	LocalConstant localConstant;

	DirectX::SimpleMath::Matrix localRot;
	DirectX::SimpleMath::Vector3 localLocation;
};

template<typename V, typename I>
inline void StaticMesh::Initialize(ID3D12Device5* device, ID3D12GraphicsCommandList* commandList, Mesh<V, I>& mesh)
{
	m_indexCount = (UINT)mesh.m_indices.size();

	utility->CreateBuffer<V>(mesh.m_vertices, m_vertexGpu, m_vertexUpload);
	utility->CreateBuffer<I>(mesh.m_indices, m_indexGpu, m_indexUpload);

	m_vertexBufferView.BufferLocation = m_vertexGpu->GetGPUVirtualAddress();
	m_vertexBufferView.SizeInBytes = (UINT)(mesh.m_vertices.size() * sizeof(V));
	m_vertexBufferView.StrideInBytes = (UINT)(sizeof(V));

	m_indexBufferView.BufferLocation = m_indexGpu->GetGPUVirtualAddress();
	m_indexBufferView.Format = ((sizeof(I) == sizeof(uint16_t)) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT);
	m_indexBufferView.SizeInBytes = (UINT)(mesh.m_indices.size() * sizeof(I));

	utility->CreateConstantBuffer(
		sizeof(LocalConstant),
		m_localCB,
		reinterpret_cast<void**>(&pLocalConstant)
	);

	memcpy(
		pLocalConstant,
		&localConstant,
		sizeof(LocalConstant)
	);

}


