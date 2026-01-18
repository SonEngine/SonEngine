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
	cmp->SetActorData(ad);

	SetRootComponent(cmp);
	SetActorData(ad);
}
