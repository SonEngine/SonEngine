#include "StaticMesh.h"

#include "Vertex.h"
#include <vector>

StaticMesh::StaticMesh()
{
}

void StaticMesh::Render(ID3D12GraphicsCommandList* commandList)
{
	commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	commandList->IASetIndexBuffer(&m_indexBufferView);

	// TODO : texture랑 heap mesh에 추가하기
	/*ID3D12DescriptorHeap* heaps[] = {
		m_texturesHeap.Get()
	};

	commandList->SetDescriptorHeaps(1, heaps);
	commandList->SetGraphicsRootDescriptorTable(0, m_texturesHeap->GetGPUDescriptorHandleForHeapStart());
*/

	commandList->DrawIndexedInstanced(m_indexCount, 1, 0, 0, 0);
}
