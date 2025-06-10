#include "StaticMesh.h"
#include "Utility.h"
#include "Vertex.h"
#include <vector>

StaticMesh::StaticMesh()
{
}

void StaticMesh::Initialize(ID3D12Device5* device, ID3D12GraphicsCommandList* commandList)
{

	std::vector<PositionVertex> vertices{
		{Vector3(-1, -1, 1)},
		{Vector3(0, 1, 1)},
		{Vector3(1, -1, 1)}
	};
	std::vector<uint16_t> indices{
		0,1,2
	};

	m_indexCount = indices.size();

	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(vertices.size() * sizeof(PositionVertex)),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_vertexUpload)
	));

	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(vertices.size() * sizeof(PositionVertex)),
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&m_vertexGpu)
	));

	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(indices.size() * sizeof(uint16_t)),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_indexUpload)
	));

	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(indices.size() * sizeof(uint16_t)),
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&m_indexGpu)
	));

	D3D12_SUBRESOURCE_DATA subData;
	subData.pData = vertices.data();
	subData.RowPitch = vertices.size() * sizeof(PositionVertex);
	subData.SlicePitch = subData.RowPitch;

	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_vertexGpu.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));

	UpdateSubresources<1>(commandList, m_vertexGpu.Get(), m_vertexUpload.Get(), 0, 0, 1, &subData);
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_vertexGpu.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));

	D3D12_SUBRESOURCE_DATA subData2;
	subData2.pData = indices.data();
	subData2.RowPitch = indices.size() * sizeof(uint16_t);
	subData2.SlicePitch = subData2.RowPitch;

	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_indexGpu.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));

	UpdateSubresources<1>(commandList, m_indexGpu.Get(), m_indexUpload.Get(), 0, 0, 1, &subData2);
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_indexGpu.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));


	m_vertexBufferView.BufferLocation = m_vertexGpu->GetGPUVirtualAddress();
	m_vertexBufferView.SizeInBytes = vertices.size() * sizeof(PositionVertex);
	m_vertexBufferView.StrideInBytes = sizeof(PositionVertex);

	m_indexBufferView.BufferLocation = m_indexGpu->GetGPUVirtualAddress();
	m_indexBufferView.Format = DXGI_FORMAT_R16_UINT;
	m_indexBufferView.SizeInBytes = indices.size() * sizeof(uint16_t);
}

void StaticMesh::Render(ID3D12GraphicsCommandList* commandList)
{
	commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	commandList->IASetIndexBuffer(&m_indexBufferView);

	commandList->DrawIndexedInstanced(m_indexCount, 1, 0, 0, 0);
}
