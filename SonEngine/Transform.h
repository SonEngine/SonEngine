#pragma once

#include "directxtk12\SimpleMath.h"

struct Transform {
	DirectX::SimpleMath::Vector3 location;
	DirectX::SimpleMath::Quaternion quat;
	DirectX::SimpleMath::Vector3 scale = DirectX::SimpleMath::Vector3(1.f,1.f,1.f);

	DirectX::SimpleMath::Matrix ToMatrix() const
	{
		DirectX::SimpleMath::Matrix mat;
		mat = DirectX::XMMatrixRotationQuaternion(quat);
		mat.m[3][0] = location.x;
		mat.m[3][1] = location.y;
		mat.m[3][2] = location.z;

		return DirectX::SimpleMath::Matrix::CreateScale(scale) * mat;
	}
};