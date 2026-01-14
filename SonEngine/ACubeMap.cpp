#include "ACubeMap.h"
#include "CubeMapComponent.h"
#include "StaticMesh.h"
#include "GeometryGenerator.h"

ACubeMap::ACubeMap()
	:Actor()
{
}

ACubeMap::ACubeMap(std::string actorName, World* world)
	:Actor(actorName, world)
{
}

ACubeMap::~ACubeMap()
{
}

void ACubeMap::Initialize(std::shared_ptr<StaticMesh> mesh, const ActorData& ad)
{
	std::shared_ptr<CubeMapComponent> cmp = std::make_shared<CubeMapComponent>(this);
	cmp->SetMesh(mesh);
	cmp->SetPhysX(ad.useSimulate);
	cmp->SetPhysXMode(ad.mode);
	cmp->SetHeightScale(ad.heightScale);
	cmp->UpdateTexTransform(ad.lc.texTransform);
	cmp->SetPSOName(ad.psoName);
	cmp->SetUpdateConstant(ad.updateConstants);
	cmp->SetTextureName(ad.material);

	SetRootComponent(cmp);
	SetActorData(ad);
}
