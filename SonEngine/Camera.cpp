#include "Camera.h"


Camera::Camera():
	Actor()
{
	std::shared_ptr<SceneComponent> c = std::make_shared<SceneComponent>(this);
	m_rootComponent = c;
}

void Camera::UpdateCameraPosition(const DirectX::SimpleMath::Vector3& delPos)
{
	Actor::UpdateActorLocation(delPos);
}

void Camera::UpdateCameraRotation(const int & mouseDeltaX, const int& mouseDeltaY)
{
	float delX = mouseDeltaX * m_rotateSpeed;
	float delY = mouseDeltaY * m_rotateSpeed;
	xAngle += delX;
	if (xAngle >= 360) {
		xAngle -= 360;
	}
	if (xAngle <= -360) {
		xAngle += 360;
	}

	float xRadian = DirectX::XMConvertToRadians(xAngle);

	DirectX::SimpleMath::Matrix m_rotation = DirectX::XMMatrixRotationY(xRadian);

	DirectX::SimpleMath::Vector3 m_frontDir = DirectX::SimpleMath::Vector3::Transform(m_rootComponent->GetBaseFrontDirection(), m_rotation);
	DirectX::SimpleMath::Vector3 m_rightDir = m_rootComponent->GetBaseUpDirection().Cross(m_frontDir);;

	if (yAngle + delY >= maxYAngle)
		delY = maxYAngle - yAngle;
	else if (yAngle + delY <= minYAngle)
		delY = minYAngle - yAngle;

	yAngle += delY;
	//std::cout << yAngle << '\n';
	float yRadian = DirectX::XMConvertToRadians(yAngle);


	m_rotation = DirectX::XMMatrixRotationAxis(m_rightDir, yRadian);
	m_frontDir = DirectX::SimpleMath::Vector3::Transform(m_frontDir, m_rotation);
	
	m_rootComponent->SetFrontDirection(m_frontDir);
	m_rootComponent->SetRightDirection(m_rightDir);
	//m_upDirection = m_frontDirection.Cross(m_rightDirection);
}

void Camera::Tick(const float& deltaTime)
{
}
