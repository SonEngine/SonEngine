#pragma once

#include "PrimitiveComponent.h"
#include "ActorData.h"

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
	void SetAnimationSpeed(const float& newSpeed) { m_animationSpeed = newSpeed; }
	void SetAnimationName(const std::string& newName) { m_animationName = newName; }
	void UpdateAnimation(const float& deltaTime);
	void SetAnimationData(const AnimData& animData);
	void SetPlayAnimation(const bool& playAnimation) { bUpdateAnim = playAnimation; }

private:
	bool bUpdateAnim = true;
	bool bUpdateRoot = true;
	float m_currentFrame = 0.f;
	float m_animationSpeed = 60.f;
	std::shared_ptr<StaticMesh> m_mesh;
	SkinnedLocalConstant m_skinnedLocalConstant;
	std::string m_animationName;
};