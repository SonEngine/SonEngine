#pragma once

#include "PrimitiveComponent.h"
#include "StaticMesh.h"

class StaticMeshComponent : public PrimitiveComponent
{
public:
	StaticMeshComponent(Actor* owner);
	virtual ~StaticMeshComponent();

public:
	void SetMesh(std::shared_ptr<StaticMesh> newMesh);

public:
	StaticMesh* GetMesh() const { return m_mesh.get(); }
private:
	std::shared_ptr<StaticMesh> m_mesh;
};