#pragma once

#include "directxtk12\SimpleMath.h"

// SceneComponent의 위치, 회전, 스케일을 저장
// TODO : rotation quaternion 으로 변경
struct Transform {
	DirectX::SimpleMath::Vector3 location;
	DirectX::SimpleMath::Matrix rotation;
	DirectX::SimpleMath::Vector3 scale = DirectX::SimpleMath::Vector3(1.f,1.f,1.f);

	DirectX::SimpleMath::Matrix ToMatrix() const
	{
		DirectX::SimpleMath::Matrix mat;
		mat = rotation;
		mat.m[3][0] = location.x;
		mat.m[3][1] = location.y;
		mat.m[3][2] = location.z;

		return DirectX::SimpleMath::Matrix::CreateScale(scale) * mat;
	}
};