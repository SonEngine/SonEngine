#pragma once

#include "d3d12.h"
#include "d3dx12.h"
#include "wrl.h"

class StaticMesh {

public:

	StaticMesh();

	void Initialize(ID3D12Device5* device, ID3D12GraphicsCommandList* commandList);
	void Render(ID3D12GraphicsCommandList* commandList);

private:

	Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexGpu;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexUpload;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_indexGpu;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_indexUpload;

	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW m_indexBufferView;

	UINT m_indexCount = 0;
};