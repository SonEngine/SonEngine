#include "StaticMeshComponent.h"
#include "Actor.h"

StaticMeshComponent::StaticMeshComponent(Actor* owner)
	:PrimitiveComponent(owner)
{
}

StaticMeshComponent::~StaticMeshComponent()
{
}

void StaticMeshComponent::SetMesh(std::shared_ptr<StaticMesh> newMesh)
{
	m_mesh = newMesh;
}

