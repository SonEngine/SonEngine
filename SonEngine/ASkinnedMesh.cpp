#include "ASkinnedMesh.h"
#include "SkinnedMeshComponent.h"
#include "CameraComponent.h"
#include "StaticMesh.h"
#include "GeometryGenerator.h"
#include "GraphicsCommon.h"
#include "ModelLoader.h"

ASkinnedMesh::ASkinnedMesh()
{
}

ASkinnedMesh::ASkinnedMesh(std::string actorName, World* world)
	:Actor(actorName, world)
{
}

ASkinnedMesh::~ASkinnedMesh()
{
}

void ASkinnedMesh::Initialize(std::shared_ptr<StaticMesh> mesh, const ActorData& ad, const AnimData& animData)
{
	std::shared_ptr<SkinnedMeshComponent> root = std::make_shared<SkinnedMeshComponent>(this);
	root->SetMesh(mesh);
	root->SetActorData(ad);
	root->SetAnimationData(animData);

	if (ad.name == "player" && Graphics::world)
	{
		std::shared_ptr<CameraComponent> cameraCmp = std::make_shared<CameraComponent>(this);
		cameraCmp->Initialize(70.f, Graphics::world->m_cameraWidth, Graphics::world->m_cameraHeight, 0.1f, 1000.f);
		cameraCmp->SetLocation(Vector3(0, 1.6f, 0.f));
		root->Attach(cameraCmp);
	}
	SetRootComponent(root);
	SetActorData(ad);
	UpdateAnimation(0.f);
}

void ASkinnedMesh::Tick(const float& deltaTime)
{
	UpdateAnimation(deltaTime);

}
