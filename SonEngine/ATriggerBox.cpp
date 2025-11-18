#include "ATriggerBox.h"
#include "StaticMeshComponent.h"
#include "StaticMesh.h"
#include "GeometryGenerator.h"
#include "AMovingPlatform.h"

ATriggerBox::ATriggerBox()
	:Actor()
{
}

ATriggerBox::ATriggerBox(std::string actorName, World* world)
	:Actor(actorName, world)
{
}

ATriggerBox::~ATriggerBox()
{
}

void ATriggerBox::Initialize(ID3D12Device5* device, ID3D12GraphicsCommandList* commandList, const std::string & texName,
	DirectX::SimpleMath::Matrix transform)
{
	Vector3 scale;
	DirectX::SimpleMath::Quaternion rot;
	Vector3 loc;

	if (transform.Decompose(scale, rot, loc))
	{
		std::shared_ptr<StaticMesh> mesh = std::make_shared<StaticMesh>();
		mesh->Initialize(device, commandList, GeometryGenerator::MakeCube(scale.x,scale.y, scale.z));
		mesh->SetAlbedoTexture(texName);
		
		std::shared_ptr<StaticMeshComponent> cmp = std::make_shared<StaticMeshComponent>(this);
		cmp->SetMesh(mesh);
		cmp->SetPhysX(true);
		cmp->SetPhysXMode(PhysXMode::PM_Trigger);
		cmp->OnComponentBeginOverlap.Bind(this, &ATriggerBox::OnBeginTrigger);
		cmp->OnComponentEndOverlap.Bind(this, &ATriggerBox::OnEndTrigger);

		SetRootComponent(cmp);
		SetActorLocation(loc);
	}
}


void ATriggerBox::OnBeginTrigger(PrimitiveComponent* primitive)
{
	//std::cout << "TriggerBox OnBeginTrigger\n";
	if (m_target)
	{
		if (AMovingPlatform* platform = dynamic_cast<AMovingPlatform*>(m_target))
		{
			platform->SetTriggered(true);
		}
	}
}

void ATriggerBox::OnEndTrigger(PrimitiveComponent* primitive)
{
	//std::cout << "TriggerBox OnBeginTrigger\n";
	if (m_target)
	{
		if (AMovingPlatform* platform = dynamic_cast<AMovingPlatform*>(m_target))
		{
			platform->SetTriggered(false);
		}
	}
}
