#include "PhysXEngine.h"
#include <string>
#include <vector>

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
		PxPairFlag::eNOTIFY_TOUCH_PERSISTS |
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

	PxTransform t = PxTransform(PxVec3(0.f, 2.f, 0.f));
	PxReal halfExtent = 0.3f;
	PxFilterData filterData;
	filterData.word0 = 1;
	PxShape* shape =
		gPhysics->createShape(PxBoxGeometry(halfExtent, halfExtent, halfExtent), *gMaterial);


	shape->setSimulationFilterData(filterData);
	PxRigidDynamic* body = gPhysics->createRigidDynamic(t);
	body->setName("box");
	body->attachShape(*shape);

	PxRigidBodyExt::updateMassAndInertia(*body, 10.0f);

	gScene->addActor(*body);	
	shape->release();

	return true;
}

void PhysXEngine::Tick(float deltaTime)
{
	float simulationTick = ((deltaTime) < (1 / 60.f) ? deltaTime : (1 / 60.f));;
	gScene->simulate(simulationTick);
	gScene->fetchResults(true);

	PxU32 nbActors = gScene->getNbActors(PxActorTypeFlag::eRIGID_DYNAMIC |
		PxActorTypeFlag::eRIGID_STATIC);

	std::vector<PxRigidActor*> actors(nbActors);
	gScene->getActors(PxActorTypeFlag::eRIGID_DYNAMIC |
		PxActorTypeFlag::eRIGID_STATIC,
		reinterpret_cast<PxActor**>(&actors[0]), nbActors);

	PxShape* shapes[MAX_NUM_ACTOR_SHAPES];

	for (PxU32 i = 0; i < nbActors; i++) {

		const PxU32 nbShapes = actors[i]->getNbShapes();
		PX_ASSERT(nbShapes <= MAX_NUM_ACTOR_SHAPES);
		actors[i]->getShapes(shapes, nbShapes);

		for (PxU32 j = 0; j < nbShapes; j++) {
			const PxMat44 shapePose(
				PxShapeExt::getGlobalPose(*shapes[j], *actors[i]));

			if (actors[i]->is<PxRigidDynamic>()) {

				shapePose.front();
			}
		}
	}
}
void PhysXEngine::CreateStack(const PxTransform& t, PxU32 size, PxReal halfExtent)
{
	PxFilterData filterData;
	filterData.word0 = 1;
	PxShape* shape =
		gPhysics->createShape(PxBoxGeometry(halfExtent, halfExtent, halfExtent), *gMaterial);


	shape->setSimulationFilterData(filterData);
	static int index = 0;

	for (PxU32 i = 0; i < size; i++)
	{
		for (PxU32 j = 0; j < size - i; j++)
		{
			PxVec3 di = PxVec3(4.f / 3.f, 2.f, 0.f) * halfExtent * PxReal(i);
			PxVec3 dj = PxVec3(8.f / 3.f, 0.f, 0.f) * halfExtent * PxReal(j);


			PxTransform localTm(di + dj + PxVec3(0, halfExtent, 0.f));
			PxRigidDynamic* body = gPhysics->createRigidDynamic(t.transform(localTm));
			body->setName("box");
			body->attachShape(*shape);

			PxRigidBodyExt::updateMassAndInertia(*body, 10.0f);

			gScene->addActor(*body);
		}
	}
	shape->release();
}

void PhysXEngine::onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs)
{
	PxActor* actor1 = pairHeader.actors[0];
	PxActor* actor2 = pairHeader.actors[1];

	// 충돌된 액터들에 대한 로그 출력
	std::string name1 = actor1->getName();
	std::string name2 = actor2->getName();
}
