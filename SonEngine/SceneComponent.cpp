#include "SceneComponent.h"
#include "directxtk12/SimpleMath.h"
#include "StaticMeshComponent.h"

using DirectX::SimpleMath::Vector3;

SceneComponent::SceneComponent(Actor* owner)
	:ActorComponent(owner),
	m_location(Vector3(0, 0, 0)),
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
	m_location = newLocation;
	if (StaticMeshComponent* cmp = dynamic_cast<StaticMeshComponent*>(this))
	{
		cmp->UpdateLocation();
	}
}

void SceneComponent::SetFrontDirection(const DirectX::SimpleMath::Vector3& newDir)
{
	m_frontDirection = newDir;
}

void SceneComponent::SetUpDirection(const DirectX::SimpleMath::Vector3& newDir)
{
	m_upDirection = newDir;
}

void SceneComponent::SetRightDirection(const DirectX::SimpleMath::Vector3& newDir)
{
	m_rightDirection = newDir;
}

void SceneComponent::AddLocation(const DirectX::SimpleMath::Vector3& delLocation)
{
	m_location += delLocation;
	if (StaticMeshComponent* cmp = dynamic_cast<StaticMeshComponent*>(this))
	{
		cmp->UpdateLocation();
	}
}

DirectX::SimpleMath::Matrix SceneComponent::GetViewMatrix() const
{
	return XMMatrixLookToLH(m_location, m_frontDirection, m_upDirection);
}

void SceneComponent::GetChildrenComponents(std::vector<std::shared_ptr<SceneComponent>>& children) const
{
	children = m_children;
}

