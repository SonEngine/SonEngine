#pragma once

#include "PrimitiveComponent.h"

class StaticMesh;

class DotComponent : public PrimitiveComponent
{
public:
	DotComponent(Actor* owner);
	virtual ~DotComponent();

public:
	void SetMesh(std::shared_ptr<StaticMesh> newMesh);


public:
	StaticMesh* GetMesh() const { return m_mesh.get(); }
	std::shared_ptr<StaticMesh> GetMeshPtr() const { return m_mesh; }

private:
	std::shared_ptr<StaticMesh> m_mesh;
};