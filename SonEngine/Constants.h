#pragma once

#include "directxtk12/SimpleMath.h"

__declspec(align(256)) struct LocalConstant
{
	DirectX::SimpleMath::Matrix model;
};

__declspec(align(256)) struct GlobalConstant
{
	DirectX::SimpleMath::Matrix view;
	DirectX::SimpleMath::Matrix proj;
}; 

__declspec(align(256)) struct PhongGlobalConstant
{
	DirectX::SimpleMath::Matrix view;
	DirectX::SimpleMath::Matrix proj;

	DirectX::SimpleMath::Vector4 cameraPos;
	DirectX::SimpleMath::Vector4 cameraDir;

	DirectX::SimpleMath::Vector4 DirectionLightPos;
	DirectX::SimpleMath::Vector4 DirectionLightDir;
};