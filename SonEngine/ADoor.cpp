#include "ADoor.h"
#include "StaticMeshComponent.h"
#include "StaticMesh.h"
#include "GeometryGenerator.h"
#include "GraphicsCommon.h"
#include "ModelLoader.h"

ADoor::ADoor()
{
}

ADoor::ADoor(std::string actorName, World* world)
	:Actor(actorName, world)
{
}

ADoor::~ADoor()
{
}

void ADoor::Initialize(const ActorData& ad)
{
	using namespace Graphics;

	if (world)
	{
		std::shared_ptr<StaticMeshComponent> root = std::make_shared<StaticMeshComponent>(this);
		root->SetMesh(world->pbrModelLoader->GetMeshes("door0"));
		root->SetActorData(ad);
		SetRootComponent(root);

		std::shared_ptr<StaticMeshComponent> cmp1 = std::make_shared<StaticMeshComponent>(this);
		cmp1->SetLocation(Vector3(0.9f, 1.1f, 0.f));
		cmp1->SetMesh(world->pbrModelLoader->GetMeshes("door1"));
		cmp1->SetActorData(ad);

		std::shared_ptr<StaticMeshComponent> cmp2 = std::make_shared<StaticMeshComponent>(this);
		cmp2->SetMesh(world->pbrModelLoader->GetMeshes("door2"));
		cmp2->SetLocation(Vector3(-0.9f, 1.1f, 0.f));
		cmp2->SetActorData(ad);

		root->Attach(cmp1);
		root->Attach(cmp2);
		root->SetLocation(ad.pos);
				
		UpdateMipState(ad.forceMip0);
		UpdateUseReflect(ad.useReflect);
	}
}

void ADoor::Tick(const float& deltaTime)
{
}
