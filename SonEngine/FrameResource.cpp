#include "FrameResource.h"
#include "GraphicsCommon.h"
#include <iostream>

void FrameResource::Initialize() {

	if (Graphics::utility == nullptr)
	{
		std::cout << "Failed FrameResource::Initialize -> Graphics::utility == nullptr\n";
	}
	Graphics::utility->CreateConstantBuffer(
		sizeof(GlobalConstant),
		m_phongGCBuffer,
		reinterpret_cast<void**>(&pPhongGCB)
	);
}