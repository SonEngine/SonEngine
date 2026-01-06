#include "DotComponent.h"
#include "Actor.h"
#include "StaticMesh.h"


DotComponent::DotComponent(Actor* owner)
	:PrimitiveComponent(owner)
{
	m_mesh = std::shared_ptr<StaticMesh>();
}

DotComponent::~DotComponent()
{
}



void DotComponent::SetMesh(std::shared_ptr<StaticMesh> newMesh)
{
	m_mesh = std::move(newMesh);
}

