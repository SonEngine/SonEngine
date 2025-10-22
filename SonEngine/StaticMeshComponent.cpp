#include "StaticMeshComponent.h"
#include "Actor.h"

StaticMeshComponent::StaticMeshComponent(Actor* owner)
	:PrimitiveComponent(owner)
{
}

void StaticMeshComponent::Render(ID3D12GraphicsCommandList* commandList, const TextureLoader* texLoader)
{
	if (m_mesh)
	{
		m_mesh->Render(commandList, texLoader);
	}
}

void StaticMeshComponent::SetMesh(std::shared_ptr<StaticMesh> newMesh)
{
	m_mesh = newMesh;
}
