#pragma once

#include "Actor.h"

class APointCloud : public Actor {
public:
	APointCloud();
	APointCloud(std::string actorName, World* world);
	virtual ~APointCloud();

public:
	void Initialize(std::shared_ptr<StaticMesh> mesh, const ActorData& ad) override;

public:
	void SetTarget(Actor* target) { m_target = target; }

public:
	void OnBeginTrigger(class PrimitiveComponent* primitive);
	void OnEndTrigger(PrimitiveComponent* primitive);

private:
	Actor* m_target;
};