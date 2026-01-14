#pragma once

#include "Actor.h"

class ALight : public Actor {
public:
	ALight();
	ALight(std::string actorName, World* world);
	virtual ~ALight();
public:
	void Tick(const float& deltaTime) override;

public:
	void Initialize(std::shared_ptr<StaticMesh> mesh, const ActorData& ad, const LightData& ld);
	float m_rotSpeed = 0.f;
};