#pragma once

#include "d3d12.h"
#include "Constants.h"
#include "wrl.h"

class FrameResource {
public:
	FrameResource() {};

	void Initialize();

private:
	PhongGlobalConstant phongGC;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_phongGCBuffer;
	void* pPhongGCB = nullptr;

public:
	UINT64 m_currentFence = 0;
};