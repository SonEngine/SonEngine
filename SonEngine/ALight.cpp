#include "ALight.h"
#include "LightComponent.h"
#include "StaticMesh.h"
#include "GeometryGenerator.h"

ALight::ALight()
	:Actor()
{
}

ALight::ALight(std::string actorName, World* world)
	:Actor(actorName, world)
{
}

ALight::~ALight()
{
}

void ALight::Initialize(std::shared_ptr<StaticMesh> mesh, const ActorData& ad, const LightData& ld)
{
	std::shared_ptr<LightComponent> cmp = std::make_shared<LightComponent>(this);
	cmp->SetMesh(mesh);
	cmp->SetPhysX(ad.useSimulate);
	cmp->SetPhysXMode(ad.mode);
	cmp->SetHeightScale(ad.heightScale);
	cmp->UpdateTexTransform(ad.lc.texTransform);
	cmp->SetPSOName(ad.psoName);
	cmp->SetUpdateConstant(ad.updateConstants);
	cmp->SetTextureName(ad.material);

	cmp->SetTextureName(ad.material);
	cmp->InitLightInfo(ld);

	SetRootComponent(cmp);
	SetActorData(ad);
}

void ALight::Tick(const float& deltaTime)
{
	float degree = deltaTime * 360.f * m_rotSpeed;
	float radian = DirectX::XMConvertToRadians(degree);
	DirectX::SimpleMath::Matrix mat = DirectX::XMMatrixRotationY(radian);
	Vector3 loc = GetActorLocation();
	auto newLoc = Vector3::Transform(loc, mat);
	SetActorLocation(newLoc);
	
}