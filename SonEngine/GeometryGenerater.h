#pragma once

#include "Mesh.h"
#include "Vertex.h"

struct GeometryGenerator
{
	static Mesh<SimpleVertex, uint16_t> MakeSimpleBox(int x, int y);
	static Mesh<SimpleVertex, uint16_t> MakeSimpleRect(float x, float y);

};