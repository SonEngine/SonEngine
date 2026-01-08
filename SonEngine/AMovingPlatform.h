#pragma once

#include "Actor.h"
#include "ActorData.h"
//#include "StaticMesh.h"

class StaticMesh;

class AMovingPlatform : public Actor {
public:
	AMovingPlatform();
	AMovingPlatform(std::string actorName, World* world);
	virtual ~AMovingPlatform();

public:
	void Initialize(ID3D12Device5* device, ID3D12GraphicsCommandList* commandList, const std::string& texName, DirectX::SimpleMath::Matrix transform);
	void Initialize(std::shared_ptr<StaticMesh> mesh, const ActorData& ad) override;

public:
	void Tick(const float& deltaTime) override;

public:
	void SetSpeed(float speed) { m_speed = speed; }
	void SetDirection(DirectX::SimpleMath::Vector3 dir) { m_movingDir = dir; }
	void SetTriggered(bool triggered) { isTriggered = triggered; }

private:
	float m_speed = 1.f;
	DirectX::SimpleMath::Vector3 m_movingDir = DirectX::SimpleMath::Vector3(1,0,0);
	bool isTriggered = false;
};