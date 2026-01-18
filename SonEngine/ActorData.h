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
	//DirectX::SimpleMath::Vector3 pos;
	bool useSimulate;
	PhysXMode mode;
	bool updateConstants;

	/*bool forceMip0;
	bool useReflect;
	float heightScale = 0.f;
	float roughness = 0.f;
	float metallic = 0.f;*/
	LocalConstant lc;
};

struct LightData
{
	float viewWidth;
	float viewHeight;
	float nearZ;
	float farZ;

	float intensity;
	Vector4 brightness;
	Vector4 color;
	Vector3 dir;
};