#include "APointCloud.h"
#include "PointCloudComponent.h"
#include "StaticMesh.h"
#include "GeometryGenerator.h"

APointCloud::APointCloud()
	:Actor()
{
}

APointCloud::APointCloud(std::string actorName, World* world)
	:Actor(actorName, world)
{
}

APointCloud::~APointCloud()
{
}

void APointCloud::Initialize(std::shared_ptr<StaticMesh> mesh, const ActorData& ad)
{
	std::shared_ptr<PointCloudComponent> cmp = std::make_shared<PointCloudComponent>(this);
	cmp->SetMesh(mesh);
	cmp->SetActorData(ad);

	SetRootComponent(cmp);
	SetActorData(ad);
}
