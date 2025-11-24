#include "ModelLoader.h"
#include  "GeometryGenerator.h"

void ModelLoader<Vertex, uint16_t>::Initialize()
{
	Asset<Vertex, uint16_t> cube;
	cube.m_meshes.push_back({ GeometryGenerator::MakeCube(1.f, 1.f, 1.f) });
	assets["cube"] = cube;
}