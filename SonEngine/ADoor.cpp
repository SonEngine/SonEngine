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

		ActorData rightData = ad;
		rightData.collisionLocation = Vector3(-0.5f, 0.2f, 0.025f);
		rightData.lc.collisionScale = Vector3(0.45f, 1.2f, 0.05f);
		rightData.lc.collisionShape = 0;

		doorRight = std::make_shared<StaticMeshComponent>(this);
		doorRight->SetMesh(world->pbrModelLoader->GetMeshes("door1"));
		doorRight->SetLocation(Vector3(0.9f, 1.1f, -0.013f));
		doorRight->SetActorData(rightData);
		
		ActorData leftData = ad;
		leftData.collisionLocation = Vector3(0.5f, 0.2f, 0.025f);
		leftData.lc.collisionScale = Vector3(0.45f, 1.2f, 0.05f);
		leftData.lc.collisionShape = 0;

		doorLeft = std::make_shared<StaticMeshComponent>(this);
		doorLeft->SetMesh(world->pbrModelLoader->GetMeshes("door2")); // door Left
		doorLeft->SetLocation(Vector3(-0.989f, 1.1f, -0.013f));
		doorLeft->SetActorData(leftData);

		if (Graphics::world && ad.useSimulate)
		{
			std::shared_ptr<CollisionComponent> collisionLeft = std::make_shared<CollisionComponent>(this);
			collisionLeft->SetMesh(Graphics::world->dotModelLoader->GetMeshes("point"));
			
			collisionLeft->SetActorData(leftData);
			doorLeft->Attach(collisionLeft);

			std::shared_ptr<CollisionComponent> collisionRight = std::make_shared<CollisionComponent>(this);
			collisionRight->SetMesh(Graphics::world->dotModelLoader->GetMeshes("point"));

			collisionRight->SetActorData(rightData);
			doorRight->Attach(collisionRight);
		}

		root->Attach(doorLeft);
		root->Attach(doorRight);
		SetActorData(ad);

	}
}

void ADoor::Tick(const float& deltaTime)
{
	if (openDoor)
	{
		float degree = rotationSpeed * deltaTime;
		leftDegree += degree;
		if (leftDegree >= 120.f)
		{
			leftDegree = 120.f;
			openDoor = false;
			currentState = Ds_opened;
		}

		float leftRadian = DirectX::XMConvertToRadians(leftDegree);
		auto leftQ = DirectX::SimpleMath::Quaternion::CreateFromRotationMatrix(DirectX::XMMatrixRotationY(leftRadian));
		float rightRadian = DirectX::XMConvertToRadians(-leftDegree);
		auto rightQ = DirectX::SimpleMath::Quaternion::CreateFromRotationMatrix(DirectX::XMMatrixRotationY(rightRadian));

		doorLeft->SetRotation(leftQ);
		doorRight->SetRotation(rightQ);
	}
	else if (closeDoor)
	{
		float degree = -rotationSpeed * deltaTime;
		leftDegree += degree;
		if (leftDegree <= 0.f)
		{
			leftDegree = 0.f;
			closeDoor = false;
			currentState = DS_closed;
		}

		float leftRadian = DirectX::XMConvertToRadians(leftDegree);
		auto leftQ = DirectX::SimpleMath::Quaternion::CreateFromRotationMatrix(DirectX::XMMatrixRotationY(leftRadian));
		float rightRadian = DirectX::XMConvertToRadians(-leftDegree);
		auto rightQ = DirectX::SimpleMath::Quaternion::CreateFromRotationMatrix(DirectX::XMMatrixRotationY(rightRadian));

		doorLeft->SetRotation(leftQ);
		doorRight->SetRotation(rightQ);
	}
}

void ADoor::Interact()
{
	if (currentState == DS_closed || currentState == DS_closing) {
		currentState = DS_opening;
		openDoor = true;
	}
	else
	{
		currentState = DS_closing;
		closeDoor = true;
	}
}
