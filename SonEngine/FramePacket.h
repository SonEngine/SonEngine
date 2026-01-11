#pragma once

#include "directxtk12\SimpleMath.h"
#include "PaintBoardHLSLCompat.h"
#include "PBRHLSLCompat.h"

struct FramePacket
{
	uint64_t frameId = 0;
	PBRGlobalConstant gc;
	PBGlobalConstant pbgc;
	float deltaTime;
};