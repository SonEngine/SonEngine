#pragma once

#include "directxtk12/SimpleMath.h"

using DirectX::SimpleMath::Vector4;
using DirectX::SimpleMath::Vector3;
using DirectX::SimpleMath::Vector2;

struct PositionVertex {
	Vector3 position;
};

struct SimpleVertex {
	Vector3 position;
	Vector2 uv;
};

struct Vertex {
	Vector3 position;
	Vector3 normal;
	Vector2 uv;
};

struct PointCloudVertex {
	Vector3 position;
	Vector4 color;
};