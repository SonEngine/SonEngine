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

void CubeMapComponent::SetRotation(const DirectX::SimpleMath::Matrix& mat)
{
	if (m_mesh)
	{
		m_mesh->SetRotation(mat);
	}
}

void CubeMapComponent::Translate(const DirectX::SimpleMath::Vector3& del)
{
	if (m_mesh)
	{
		m_mesh->Translate(del.x, del.y, del.z);
	}
}

void CubeMapComponent::UpdateLocation()
{
	if (m_mesh)
	{
		DirectX::SimpleMath::Vector3 loc = GetLocation();
		m_mesh->SetLocation(loc.x, loc.y, loc.z);
	}
}

void CubeMapComponent::UpdateRotation()
{
	if (m_mesh)
	{
		DirectX::SimpleMath::Quaternion q = GetRotation();
		m_mesh->SetRotation(DirectX::XMMatrixRotationQuaternion(q));
	}
}
