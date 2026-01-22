#include "Camera.h"


Camera::Camera():
	Actor()
{
	std::shared_ptr<SceneComponent> c = std::make_shared<SceneComponent>(this);
	m_rootComponent = c;
}

void Camera::Initialize()
{
	UpdateProjMatrix();
}

void Camera::Tick(const float& deltaTime)
{
}

void Camera::UpdateCameraLocation(const DirectX::SimpleMath::Vector3& delLoc)
{
	Actor::UpdateActorLocation(delLoc);
}

void Camera::UpdateCameraRotation(const int & mouseDeltaX, const int& mouseDeltaY, const float& deltaTime)
{
	float delX = mouseDeltaX * m_rotateSpeed * deltaTime;
	float delY = mouseDeltaY * m_rotateSpeed * deltaTime;
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


DirectX::SimpleMath::Matrix Camera::GetProjMatrix() const
{
	return projMatrix;
}

void Camera::UpdateProjMatrix()
{
	m_fovRadians = DirectX::XMConvertToRadians(m_fovDegrees);
	if (cameraMode == CM_Perspective)
	{
		projMatrix = DirectX::XMMatrixPerspectiveFovLH(
			m_fovRadians, m_aspectRatio, m_nearZ, m_farZ);
	}
	else
	{
		projMatrix = DirectX::SimpleMath::Matrix::CreateOrthographic((float)m_width, (float)m_height, m_nearZ, m_farZ);
	}
}

void Camera::SetCameraMode(CameraMode newMode)
{
	cameraMode = newMode;
	UpdateProjMatrix();
}