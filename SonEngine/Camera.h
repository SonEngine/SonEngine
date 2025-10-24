#pragma once

#include "Actor.h"

class Camera : public Actor {
public:
	Camera();

public:
	virtual void Initialize();
	void Tick(const float& deltaTime) override;

public:
	void UpdateCameraLocation(const DirectX::SimpleMath::Vector3& delLoc);
	void UpdateCameraRotation(const int& mouseDeltaX, const int& mouseDeltaY);
	void UpdateProjMatrix();
	public:
	DirectX::SimpleMath::Matrix GetProjMatrix() const;


protected:
	float yAngle = 0.f;
	float xAngle = 0.f;

	float maxYAngle = 89.f;
	float minYAngle = -89.f;

	float m_rotateSpeed = 0.5f;

	// Projection 
public:
	float m_aspectRatio;
	float m_fovDegrees = 60.f;
	float m_fovRadians;
	float m_fovAngle = 70.f;
	float m_nearZ = 0.1f;
	float m_farZ = 100.f;

	DirectX::SimpleMath::Matrix projMatrix;

};