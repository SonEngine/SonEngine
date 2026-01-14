#include "ADot.h"
#include "DotComponent.h"
#include "StaticMesh.h"
#include "GeometryGenerator.h"

ADot::ADot()
	:Actor()
{
}

ADot::ADot(std::string actorName, World* world)
	:Actor(actorName, world)
{
}

ADot::~ADot()
{
}

void ADot::Initialize(std::shared_ptr<StaticMesh> mesh, const ActorData& ad)
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
