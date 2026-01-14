#pragma once

#include "Actor.h"

class ACubeMap : public Actor {
public:
	ACubeMap();
	ACubeMap(std::string actorName, World* world);
	virtual ~ACubeMap();

public:
	void Initialize(std::shared_ptr<StaticMesh> mesh, const ActorData& ad) override;
};