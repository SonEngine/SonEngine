#pragma once

#include "Constants.h"
#include "PaintBoardHLSLCompat.h"
#include "PhongHLSLCompat.h"

struct FramePacket
{
	uint64_t frameId = 0;
	PhongGlobalConstant gc;
	PBGlobalConstant pbgc;
	float deltaTime;
};