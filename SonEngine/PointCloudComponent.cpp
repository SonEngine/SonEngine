#include "PointCloudComponent.h"
#include "Actor.h"
#include "StaticMesh.h"


PointCloudComponent::PointCloudComponent(Actor* owner)
	:PrimitiveComponent(owner)
{
	m_mesh = std::shared_ptr<StaticMesh>();
}

PointCloudComponent::~PointCloudComponent()
{
}

void PointCloudComponent::SetMesh(std::shared_ptr<StaticMesh> newMesh)
{
	m_mesh = std::move(newMesh);
}


