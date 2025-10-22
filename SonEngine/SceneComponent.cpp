#include "SceneComponent.h"
#include "directxtk12/SimpleMath.h"

using DirectX::SimpleMath::Vector3;

SceneComponent::SceneComponent(Actor* owner)
	:ActorComponent(owner),
	m_location(Vector3(0, 0, -3)),
	m_frontDirection(Vector3(0, 0, 1)),
	m_baseFrontDirection(Vector3(0, 0, 1)),
	m_upDirection(Vector3(0, 1, 0)),
	m_baseUpDirection(Vector3(0, 1, 0)),
	m_rightDirection(Vector3(1, 0, 0))
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
}

DirectX::SimpleMath::Matrix SceneComponent::GetViewMatrix() const
{
	return XMMatrixLookToLH(m_location, m_frontDirection, m_upDirection);
}

