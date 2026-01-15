#pragma once

#include "Actor.h"
#include "ActorData.h"
//#include "StaticMesh.h"

class StaticMesh;

class ADoor : public Actor {
public:
	ADoor();
	ADoor(std::string actorName, World* world);
	virtual ~ADoor();

public:
	void Initialize(const ActorData& ad);

public:
	void Tick(const float& deltaTime) override;

public:
	void SetSpeed(float speed) { m_speed = speed; }
	void SetDirection(DirectX::SimpleMath::Vector3 dir) { m_movingDir = dir; }
	void SetTriggered(bool triggered) { isTriggered = triggered; }

private:
	float m_speed = 1.f;
	DirectX::SimpleMath::Vector3 m_movingDir = DirectX::SimpleMath::Vector3(1, 0, 0);
	bool isTriggered = false;
};