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