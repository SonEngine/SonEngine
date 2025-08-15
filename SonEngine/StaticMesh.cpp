#include "StaticMesh.h"

#include "Vertex.h"
#include <vector>

StaticMesh::StaticMesh()
{
}

void StaticMesh::Initialize(ID3D12Device5* device, ID3D12GraphicsCommandList* commandList, GraphicsUtils::Utility* utility)
{
	// TODO : Utility 함수화

	std::vector<SimpleVertex> vertices{
		{Vector3(-1, -1, 1), Vector2(0, 1)},
		{Vector3(-1, 1, 2), Vector2(0, 0)},
		{Vector3(1, 1, 2), Vector2(1, 0)},
		{Vector3(1, -1, 1), Vector2(1, 1)}
	};
	std::vector<uint16_t> indices{
		0, 1, 2, 0, 2, 3
	};

	m_indexCount = (UINT)indices.size();

	utility->CreateBuffer<SimpleVertex>(vertices, m_vertexGpu, m_vertexUpload);
	utility->CreateBuffer<uint16_t>(indices, m_indexGpu, m_indexUpload);
	
	m_vertexBufferView.BufferLocation = m_vertexGpu->GetGPUVirtualAddress();
	m_vertexBufferView.SizeInBytes = (UINT)(vertices.size() * sizeof(SimpleVertex));
	m_vertexBufferView.StrideInBytes = (UINT)(sizeof(SimpleVertex));

	m_indexBufferView.BufferLocation = m_indexGpu->GetGPUVirtualAddress();
	m_indexBufferView.Format = DXGI_FORMAT_R16_UINT;
	m_indexBufferView.SizeInBytes = (UINT)(indices.size() * sizeof(uint16_t));
}

void StaticMesh::Render(ID3D12GraphicsCommandList* commandList)
{
	commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	commandList->IASetIndexBuffer(&m_indexBufferView);

	commandList->DrawIndexedInstanced(m_indexCount, 1, 0, 0, 0);
}
