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
		ActorData rootData = ad;
		rootData.useSimulate = false;

		root = std::make_shared<StaticMeshComponent>(this);
		root->SetMesh(world->pbrModelLoader->GetMeshes("door0"));
		root->SetActorData(rootData);
		SetRootComponent(root);

		doorRight = std::make_shared<StaticMeshComponent>(this);
		doorRight->SetLocation(Vector3(0.9f, 1.1f, -0.013f));
		doorRight->SetMesh(world->pbrModelLoader->GetMeshes("door1"));
		doorRight->SetActorData(rootData);
		
		doorLeft = std::make_shared<StaticMeshComponent>(this);
		doorLeft->SetMesh(world->pbrModelLoader->GetMeshes("door2")); // door Left

		ActorData leftData = ad;
		leftData.collisionLocation = Vector3(0.5f, 0.f, 0.05f);
		leftData.lc.collisionScale = Vector3(0.4f, 1.0f, 0.05f);
		leftData.lc.collisionShape = 0;
		//doorLeft->SetLocation(Vector3(-0.989f, 1.1f, -0.013f));
		doorLeft->SetLocation(Vector3(-1.5f, 1.5f, -0.f));
		doorLeft->SetActorData(leftData);

		if (Graphics::world && ad.useSimulate)
		{
			std::shared_ptr<CollisionComponent> collisionCmp = std::make_shared<CollisionComponent>(this);
			collisionCmp->SetMesh(Graphics::world->dotModelLoader->GetMeshes("point"));
			
			collisionCmp->SetActorData(leftData);
			doorLeft->Attach(collisionCmp);
		}
		SetActorData(ad);

		root->Attach(doorLeft);
		root->Attach(doorRight);
		
	}
}

void ADoor::Tick(const float& deltaTime)
{
	/*float degree = rotationSpeed * deltaTime;
	float radian =  DirectX::XMConvertToRadians(degree);
	auto mat = DirectX::XMMatrixRotationY(radian);
	auto q = DirectX::SimpleMath::Quaternion::CreateFromRotationMatrix(mat);
	doorLeft->AddRotation(q);*/
}
