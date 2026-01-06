#pragma once

#include "PrimitiveComponent.h"

class StaticMesh;

class PointCloudComponent : public PrimitiveComponent
{
public:
	PointCloudComponent(Actor* owner);
	virtual ~PointCloudComponent();

public:
	void SetMesh(std::shared_ptr<StaticMesh> newMesh);


public:
	StaticMesh* GetMesh() const { return m_mesh.get(); }
	std::shared_ptr<StaticMesh> GetMeshPtr() const { return m_mesh; }

private:
	std::shared_ptr<StaticMesh> m_mesh;
};