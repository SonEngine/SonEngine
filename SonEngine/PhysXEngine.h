#pragma once

#include "physx\PxPhysicsAPI.h"
#include "PhysXProxy.h"

#define PVD_HOST "127.0.0.1"
#define MAX_NUM_ACTOR_SHAPES 100


class PhysXEngine : public physx::PxSimulationEventCallback {
public:
	PhysXEngine();
	virtual ~PhysXEngine();
	
public:
	bool Initialize();

	// 물리 시뮬레이션 후 primitive에 전달
	void Tick(float deltaTime);

public:
	void AddRigidDynamic(physx::PxRigidDynamic* rigidBody);
	void RegisterPrimitive(PrimitiveComponent* primitive, bool usePhysx = false);
	void SyncKinematics();
	//void RegisterPrimitive(class PrimitiveComponent* primitive);

public:
	physx::PxPhysics* GetPhysics() { return gPhysics; }
	physx::PxMaterial* GetMatarial() { return gMaterial; }

protected:
	void onContact(const physx::PxContactPairHeader& pairHeader, const  physx::PxContactPair* pairs, physx::PxU32 nbPairs) override;
	void onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count) override { PX_UNUSED(constraints); PX_UNUSED(count); }
	void onWake(physx::PxActor** actors, physx::PxU32 count) override { PX_UNUSED(actors); PX_UNUSED(count); }
	void onSleep(physx::PxActor** actors, physx::PxU32 count) override { PX_UNUSED(actors); PX_UNUSED(count); }
	void onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count) override;
	void SetKinematicMode(PrimitiveComponent* prim, physx::PxRigidDynamic* dyn);
	void SetDynamicMode(PrimitiveComponent* prim, physx::PxRigidDynamic* dyn);
	void CreatePlayerController(PrimitiveComponent* primitive, physx::PxScene* scene, physx::PxPhysics* physics, physx::PxMaterial* material);
	//void CreatePlayerController(physx::PxScene* scene, physx::PxPhysics* physics, physx::PxMaterial* material);
	void onAdvance(const physx::PxRigidBody* const*, const physx::PxTransform*, const physx::PxU32) override {}
	
public:
	void TickPlayerController(float dt, const physx::PxVec3& inputDir, float moveSpeed);


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

private:
	physx::PxControllerManager* gControllerMgr;
	physx::PxCapsuleController* gPlayerController;

private:
	std::vector< PhysXProxy> proxyArr;

private:
	class PrimitiveComponent* grabbedPrimitive = nullptr;
	class physx::PxRigidDynamic* grabbedDynamic = nullptr;
	physx::PxF32 grabbedDistance = 0;

	class PrimitiveComponent* playerPrimitive = nullptr;

};