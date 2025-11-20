#include "StaticMeshProxy.h"


StaticMeshProxy::StaticMeshProxy()
	:PrimitiveProxy()
{
	
}

StaticMeshProxy::~StaticMeshProxy()
{
}


void StaticMeshProxy::Draw(ID3D12GraphicsCommandList* commandList)
{
	commandList->IASetVertexBuffers(0, 1, &staticMesh->GetVertexBufferView());
	commandList->IASetIndexBuffer(&staticMesh->GetIndexBufferView());
	commandList->SetGraphicsRootConstantBufferView(1, localCB->GetGPUVirtualAddress());

	commandList->DrawIndexedInstanced(staticMesh->GetIndexCount(), 1, 0, 0, 0);
}
