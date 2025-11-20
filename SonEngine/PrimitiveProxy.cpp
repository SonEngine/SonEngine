#include "PrimitiveProxy.h"
#include "GraphicsCommon.h"


PrimitiveProxy::PrimitiveProxy()
{
	Graphics::utility->CreateConstantBuffer(
		sizeof(LocalConstant),
		localCB,
		reinterpret_cast<void**>(&pLocalCB)
	);

	memcpy(
		pLocalCB,
		&localConstantData,
		sizeof(LocalConstant)
	);
}

PrimitiveProxy::~PrimitiveProxy()
{
}

void PrimitiveProxy::Draw(ID3D12GraphicsCommandList* commandList)
{
}

