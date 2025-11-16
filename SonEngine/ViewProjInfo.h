#pragma once

#include "directxtk12\SimpleMath.h"

struct ViewProjInfo {
	DirectX::SimpleMath::Vector3 viewDirection;
	DirectX::SimpleMath::Vector3 viewLocation;
	DirectX::SimpleMath::Matrix view;
	DirectX::SimpleMath::Matrix proj;
};