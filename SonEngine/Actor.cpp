#include "Actor.h"

using DirectX::SimpleMath::Vector3;

Actor::Actor() :
	m_position(Vector3(0, 0, -3)),
	m_frontDirection(Vector3(0, 0, 1)),
	m_baseFrontDirection(Vector3(0, 0, 1)),
	m_upDirection(Vector3(0, 1, 0)),
	m_baseUpDirection(Vector3(0, 1, 0)),
	m_rightDirection(Vector3(1, 0, 0))
{
}

void Actor::SetSpeed(const float& newSpeed)
{
	m_speed = newSpeed;
}
void Actor::SetRotateSpeed(const float& newSpeed)
{
	m_rotateSpeed = newSpeed;
}

void Actor::SetPosition(const DirectX::SimpleMath::Vector3& newPos)
{
	m_position = newPos;
}

DirectX::SimpleMath::Matrix Actor::GetViewMatrix() const
{
	return XMMatrixLookToLH(m_position, m_frontDirection, m_upDirection);
}
