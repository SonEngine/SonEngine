#include "AMovingPlatform.h"
#include "StaticMeshComponent.h"
#include "StaticMesh.h"
#include "GeometryGenerator.h"


AMovingPlatform::AMovingPlatform()
{
}

AMovingPlatform::AMovingPlatform(std::string actorName, World* world)
	:Actor(actorName, world)
{
}

AMovingPlatform::~AMovingPlatform()
{
}

void AMovingPlatform::Initialize(ID3D12Device5* device, ID3D12GraphicsCommandList* commandList, const std::string& texName, DirectX::SimpleMath::Matrix transform)
{
	Vector3 scale;
	DirectX::SimpleMath::Quaternion rot;
	Vector3 loc;

	if (transform.Decompose(scale, rot, loc))
	{
		std::shared_ptr<StaticMesh> mesh = std::make_shared<StaticMesh>();
		mesh->Initialize(device, commandList, GeometryGenerator::MakeCube(scale.x, scale.y, scale.z));
		
		std::shared_ptr<StaticMeshComponent> cmp = std::make_shared<StaticMeshComponent>(this);
		cmp->SetMesh(mesh);
		cmp->SetPhysX(false);
		cmp->SetPhysXMode(PhysXMode::PM_Dynamic);

		SetRootComponent(cmp);
		SetActorLocation(loc);
		SetTextureName(texName);
	}
}

void AMovingPlatform::Initialize(std::shared_ptr<StaticMesh> mesh, const ActorData& ad)
{
	std::shared_ptr<StaticMeshComponent> cmp = std::make_shared<StaticMeshComponent>(this);
	cmp->SetMesh(mesh);
	cmp->SetPhysX(ad.useSimulate);
	cmp->SetPhysXMode(ad.mode);
	SetRootComponent(cmp);
	SetActorLocation(ad.pos);
	SetTextureName(ad.material);
	SetUpdateConstant(ad.updateConstants);
}

void AMovingPlatform::Tick(const float& deltaTime)
{
	if (isTriggered)
	{
		float radian = DirectX::XMConvertToRadians(deltaTime * 90.f);
		auto q = DirectX::SimpleMath::Quaternion::CreateFromAxisAngle(Vector3(0, 1, 0), radian);
		Actor::UpdateActorRotation(q);
	}
}
