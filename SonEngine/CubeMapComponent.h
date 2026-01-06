#pragma once

#include "PrimitiveComponent.h"

class StaticMesh;

class CubeMapComponent : public PrimitiveComponent
{
public:
	CubeMapComponent(Actor* owner);
	virtual ~CubeMapComponent();

public:
	void SetMesh(std::shared_ptr<StaticMesh> newMesh);

public:
	StaticMesh* GetMesh() const { return m_mesh.get(); }
	std::shared_ptr<StaticMesh> GetMeshPtr() const { return m_mesh; }

private:
	std::shared_ptr<StaticMesh> m_mesh;
};