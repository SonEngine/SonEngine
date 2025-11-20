#include "SceneComponent.h"
#include "directxtk12/SimpleMath.h"
#include "StaticMeshComponent.h"

using DirectX::SimpleMath::Vector3;

SceneComponent::SceneComponent(Actor* owner)
	:ActorComponent(owner),
	m_frontDirection(Vector3(0, 0, 1)),
	m_baseFrontDirection(Vector3(0, 0, 1)),
	m_upDirection(Vector3(0, 1, 0)),
	m_baseUpDirection(Vector3(0, 1, 0)),
	m_rightDirection(Vector3(1, 0, 0))
{
}

SceneComponent::~SceneComponent()
{
}

void SceneComponent::SetSpeed(const float& newSpeed)
{
	m_speed = newSpeed;
}
void SceneComponent::SetRotateSpeed(const float& newSpeed)
{
	m_rotateSpeed = newSpeed;
}

void SceneComponent::SetLocation(const DirectX::SimpleMath::Vector3& newLocation)
{
	localTransform.location = newLocation;
	if (StaticMeshComponent* cmp = dynamic_cast<StaticMeshComponent*>(this))
	{
		cmp->UpdateLocation();
	}
}

void SceneComponent::SetRotation(const DirectX::SimpleMath::Quaternion& newQuat)
{
	localTransform.quat = newQuat;
	if (StaticMeshComponent* cmp = dynamic_cast<StaticMeshComponent*>(this))
	{
		cmp->UpdateRotation();
	}
}

void SceneComponent::AddLocation(const DirectX::SimpleMath::Vector3& delLocation)
{
	localTransform.location += delLocation;
	if (StaticMeshComponent* cmp = dynamic_cast<StaticMeshComponent*>(this))
	{
		cmp->UpdateLocation();
	}
}
void SceneComponent::AddRotation(const DirectX::SimpleMath::Quaternion& delQ)
{
	localTransform.quat *= delQ;
	if (StaticMeshComponent* cmp = dynamic_cast<StaticMeshComponent*>(this))
	{
		cmp->UpdateRotation();
	}
}

DirectX::SimpleMath::Matrix SceneComponent::GetViewMatrix() const
{
	return XMMatrixLookToLH(GetLocation(), m_frontDirection, m_upDirection);
}

void SceneComponent::GetChildrenComponents(std::vector<std::shared_ptr<SceneComponent>>& children) const
{
	children = m_children;
}

void SceneComponent::OnRegister() 
{
	for (const auto & c : m_children)
	{
		c->OnRegister();
	}
}

