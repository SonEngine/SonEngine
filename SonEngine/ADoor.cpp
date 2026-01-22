#include "ADoor.h"
#include "StaticMesh.h"
#include "GeometryGenerator.h"
#include "GraphicsCommon.h"
#include "ModelLoader.h"

ADoor::ADoor()
{
}

ADoor::ADoor(std::string actorName, World* world)
	:Actor(actorName, world)
{
}

ADoor::~ADoor()
{
}

void ADoor::Initialize(const ActorData& ad)
{
	using namespace Graphics;

	if (world)
	{
		 root = std::make_shared<StaticMeshComponent>(this);
		root->SetMesh(world->pbrModelLoader->GetMeshes("door0"));
		root->SetActorData(ad);
		SetRootComponent(root);

		doorRight = std::make_shared<StaticMeshComponent>(this);
		doorRight->SetLocation(Vector3(0.9f, 1.1f, -0.013f));
		doorRight->SetMesh(world->pbrModelLoader->GetMeshes("door1"));
		doorRight->SetActorData(ad);

		doorLeft = std::make_shared<StaticMeshComponent>(this);
		doorLeft->SetMesh(world->pbrModelLoader->GetMeshes("door2")); // door Left
		doorLeft->SetLocation(Vector3(-0.989f, 1.1f, -0.013f));
		auto mat = DirectX::XMMatrixRotationY(DirectX::XM_PIDIV4);
		auto q = DirectX::SimpleMath::Quaternion::CreateFromRotationMatrix(mat);
		doorLeft->SetRotation(q);
		doorLeft->SetActorData(ad);

		root->Attach(doorLeft);
		root->Attach(doorRight);
		
		SetActorData(ad);
	}
}

void ADoor::Tick(const float& deltaTime)
{
	float degree = rotationSpeed * deltaTime;
	float radian =  DirectX::XMConvertToRadians(degree);
	auto mat = DirectX::XMMatrixRotationY(radian);
	auto q = DirectX::SimpleMath::Quaternion::CreateFromRotationMatrix(mat);
	doorLeft->AddRotation(q);
}
