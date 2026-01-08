#pragma once

#include "Actor.h"

class ATriggerBox : public Actor {
public:
	ATriggerBox();
	ATriggerBox(std::string actorName, World* world);
	virtual ~ATriggerBox();

public:
	void Initialize(ID3D12Device5* device, ID3D12GraphicsCommandList* commandList, const std::string& texName, DirectX::SimpleMath::Matrix transform);
	void Initialize(std::shared_ptr<StaticMesh> mesh, const ActorData& ad) override;

public:
	void SetTarget(Actor* target) {	m_target = target; }

public:
	void OnBeginTrigger(class PrimitiveComponent* primitive);
	void OnEndTrigger(PrimitiveComponent* primitive);

private:
	Actor* m_target;
};