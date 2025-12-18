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

void DotComponent::SetRotation(const DirectX::SimpleMath::Matrix& mat)
{
	if (m_mesh)
	{
		m_mesh->SetRotation(mat);
	}
}

void DotComponent::Translate(const DirectX::SimpleMath::Vector3& del)
{
	if (m_mesh)
	{
		m_mesh->Translate(del.x, del.y, del.z);
	}
}

void DotComponent::UpdateLocation()
{
	if (m_mesh)
	{
		DirectX::SimpleMath::Vector3 loc = GetLocation();
		m_mesh->SetLocation(loc.x, loc.y, loc.z);
	}
}

void DotComponent::UpdateRotation()
{
	if (m_mesh)
	{
		DirectX::SimpleMath::Quaternion q = GetRotation();
		m_mesh->SetRotation(DirectX::XMMatrixRotationQuaternion(q));
	}
}
