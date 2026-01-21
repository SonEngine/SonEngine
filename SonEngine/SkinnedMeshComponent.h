#pragma once

#include "PrimitiveComponent.h"

class StaticMesh;

class SkinnedMeshComponent : public PrimitiveComponent
{
public:
	SkinnedMeshComponent(Actor* owner);
	virtual ~SkinnedMeshComponent();

public:
	void SetMesh(std::shared_ptr<StaticMesh> newMesh);

public:
	StaticMesh* GetMesh() const { return m_mesh.get(); }
	std::shared_ptr<StaticMesh> GetMeshPtr() const { return m_mesh; }
	SkinnedLocalConstant GetSkinnedLocalConstant() { return m_skinnedLocalConstant; }

public:
	void SetActorData(const ActorData& ad) override;
	void SetAnimationSpeed(const float& newSpeed) { m_animationSpeed = newSpeed; }
	void UpdateAnimation(const float& deltaTime);

private:
	bool bUpdateAnim = true;
	bool bUpdateRoot = true;
	float m_currentFrame = 0.f;
	float m_animationSpeed = 60.f;
	std::shared_ptr<StaticMesh> m_mesh;
	SkinnedLocalConstant m_skinnedLocalConstant;
};