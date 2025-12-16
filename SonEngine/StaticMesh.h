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
	virtual ~StaticMesh();

	template<typename V, typename I>
	void Initialize(ID3D12Device5* device, ID3D12GraphicsCommandList* commandList, Mesh<V,I>& mesh);

	// PointCloud 초기화
	template<typename V, typename I>
	void InitializePC(ID3D12Device5* device, ID3D12GraphicsCommandList* commandList, const std::vector<Mesh<V, I>>& meshes);

	template<typename V, typename I>
	void Initialize(ID3D12Device5* device, ID3D12GraphicsCommandList* commandList, const std::vector<Mesh<V, I>>& meshes);

	void Render(ID3D12GraphicsCommandList* commandList, const TextureLoader* textureLoader);
	void Render(ID3D12GraphicsCommandList* commandList);
	
	void SetAlbedoTexture(const std::string& filename) { albedoTexture = filename; }
	
	void RenderPoints(ID3D12GraphicsCommandList* commandList, int cbIdx /* constant Buffer Index*/);

	void RenderPoints(ID3D12GraphicsCommandList* commandList, int cbIdx, const TextureLoader* textureLoader);

	void SetLocation(const float& x, const float& y, const float& z);
	void Translate(const float& delX, const float& delY, const float& delZ);

	void SetRotation(const DirectX::SimpleMath::Matrix& mat);

public:
	D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView(int index = 0) const { return m_vertexBufferViews[index]; }
	D3D12_INDEX_BUFFER_VIEW GetIndexBufferView(int index = 0)const { return m_indexBufferViews[index]; }
	UINT GetIndexCount(int index = 0)const { return m_indexCounts[index]; }

	std::string GetAlbedoTextureName() const { return albedoTexture; }

protected:
	std::vector <Microsoft::WRL::ComPtr<ID3D12Resource>> m_vertexGpu;
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> m_vertexUpload;
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> m_indexGpu;
	std::vector <Microsoft::WRL::ComPtr<ID3D12Resource>> m_indexUpload;

	std::vector<D3D12_VERTEX_BUFFER_VIEW> m_vertexBufferViews;
	std::vector<D3D12_INDEX_BUFFER_VIEW> m_indexBufferViews;

	std::vector<TextureGPUResource> GPUResources;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_texturesHeap;

	std::vector<UINT> m_indexCounts;
	std::vector<UINT> m_vertexCounts;
	UINT meshCount = 0;

	std::string albedoTexture;

protected:
	void* pLocalConstant = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_localCB;
	LocalConstant localConstant;

	DirectX::SimpleMath::Matrix localRot;
	DirectX::SimpleMath::Vector3 localLocation;
};

#include "StaticMesh.inl"