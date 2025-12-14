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

void PointCloudComponent::SetRotation(const DirectX::SimpleMath::Matrix& mat)
{
	if (m_mesh)
	{
		m_mesh->SetRotation(mat);
	}
}

void PointCloudComponent::Translate(const DirectX::SimpleMath::Vector3& del)
{
	if (m_mesh)
	{
		m_mesh->Translate(del.x, del.y, del.z);
	}
}

void PointCloudComponent::UpdateLocation()
{
	if (m_mesh)
	{
		DirectX::SimpleMath::Vector3 loc = GetLocation();
		m_mesh->SetLocation(loc.x, loc.y, loc.z);
	}
}

void PointCloudComponent::UpdateRotation()
{
	if (m_mesh)
	{
		DirectX::SimpleMath::Quaternion q = GetRotation();
		m_mesh->SetRotation(DirectX::XMMatrixRotationQuaternion(q));
	}
}
