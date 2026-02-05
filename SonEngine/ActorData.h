#pragma once

#include <string>
#include <directxtk12/SimpleMath.h>
#include "PBRHLSLCompat.h"
#include "PhysXMode.h"

enum ActorState {
	AS_default,
	AS_idle,
	AS_attack,
	AS_playMontage
};

struct ActorData
{
	ActorData()
	{
		useSimulate = false;
		mode = PhysXMode::PM_Default;
		updateConstants = false;
		lc.useReflect = false;
		lc.heightScale = 0.f;
		lc.forceMip0 = false;
		lc.roughness = 0.2f;
		lc.metallic = 0.8f;
		lc.collisionScale = Vector3(0.5, 0.5, 0.5);
		lc.collisionShape = PhysXShape::PS_cube;
		collisionLocation = Vector3::Zero;
	};

	std::string name;
	std::string mesh;
	std::string material;
	std::string psoName;
	//DirectX::SimpleMath::Vector3 pos;
	bool useSimulate;
	PhysXMode mode = PhysXMode::PM_Default;
	bool updateConstants;
	Vector3 collisionLocation;
	LocalConstant lc;
};

struct LightData
{
	float viewWidth;
	float viewHeight;
	float nearZ;
	float farZ;

	float intensity;
	Vector4 brightness;
	Vector4 color;
	Vector3 dir;
};


struct AnimData
{
	std::string name;
	float animationSpeed = 60.f;
	bool playAnimation = true;
	ActorState actorState = ActorState::AS_default;
};

inline ActorData CreateBulletActorData(const Vector3& location)
{
	static int i = 0;
	ActorData ad;
	
	ad.useSimulate = true;
	ad.mode = PhysXMode::PM_Bullet;
	ad.updateConstants = true;
	ad.lc.useReflect = false;
	ad.lc.heightScale = 0.f;
	ad.lc.forceMip0 = false;
	ad.lc.roughness = 0.2f;
	ad.lc.metallic = 0.8f;
	ad.lc.collisionScale = Vector3(0.5, 0.5, 0.5);
	ad.lc.collisionShape = PhysXShape::PS_cube;
	ad.collisionLocation = Vector3::Zero;

	ad.name = "bullet_" + std::to_string(i);
	ad.mesh = "sphere";
	ad.material ="Metal052C_4K-PNG_albedo";
	ad.psoName = "pbrPSO";

	DirectX::SimpleMath::Matrix model =
		DirectX::SimpleMath::Matrix::CreateTranslation(location);

	ad.lc.model = model.Transpose();
	ad.lc.modelInvTranspose = model.Invert();
	i++;
	return ad;
}