#include "PhysXEngine.h"
#include <string>
#include <vector>
#include <iostream>

#include "PrimitiveComponent.h"


using namespace physx;


static physx::PxFilterFlags contactReportFilterShader(physx::PxFilterObjectAttributes attributes0, physx::PxFilterData filterData0,
	physx::PxFilterObjectAttributes attributes1, physx::PxFilterData filterData1,
	physx::PxPairFlags& pairFlags, const void* constantBlock, physx::PxU32 constantBlockSize)
{
	using namespace physx;

	PX_UNUSED(attributes0);
	PX_UNUSED(attributes1);
	PX_UNUSED(filterData0);
	PX_UNUSED(filterData1);
	PX_UNUSED(constantBlockSize);
	PX_UNUSED(constantBlock);

	// all initial and persisting reports for everything, with per-point data
	//if(filterData0.word0)
	pairFlags =
		PxPairFlag::eSOLVE_CONTACT |
		PxPairFlag::eDETECT_DISCRETE_CONTACT |
		PxPairFlag::eNOTIFY_TOUCH_FOUND |
		PxPairFlag::eNOTIFY_TOUCH_LOST |
		PxPairFlag::eNOTIFY_CONTACT_POINTS;

	return PxFilterFlag::eDEFAULT;
}

PhysXEngine::PhysXEngine()
{
}

PhysXEngine::~PhysXEngine()
{
}

bool PhysXEngine::Initialize()
{
	gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);
	gPvd = PxCreatePvd(*gFoundation);
	PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
	gPvd->connect(*transport, PxPvdInstrumentationFlag::eALL);
	gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale(), true, gPvd);

	PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
	gDispatcher = PxDefaultCpuDispatcherCreate(3);
	sceneDesc.cpuDispatcher = gDispatcher;
	sceneDesc.filterShader = contactReportFilterShader;
	sceneDesc.simulationEventCallback = this;
	gScene = gPhysics->createScene(sceneDesc);

	PxPvdSceneClient* pvdClient = gScene->getScenePvdClient();
	if (pvdClient) {
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
	}
	gMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.6f);

	PxRigidStatic* groundPlane =
		PxCreatePlane(*gPhysics, PxPlane(0, 1, 0, 0), *gMaterial);
	groundPlane->setName("Ground");

	gScene->addActor(*groundPlane);

	return true;
}

void PhysXEngine::Tick(float deltaTime)
{
	float simulationTick = ((deltaTime) < (1 / 60.f) ? deltaTime : (1 / 60.f));;
	gScene->simulate(simulationTick);
	gScene->fetchResults(true);

	for (auto& proxy : proxyArr)
	{
		if (proxy.simulate)
		{
			proxy.primitive->SyncFromPhysX(proxy.body->getGlobalPose());
		}
	}
}

void PhysXEngine::AddRigidDynamic(physx::PxRigidDynamic* rigidBody)
{
	//gScene->addActor(*rigidBody);
}

void PhysXEngine::RegisterPrimitive(class PrimitiveComponent* primitive, bool usePhysx)
{
	if (primitive == nullptr)
		return;

	std::string name = primitive->GetName();

	DirectX::SimpleMath::Vector3 loc = primitive->GetLocation();
	DirectX::SimpleMath::Quaternion rot = primitive->GetRotation();

	PxTransform t = PxTransform(PxVec3(loc.x, loc.y, loc.z),PxQuat(rot.x,rot.y,rot.z,rot.w));
	PxReal halfExtent = 0.5f;
	PxFilterData filterData;
	filterData.word0 = 1;

	PxShape* shape = gPhysics->createShape(PxBoxGeometry(halfExtent, halfExtent, halfExtent), *gMaterial);
	shape->userData = primitive;
	shape->setSimulationFilterData(filterData);

	PxRigidDynamic* body = gPhysics->createRigidDynamic(t);
	PhysXMode mode = primitive->GetPhysXMode();

	if (mode == PM_Dynamic)
	{
		// dynamic 전용
		body->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, false);
	}
	else if (mode == PM_Kinematic)
	{
		body->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
		body->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
	}
	else if (mode == PM_Trigger)
	{
		shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);
		shape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, true);
		body->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
	}
	body->setName(name.c_str());
	body->attachShape(*shape);

	PxRigidBodyExt::updateMassAndInertia(*body, 10.0f);

	if (usePhysx && body)
	{
		gScene->addActor(*body);
	}

	shape->release();

	PhysXProxy proxy;
	proxy.body = body;
	proxy.primitive = primitive;
	proxy.name = name;
	proxy.simulate = usePhysx;

	proxyArr.push_back(proxy);
}

void PhysXEngine::SyncKinematics()
{
	for (auto& proxy : proxyArr)
	{
		if (proxy.primitive->IsKinematic())
		{
			if (proxy.body)
			{
				PxTransform t = proxy.primitive->GetPxTransform();
				//std::cout << "SyncKinematic - " << t.p.x << t.p.y << t.p.z << '\n';
				proxy.body->setKinematicTarget(t);
			}
		}
	}
}

void PhysXEngine::onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs)
{
	for (PxU32 i = 0; i < nbPairs; i++)
	{
		PxShape* shape0 = pairs[i].shapes[0];
		PxShape* shape1 = pairs[i].shapes[1];

		PrimitiveComponent* prim0 = static_cast<PrimitiveComponent*>(shape0->userData);
		PrimitiveComponent* prim1 = static_cast<PrimitiveComponent*>(shape1->userData);
		if (prim0)
		{
			std::cout << "prim0 : " << prim0->GetName() << std::endl;
		}
		if (prim1)
		{
			std::cout << "prim1 : " << prim1->GetName() << std::endl;
		}
	}

}

void PhysXEngine::onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count)
{
	for (PxU32 i = 0; i < count; i++)
	{
		PxTriggerPair p = pairs[i];
		if (p.status & PxPairFlag::eNOTIFY_TOUCH_FOUND)
		{
			std::cout << "Touch Found ! \n";
			PxShape* t = p.triggerShape;
			PxShape* other = p.otherShape;

			PrimitiveComponent* triggerPrimitive = static_cast<PrimitiveComponent*>(t->userData);
			PrimitiveComponent* otherPrimitive = static_cast<PrimitiveComponent*>(other->userData);
			if (triggerPrimitive)
			{
				//std::cout << "triggerPrimitive : " << triggerPrimitive->GetName() << std::endl;
				triggerPrimitive->OnComponentBeginOverlap.Execute(otherPrimitive);
			}
		}
		else if (p.status & PxPairFlag::eNOTIFY_TOUCH_LOST)
		{
			std::cout << "Touch Lost ! \n";
			PxShape* t = p.triggerShape;
			PxShape* other = p.otherShape;

			PrimitiveComponent* triggerPrimitive = static_cast<PrimitiveComponent*>(t->userData);
			PrimitiveComponent* otherPrimitive = static_cast<PrimitiveComponent*>(other->userData);

			if (triggerPrimitive)
			{
				//std::cout << "triggerPrimitive : " << triggerPrimitive->GetName() << std::endl;
				triggerPrimitive->OnComponentEndOverlap.Execute(otherPrimitive);

			}
		}
	}
}