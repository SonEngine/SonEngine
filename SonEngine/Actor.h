#pragma once

#include <directxtk12/SimpleMath.h>

class Actor
{
public:
	Actor();

public:
	void SetSpeed(const float& newSpeed);
	void SetRotateSpeed(const float& newSpeed);
	void SetPosition(const DirectX::SimpleMath::Vector3& newPos);

	float GetSpeed() const { return m_speed; }
	float GetRotateSpeed() const  {	return m_rotateSpeed;	}
	
	DirectX::SimpleMath::Vector3 GetFrontDirection() const { return m_frontDirection; }
	DirectX::SimpleMath::Vector3 GetUpDirection() const { return m_upDirection; }
	DirectX::SimpleMath::Vector3 GetRightDirection()const { return m_rightDirection; }
	DirectX::SimpleMath::Vector3 GetPosition()const { return m_position; }
	DirectX::SimpleMath::Matrix GetViewMatrix()const;

protected:
	DirectX::SimpleMath::Vector3 m_position;

	DirectX::SimpleMath::Vector3 m_baseUpDirection;
	DirectX::SimpleMath::Vector3 m_baseFrontDirection;
	
	DirectX::SimpleMath::Vector3 m_frontDirection;
	DirectX::SimpleMath::Vector3 m_upDirection;
	DirectX::SimpleMath::Vector3 m_rightDirection;

	DirectX::SimpleMath::Matrix m_rotation;
	DirectX::SimpleMath::Matrix m_viewMatrix;

	bool IsViewUpdated = true;

protected:
	float m_speed = 1.f;
	float m_rotateSpeed = 1.f;

};