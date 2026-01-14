#pragma once

#include "Actor.h"

class ADot : public Actor {
public:
	ADot();
	ADot(std::string actorName, World* world);
	virtual ~ADot();

public:
	void Initialize(std::shared_ptr<StaticMesh> mesh, const ActorData& ad) override;
};