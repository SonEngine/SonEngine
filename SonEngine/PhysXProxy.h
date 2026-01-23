#pragma once

#include <string>
#include "physx\PxPhysicsAPI.h"
#include "PrimitiveComponent.h"

struct PhysXProxy 
{
	PrimitiveComponent* primitive;
	physx::PxRigidDynamic* dynamicBody;
	physx::PxRigidStatic* staticBody;
	std::string name;
	bool simulate;
};