#include "DotComponent.h"
#include "Actor.h"
#include "StaticMesh.h"


DotComponent::DotComponent(Actor* owner)
	:PrimitiveComponent(owner)
{
	m_mesh = std::make_shared<StaticMesh>();
}

DotComponent::~DotComponent()
{
}



void DotComponent::SetMesh(std::shared_ptr<StaticMesh> newMesh)
{
	m_mesh = std::move(newMesh);
}

