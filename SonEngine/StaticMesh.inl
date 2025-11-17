#pragma once

#include "StaticMesh.h"
#include "GraphicsCommon.h"


template<typename V, typename I>
inline void StaticMesh::Initialize(ID3D12Device5* device, ID3D12GraphicsCommandList* commandList, Mesh<V, I>& mesh)
{
	m_indexCount = (UINT)mesh.m_indices.size();

	Graphics::utility->CreateBuffer<V>(mesh.m_vertices, m_vertexGpu, m_vertexUpload);
	Graphics::utility->CreateBuffer<I>(mesh.m_indices, m_indexGpu, m_indexUpload);

	m_vertexBufferView.BufferLocation = m_vertexGpu->GetGPUVirtualAddress();
	m_vertexBufferView.SizeInBytes = (UINT)(mesh.m_vertices.size() * sizeof(V));
	m_vertexBufferView.StrideInBytes = (UINT)(sizeof(V));

	m_indexBufferView.BufferLocation = m_indexGpu->GetGPUVirtualAddress();
	m_indexBufferView.Format = ((sizeof(I) == sizeof(uint16_t)) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT);
	m_indexBufferView.SizeInBytes = (UINT)(mesh.m_indices.size() * sizeof(I));

	Graphics::utility->CreateConstantBuffer(
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
