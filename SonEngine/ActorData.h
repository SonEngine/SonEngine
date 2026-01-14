#pragma once

#include <string>
#include <directxtk12/SimpleMath.h>
#include "PBRHLSLCompat.h"
#include "PhysXMode.h"

struct ActorData 
{
	std::string name;
	std::string mesh;
	std::string material;
	std::string psoName;
	DirectX::SimpleMath::Vector3 pos;
	bool useSimulate;
	PhysXMode mode;
	
	bool forceMip0;
	bool updateConstants;
	bool useReflect;
	float heightScale = 0.f;
	LocalConstant lc;
};

struct LightData
{
	float viewWidth;
	float viewHeight;
	float nearZ;
	float farZ;

	Vector4 brightness;
	Vector3 dir;
};