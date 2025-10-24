#include "StaticMesh.h"
#include "TextureLoader.h"
#include "Vertex.h"


StaticMesh::StaticMesh()
{

}

void StaticMesh::Render(ID3D12GraphicsCommandList* commandList, const TextureLoader * textureLoader)
{
	commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	commandList->IASetIndexBuffer(&m_indexBufferView);
	commandList->SetGraphicsRootConstantBufferView(1, m_localCB->GetGPUVirtualAddress());
	
	commandList->SetGraphicsRootDescriptorTable(0, textureLoader->GetGPUHandle(albedoTexture));
	commandList->DrawIndexedInstanced(m_indexCount, 1, 0, 0, 0);
}

void StaticMesh::Render(ID3D12GraphicsCommandList* commandList)
{
	commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	commandList->IASetIndexBuffer(&m_indexBufferView);
	commandList->SetGraphicsRootConstantBufferView(1, m_localCB->GetGPUVirtualAddress());

	commandList->DrawIndexedInstanced(m_indexCount, 1, 0, 0, 0);
} 

void StaticMesh::SetLocation(const float& x, const float& y, const float& z)
{
	localConstant.model.m[3][0] = x;
	localConstant.model.m[3][1] = y;
	localConstant.model.m[3][2] = z;

	memcpy(pLocalConstant, &localConstant, sizeof(LocalConstant));
}

void StaticMesh::Translate(const float& delX, const float& delY, const float& delZ)
{
	localConstant.model.m[3][0] += delX;
	localConstant.model.m[3][1] += delY;
	localConstant.model.m[3][2] += delZ;

	memcpy(pLocalConstant, &localConstant, sizeof(LocalConstant));
}
