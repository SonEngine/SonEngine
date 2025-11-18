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
		mesh->SetAlbedoTexture(texName);

		std::shared_ptr<StaticMeshComponent> cmp = std::make_shared<StaticMeshComponent>(this);
		cmp->SetMesh(mesh);
		cmp->SetPhysX(false);
		cmp->SetPhysXMode(PhysXMode::PM_Dynamic);

		SetRootComponent(cmp);
		SetActorLocation(loc);
	}
}

void AMovingPlatform::Tick(const float& deltaTime)
{
	if (isTriggered)
	{
		Vector3 del = m_movingDir * deltaTime * m_speed;
		Actor::UpdateActorLocation(del);
	}
}
