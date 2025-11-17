#pragma once

#include "Actor.h"

class ATriggerBox : public Actor {
public:
	ATriggerBox();
	ATriggerBox(std::string actorName, World* world);
	virtual ~ATriggerBox();

public:
	void OnBeginTrigger(class PrimitiveComponent* primitive);

public:
	void Initialize(ID3D12Device5* device, ID3D12GraphicsCommandList* commandList, const std::string& texName, DirectX::SimpleMath::Matrix transform);
};