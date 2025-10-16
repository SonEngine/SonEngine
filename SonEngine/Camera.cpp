#include "Camera.h"


Camera::Camera():
	Actor()
{
}

void Camera::UpdateCameraPosition(const DirectX::SimpleMath::Vector3& delPos)
{
	m_position += delPos;
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

	m_rotation = DirectX::XMMatrixRotationY(xRadian);

	m_frontDirection = DirectX::SimpleMath::Vector3::Transform(m_baseFrontDirection, m_rotation);
	m_rightDirection = m_baseUpDirection.Cross(m_frontDirection);;

	if (yAngle + delY >= maxYAngle)
		delY = maxYAngle - yAngle;
	else if (yAngle + delY <= minYAngle)
		delY = minYAngle - yAngle;

	yAngle += delY;
	//std::cout << yAngle << '\n';
	float yRadian = DirectX::XMConvertToRadians(yAngle);


	m_rotation = DirectX::XMMatrixRotationAxis(m_rightDirection, yRadian);
	m_frontDirection = DirectX::SimpleMath::Vector3::Transform(m_frontDirection, m_rotation);
	//m_upDirection = m_frontDirection.Cross(m_rightDirection);
}
