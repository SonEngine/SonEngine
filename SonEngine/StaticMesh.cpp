#include "StaticMesh.h"
#include "TextureLoader.h"
#include "Vertex.h"
#include <vector>

StaticMesh::StaticMesh()
{

}

void StaticMesh::Render(ID3D12GraphicsCommandList* commandList, const TextureLoader * textureLoader)
{
	commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	commandList->IASetIndexBuffer(&m_indexBufferView);

	commandList->SetGraphicsRootDescriptorTable(0, textureLoader->GetGPUHandle(albedoTexture));
	commandList->DrawIndexedInstanced(m_indexCount, 1, 0, 0, 0);
}

void StaticMesh::Render(ID3D12GraphicsCommandList* commandList)
{
	commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	commandList->IASetIndexBuffer(&m_indexBufferView);

	commandList->DrawIndexedInstanced(m_indexCount, 1, 0, 0, 0);
}
