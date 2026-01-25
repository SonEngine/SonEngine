#include "ASkinnedMesh.h"
#include "SkinnedMeshComponent.h"
#include "StaticMeshComponent.h"
#include "CollisionComponent.h"
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
	root = std::make_shared<SkinnedMeshComponent>(this);
	root->SetMesh(mesh);
	root->SetActorData(ad);
	root->SetAnimationData(animData);

	if (ad.name == "player" && Graphics::world)
	{
		std::shared_ptr<CameraComponent> cameraCmp = std::make_shared<CameraComponent>(this);
		cameraCmp->Initialize(70.f, (float)Graphics::world->m_cameraWidth, (float)Graphics::world->m_cameraHeight, 0.1f, 1000.f);
		cameraCmp->SetLocation(Vector3(0, 1.6f, 0.f));
		root->Attach(cameraCmp);
	}
	if (Graphics::world)
	{
		std::shared_ptr<CollisionComponent> collisionCmp = std::make_shared<CollisionComponent>(this);
		collisionCmp->SetMesh(Graphics::world->dotModelLoader->GetMeshes("point"));
		collisionCmp->SetActorData(ad);
		root->Attach(collisionCmp);

		//if (ad.name == "player")
		{
			ActorData meshAd = ad;
			meshAd.useSimulate = false;
			meshAd.psoName = "pbrPSO";
			meshAd.material = "knife_WeaponsPlaceholdMAT_albedo";
			
			meshCmp = std::make_shared<StaticMeshComponent>(this);
			meshCmp->SetMesh(Graphics::world->pbrModelLoader->GetMeshes("Knife"));
			meshCmp->SetActorData(meshAd);
			
			
			Vector3 rot = Vector3(-92.2f, -7.5f, 111.6f);
			float roll = DirectX::XMConvertToRadians(rot.x);
			float pitch = DirectX::XMConvertToRadians(rot.y);
			float yaw = DirectX::XMConvertToRadians(rot.z);
	
			Matrix R = Matrix::CreateFromYawPitchRoll(yaw, pitch, roll);
			Vector3 t = Vector3(2.2f,3.9f,-6.3f);
			//Vector3 t = Vector3(0.f, 10.f, 0.f) ;
			
			Matrix T = Matrix::CreateTranslation(t);
			
			
			meshLocalMat = T * R;
			
			root->Attach(meshCmp);
		}
	}

	SetRootComponent(root);
	SetActorData(ad);
	UpdateAnimation(0.f);
}

void ASkinnedMesh::UpdateSocketMatrix(const Vector3 rot, const Vector3& t)
{
	float roll = DirectX::XMConvertToRadians(rot.x);
	float pitch = DirectX::XMConvertToRadians(rot.y);
	float yaw = DirectX::XMConvertToRadians(rot.z);

	Matrix R = Matrix::CreateFromYawPitchRoll(yaw, pitch, roll);

	Matrix T = Matrix::CreateTranslation(t);
	meshLocalMat = T * R;
}


void ASkinnedMesh::Tick(const float& deltaTime)
{
	UpdateAnimation(deltaTime);
	if (Graphics::world && meshCmp)
	{
		
		Matrix m = Graphics::world->skinnedMeshLoader->GetBoneTransform(root->GetAnimationName(), "hand_r", meshLocalMat, 0);
		meshCmp->SetLocalTransform(m);
	}
	
}
