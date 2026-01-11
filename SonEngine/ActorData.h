#pragma once

#include <string>
#include <directxtk12/SimpleMath.h>
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
};