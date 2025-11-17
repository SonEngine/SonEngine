#include "StaticMeshComponent.h"
#include "Actor.h"
#include "StaticMesh.h"

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

void StaticMeshComponent::SetRotation(const DirectX::SimpleMath::Matrix& mat)
{
	if (m_mesh)
	{
		m_mesh->SetRotation(mat);
	}
}

void StaticMeshComponent::Translate(const DirectX::SimpleMath::Vector3& del)
{
	if (m_mesh)
	{
		m_mesh->Translate(del.x, del.y, del.z);
	}
}

void StaticMeshComponent::UpdateLocation()
{
	if (m_mesh)
	{
		DirectX::SimpleMath::Vector3 loc = GetLocation();
		m_mesh->SetLocation(loc.x, loc.y, loc.z);
	}
}
