#pragma once

#include "PrimitiveComponent.h"
#include "StaticMesh.h"

class StaticMeshComponent : public PrimitiveComponent
{
public:
	StaticMeshComponent(Actor* owner);

public:
	void Render(ID3D12GraphicsCommandList* commandList, const TextureLoader* texLoader) override;

	void SetMesh(std::shared_ptr<StaticMesh> newMesh);

private:
	std::shared_ptr<StaticMesh> m_mesh;

};