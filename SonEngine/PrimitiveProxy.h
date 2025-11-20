#pragma once

#include "d3d12.h"
#include "wrl.h"
#include "Constants.h"

class PrimitiveProxy {
public:
	PrimitiveProxy();
	virtual ~PrimitiveProxy();
	
public:
	virtual void Draw(ID3D12GraphicsCommandList* commandList);

protected:
	Microsoft::WRL::ComPtr<ID3D12Resource> localCB;
	LocalConstant localConstantData;
	void* pLocalCB;
};