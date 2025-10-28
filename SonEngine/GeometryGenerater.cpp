#include "GeometryGenerater.h"

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
		auto& [v0, v1, v2, v3] = vSet[i];

		vertices.push_back({ v[v0], Vector2(0,1) });
		vertices.push_back({ v[v1], Vector2(0,0) });
		vertices.push_back({ v[v2], Vector2(1,0) });
		vertices.push_back({ v[v3], Vector2(1,1) });

		size_t base = i * 4;
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


Mesh<Vertex, uint16_t> GeometryGenerator::MakeCube(float x, float y, float z)
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
	std::vector<Vector3> nSet
	{
		{0, 0, -1},	 // front
		{1, 0, 0},	 // right
		{-1, 0, 0},	 // left
		{0, 0, 1},	 // back
		{0, -1,0},   // bottom
		{0, 1, 0},   // top
	};
	std::vector<Vertex> vertices;
	std::vector<uint16_t> indices;

	for (size_t i = 0; i < vSet.size(); i++)
	{
		auto& [v0, v1, v2, v3] = vSet[i];

		vertices.push_back({ v[v0], nSet[i], Vector2(0,1) });
		vertices.push_back({ v[v1], nSet[i], Vector2(0,0) });
		vertices.push_back({ v[v2], nSet[i], Vector2(1,0) });
		vertices.push_back({ v[v3], nSet[i], Vector2(1,1) });

		size_t base = i * 4;
		indices.push_back(base + 0);
		indices.push_back(base + 1);
		indices.push_back(base + 2);

		indices.push_back(base + 0);
		indices.push_back(base + 2);
		indices.push_back(base + 3);
	}

	Mesh<Vertex, uint16_t> mesh;

	mesh.m_vertices = vertices;
	mesh.m_indices = indices;

	return mesh;
}

Mesh<Vertex, uint16_t> GeometryGenerator::MakePlane(float x, float z, int c)
{
	float halfX = x / 2.f;
	float halfZ = z / 2.f;

	std::vector<Vertex> vertices;
	std::vector<uint16_t> indices;

	Vector3 baseV = Vector3(-halfX, 0, halfZ);
	float delX = x / c;
	float delZ = -z / c;

	float deluv = 1.f / c;
	for (int i = 0; i <= c; i++)
	{

		Vector3 xBase = baseV + Vector3(0.f, 0.f, delZ) * (float)i;
		for (int j = 0; j <= c; j++)
		{
			Vector3 v = xBase + (float)j * Vector3(delX, 0, 0);
			vertices.push_back({ v, Vector3(0, 1, 0), Vector2(deluv * j, deluv * i) });
			if (j != c && i != c)
			{
				int a = (c + 1) * i + j;
				int b = a + c + 1;
				int c = a + 1;
				int d = b + 1;
				indices.push_back(b);
				indices.push_back(a);
				indices.push_back(c);

				indices.push_back(b);
				indices.push_back(c);
				indices.push_back(d);
			}
		}
	}


	Mesh<Vertex, uint16_t> mesh;

	mesh.m_vertices = vertices;
	mesh.m_indices = indices;

	return mesh;
}
Mesh<Vertex, uint16_t> GeometryGenerator::MakeSphere(int c, float r)
{
	float pi = (float)std::acos(-1);
	float thetaZ = pi / c;
	float thetaY = pi * 2.f / c;

	std::vector<Vertex> vertices;
	std::vector<uint16_t> indices;


	Vector3 baseZ = Vector3(0, r, 0);

	float deluv = 1.f / c;
	for (int i = 0; i <= c; i++)
	{

		DirectX::SimpleMath::Matrix zMat = DirectX::XMMatrixRotationZ(thetaZ * i);
		Vector3 baseY = Vector3::Transform(baseZ, zMat);
		for (int j = 0; j <= c; j++)
		{
			DirectX::SimpleMath::Matrix yMat = DirectX::XMMatrixRotationY(-thetaY * j);
			Vector3 v = Vector3::Transform(baseY, yMat);
			Vector3 n = v;
			n.Normalize();
			vertices.push_back({ v,n, Vector2(deluv * j, deluv * i) });
			if (j != c && i != c)
			{
				int a = (c + 1) * i + j;
				int b = a + c + 1;
				int c = a + 1;
				int d = b + 1;
				indices.push_back(b);
				indices.push_back(a);
				indices.push_back(c);

				indices.push_back(b);
				indices.push_back(c);
				indices.push_back(d);
			}
		}
	}


	Mesh<Vertex, uint16_t> mesh;

	mesh.m_vertices = vertices;
	mesh.m_indices = indices;

	return mesh;
}