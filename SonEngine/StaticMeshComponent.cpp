#include "StaticMeshComponent.h"
#include "Actor.h"
#include "StaticMesh.h"
#include "PointCloudComponent.h"

StaticMeshComponent::StaticMeshComponent(Actor* owner)
	:PrimitiveComponent(owner)
{
	m_mesh = std::shared_ptr<StaticMesh>();
}

StaticMeshComponent::~StaticMeshComponent()
{
}

void StaticMeshComponent::SetMesh(std::shared_ptr<StaticMesh> newMesh)
{
	m_mesh = std::move(newMesh);
}

void StaticMeshComponent::SetActorData(const ActorData& ad)
{
	SetPhysX(ad.useSimulate);
	SetPhysXMode(ad.mode);
	SetHeightScale(ad.heightScale);
	UpdateTexTransform(ad.lc.texTransform);
	SetPSOName(ad.psoName);
	SetUpdateConstant(ad.updateConstants);
	SetTextureName(ad.material);
	SetRoughness(ad.roughness);
	SetMetallic(ad.metallic);
}
