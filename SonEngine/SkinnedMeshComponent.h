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

	void UpdateAnimation(const float& deltaTime);

private:
	bool bUpdateAnim = true;
	std::shared_ptr<StaticMesh> m_mesh;
	SkinnedLocalConstant m_skinnedLocalConstant;
};