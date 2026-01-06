#include "CubeMapComponent.h"
#include "Actor.h"
#include "StaticMesh.h"

CubeMapComponent::CubeMapComponent(Actor* owner)
	:PrimitiveComponent(owner)
{
	m_mesh = std::shared_ptr<StaticMesh>();
}

CubeMapComponent::~CubeMapComponent()
{
}

void CubeMapComponent::SetMesh(std::shared_ptr<StaticMesh> newMesh)
{
	m_mesh = std::move(newMesh);
}

