#pragma once

#include "PrimitiveProxy.h"
#include "StaticMesh.h"

class StaticMeshProxy : public PrimitiveProxy {
public:
	StaticMeshProxy();
	virtual ~StaticMeshProxy();

	void Draw(ID3D12GraphicsCommandList* commandList) override;

private:
	StaticMesh* staticMesh;

};

