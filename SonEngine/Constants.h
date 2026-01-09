#pragma once

#include "directxtk12/SimpleMath.h"

struct LocalConstant
{
	DirectX::SimpleMath::Matrix model;
	DirectX::SimpleMath::Matrix modelInvTranspose;
	
	int forceMip0 = 0;
	int cubeMipLevel = 0;
	int useReflect = 0;
	float dummy[29];
};

__declspec(align(256)) struct GlobalConstant
{
	DirectX::SimpleMath::Matrix view;
	DirectX::SimpleMath::Matrix proj;
};

//struct LightInfo
//{
//	DirectX::SimpleMath::Vector4 brightness;
//	DirectX::SimpleMath::Vector4 location;
//	DirectX::SimpleMath::Vector4 direction;
//};
//
//__declspec(align(256)) struct PhongGlobalConstant
//{
//	DirectX::SimpleMath::Matrix view;
//	DirectX::SimpleMath::Matrix proj;
//
//	DirectX::SimpleMath::Vector4 viewLoc;
//	DirectX::SimpleMath::Vector4 viewDir;
//
//	LightInfo lights[1];
//};