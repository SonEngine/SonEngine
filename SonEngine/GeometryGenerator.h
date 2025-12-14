#pragma once

#include "Mesh.h"
#include "Vertex.h"

struct GeometryGenerator
{
	static Mesh<SimpleVertex, uint16_t> MakeSimpleRect(float x, float y);
	static Mesh<SimpleVertex, uint16_t> MakeSimpleCube(float x, float y, float z);

	static Mesh<Vertex, uint16_t> MakeCube(float x, float y, float z);

	static Mesh<Vertex, uint16_t> MakePlane(float x, float z, int c);

	static Mesh<Vertex, uint16_t> GeometryGenerator::MakeSphere(int c, float r);

	static Mesh<PointCloudVertex, uint16_t> MakePointCube(float x, float y, float z);

};