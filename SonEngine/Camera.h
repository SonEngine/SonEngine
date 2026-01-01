#pragma once

#include "Actor.h"

enum CameraMode {
	CM_Perspective,
	CM_Orthographic
};
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
	void SetCameraMode(CameraMode newMode);

public:
	DirectX::SimpleMath::Matrix GetProjMatrix() const;
	float GetXAngle() const { return xAngle; };
	float GetYAngle() const { return yAngle; };


protected:
	float xAngle = 0.f;
	float yAngle = 0.f;

	float maxYAngle = 89.f;
	float minYAngle = -89.f;

	float m_rotateSpeed = 0.5f;

	// Projection 
public:
	int m_width;
	int m_height;
	float m_aspectRatio;

	float m_fovDegrees = 60.f;
	float m_fovRadians;
	float m_fovAngle = 70.f;
	float m_nearZ = 0.1f;
	float m_farZ = 1000.f;

	DirectX::SimpleMath::Matrix projMatrix;

private:
	CameraMode cameraMode;
};