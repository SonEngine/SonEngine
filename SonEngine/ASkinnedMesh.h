#pragma once

#include "Actor.h"
#include "ActorData.h"
//#include "StaticMesh.h"

class StaticMesh;

class ASkinnedMesh : public Actor {
public:
	ASkinnedMesh();
	ASkinnedMesh(std::string actorName, World* world);
	virtual ~ASkinnedMesh();

public:
	
	void Initialize(std::shared_ptr<StaticMesh> mesh, const ActorData& ad) override;

public:
	void Tick(const float& deltaTime) override;


};