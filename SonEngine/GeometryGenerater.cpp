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

Mesh<SimpleVertex, uint16_t> GeometryGenerator::MakeSimpleCube(float x, float y, float z)
{
	float halfX = x / 2.f;
	float halfY = y / 2.f;
	float halfZ = z / 2.f;
	std::vector<Vector3> v{
		Vector3(-halfX, -halfY, -halfZ),
		Vector3(-halfX, halfY,  -halfZ),
		Vector3(halfX, halfY,  -halfZ),
		Vector3(halfX, -halfY,  -halfZ),
		Vector3(-halfX, -halfY, halfZ),
		Vector3(-halfX, halfY,  halfZ),
		Vector3(halfX, halfY,  halfZ),
		Vector3(halfX, -halfY,  halfZ)
	};
	std::vector<std::tuple<int, int, int, int>> vSet
	{
		{0,1,2,3},	// front
		{3,2,6,7},	// right
		{4,5,1,0},	// left
		{7,6,5,4},	// back
		{4,0,3,7},  // bottom
		{1,5,6,2},  // top
	};
	std::vector<SimpleVertex> vertices;
	std::vector<uint16_t> indices;

	for (size_t i = 0; i < vSet.size(); i++)
	{
		auto & [v0, v1, v2, v3] = vSet[i];

		vertices.push_back({ v[v0], Vector2(0,1) });
		vertices.push_back({ v[v1], Vector2(0,0) });
		vertices.push_back({ v[v2], Vector2(1,0) });
		vertices.push_back({ v[v3], Vector2(1,1) });

		int base = i * 4;
		indices.push_back(base + 0);
		indices.push_back(base + 1);
		indices.push_back(base + 2);

		indices.push_back(base + 0);
		indices.push_back(base + 2);
		indices.push_back(base + 3);
	}

	Mesh<SimpleVertex, uint16_t> mesh;

	mesh.m_vertices = vertices;
	mesh.m_indices = indices;

	return mesh;
}
