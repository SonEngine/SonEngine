#pragma once

#include "Actor.h"
#include "ActorData.h"
//#include "StaticMesh.h"

class StaticMesh;
class StaticMeshComponent;
class SkinnedMeshComponent;

class ASkinnedMesh : public Actor {
public:
	ASkinnedMesh();
	ASkinnedMesh(std::string actorName, World* world);
	virtual ~ASkinnedMesh();

public:
	
	void Initialize(std::shared_ptr<StaticMesh> mesh, const ActorData& ad, const AnimData& animData);

	void UpdateSocketMatrix(const Vector3 rot, const Vector3& t);

public:
	void Tick(const float& deltaTime) override;

private:
	std::shared_ptr<SkinnedMeshComponent> root;
	std::shared_ptr<StaticMeshComponent> meshCmp;
	DirectX::SimpleMath::Matrix meshLocalMat;
};