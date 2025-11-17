#pragma once

#include <string>
#include "physx\PxPhysicsAPI.h"
#include "PrimitiveComponent.h"

struct PhysXProxy 
{
	PrimitiveComponent* primitive;
	physx::PxRigidDynamic* body;
	std::string name;
	bool simulate;
};