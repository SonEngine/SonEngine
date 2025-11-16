#pragma once

#include "physx\PxPhysicsAPI.h"
#define PVD_HOST "127.0.0.1"
#define MAX_NUM_ACTOR_SHAPES 100

class PhysXEngine : public physx::PxSimulationEventCallback {
public:
	PhysXEngine();
	virtual ~PhysXEngine();
	
public:
	bool Initialize();

	void Tick(float deltaTime);

	void CreateStack(const physx::PxTransform& t, physx::PxU32 size, physx::PxReal halfExtent);

protected:
	void onContact(const  physx::PxContactPairHeader& pairHeader, const  physx::PxContactPair* pairs, physx::PxU32 nbPairs) override;

private:
	physx::PxDefaultAllocator gAllocator;
	physx::PxDefaultErrorCallback gErrorCallback;
	physx::PxFoundation* gFoundation = NULL;
	physx::PxPhysics* gPhysics = NULL;
	physx::PxDefaultCpuDispatcher* gDispatcher = NULL;
	physx::PxScene* gScene = NULL;
	physx::PxMaterial* gMaterial = NULL;
	physx::PxPvd* gPvd = NULL;
	physx::PxReal stackZ = 10.0f;
};