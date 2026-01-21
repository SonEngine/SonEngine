#include "ASkinnedMesh.h"
#include "SkinnedMeshComponent.h"
#include "StaticMesh.h"
#include "GeometryGenerator.h"
#include "GraphicsCommon.h"
#include "ModelLoader.h"

ASkinnedMesh::ASkinnedMesh()
{
}

ASkinnedMesh::ASkinnedMesh(std::string actorName, World* world)
	:Actor(actorName, world)
{
}

ASkinnedMesh::~ASkinnedMesh()
{
}

void ASkinnedMesh::Initialize(std::shared_ptr<StaticMesh> mesh, const ActorData& ad)
{
	std::shared_ptr<SkinnedMeshComponent> cmp = std::make_shared<SkinnedMeshComponent>(this);
	cmp->SetMesh(mesh);
	cmp->SetActorData(ad);


	SetRootComponent(cmp);
	SetActorData(ad);
	UpdateAnimation(0.f);
}


void ASkinnedMesh::Tick(const float& deltaTime)
{
	UpdateAnimation(deltaTime);

}
