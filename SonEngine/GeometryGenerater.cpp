#include "GeometryGenerater.h"

Mesh<SimpleVertex, uint16_t> GeometryGenerator::MakeSimpleBox(int x, int y)
{
    return Mesh<SimpleVertex, uint16_t>();
}

Mesh<SimpleVertex, uint16_t> GeometryGenerator::MakeSimpleRect(float x, float y)
{
	float halfX = x / 2.f;
	float halfY = y / 2.f;
	std::vector<SimpleVertex> vertices{
		{Vector3(-halfX, -halfY, 1), Vector2(0, 1)},
		{Vector3(-halfX, halfY, 1), Vector2(0, 0)},
		{Vector3(halfX, halfY, 1), Vector2(1, 0)},
		{Vector3(halfX, -halfY, 1), Vector2(1, 1)}
	};
	std::vector<uint16_t> indices{
		0, 1, 2, 0, 2, 3
	};

    Mesh<SimpleVertex, uint16_t> mesh;

	mesh.m_vertices = vertices;
	mesh.m_indices = indices;

	return mesh;
}
