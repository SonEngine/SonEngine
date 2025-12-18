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
	void SetRotation(const DirectX::SimpleMath::Matrix& mat);
	void Translate(const DirectX::SimpleMath::Vector3& del);
	void UpdateLocation();
	void UpdateRotation();

public:
	StaticMesh* GetMesh() const { return m_mesh.get(); }

private:
	std::shared_ptr<StaticMesh> m_mesh;
};