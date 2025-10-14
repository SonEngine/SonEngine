#pragma once

#include "Actor.h"

class Camera : public Actor {
public:
	Camera();

public:
	void UpdateCameraPosition(const DirectX::SimpleMath::Vector3& delPos);
	void UpdateCameraRotation(const int& mouseDeltaX, const int& mouseDeltaY);

protected:
	float yAngle = 0.f;
	float xAngle = 0.f;

	float maxYAngle = 89.f;
	float minYAngle = -89.f;


};