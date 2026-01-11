#include "GeometryGenerator.h"
#include <iostream>

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
		indices.push_back(uint16_t(base + 0));
		indices.push_back(uint16_t(base + 1));
		indices.push_back(uint16_t(base + 2));

		indices.push_back(uint16_t(base + 0));
		indices.push_back(uint16_t(base + 2));
		indices.push_back(uint16_t(base + 3));
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

		uint16_t base = i * 4;
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
				uint16_t a = (c + 1) * i + j;
				uint16_t b = a + c + 1;
				uint16_t c = a + 1;
				uint16_t d = b + 1;
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
				uint16_t i0 = (c + 1) * i + j;
				uint16_t i1 = i0 + c + 1;
				uint16_t i2 = i0 + 1;
				uint16_t i3 = i1 + 1;
				indices.push_back(i1);
				indices.push_back(i0);
				indices.push_back(i2);

				indices.push_back(i1);
				indices.push_back(i2);
				indices.push_back(i3);
			}
		}
	}


	Mesh<Vertex, uint16_t> mesh;

	mesh.m_vertices = vertices;
	mesh.m_indices = indices;

	return mesh;
}


Mesh<PBRVertex, uint16_t> GeometryGenerator::MakePBRSphere(int c, float r)
{
	float pi = (float)std::acos(-1);
	float thetaZ = pi / c;
	float thetaY = pi * 2.f / c;

	std::vector<PBRVertex> vertices;
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
			Vector3 pos = Vector3::Transform(baseY, yMat);
			Vector3 n = pos;
			n.Normalize();

			PBRVertex vertex;
			vertex.position = pos;
			vertex.normal = n;
			vertex.uv = Vector2(deluv * j, deluv * i);
			if (i == 0 || i == c)
			{
				//vertex.tangent = Vector3::Transform(Vector3(0, 0, -1), yMat);
				vertex.tangent = Vector3(0, 0, -1);
			}
			else
			{
				vertex.tangent = Vector3(0, 0, 0);
			}

			vertices.push_back(vertex);
			if (j != c && i != c)
			{
				uint16_t i0 = (c + 1) * i + j;
				uint16_t i1 = i0 + c + 1;
				uint16_t i2 = i0 + 1;
				uint16_t i3 = i1 + 1;
				indices.push_back(i1);
				indices.push_back(i0);
				indices.push_back(i2);

				indices.push_back(i1);
				indices.push_back(i2);
				indices.push_back(i3);
			}
		}
	}

	for (size_t i = 0; i < indices.size(); i += 3)
	{
		uint16_t i0 = indices[i];
		uint16_t i1 = indices[i + 1];
		uint16_t i2 = indices[i + 2];

		Vector3 t = CalculateTangent(vertices, i0, i1, i2);
		vertices[i0].tangent += t;
		vertices[i1].tangent += t;
		vertices[i2].tangent += t;

	}


	for (int i = 0; i <= c; i++)
	{
		for (int j = 0; j <= c; j++)
		{
			int idx = i * (c+1) + j;
			PBRVertex& v = vertices[idx];
			if (v.tangent == Vector3(0, 0, 0)/* || i == 0 || i == c*/)
			{
				v.tangent = Vector3(1, 0, 0);
			}

			if (j == 0)
			{
				Vector3 t = v.tangent;
				int lastIdx = idx + c;
				t += vertices[lastIdx].tangent;

				vertices[lastIdx].tangent = t;
				v.tangent = t;
			}
			v.tangent.Normalize();
		}
	}

	Mesh<PBRVertex, uint16_t> mesh;

	mesh.m_vertices = vertices;
	mesh.m_indices = indices;

	return mesh;
}


Mesh<PointCloudVertex, uint16_t> GeometryGenerator::MakePointCube(float x, float y, float z)
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


	std::vector<PointCloudVertex> vertices;
	std::vector<uint16_t> indices;

	for (size_t i = 0; i < v.size(); i++)
	{
		vertices.push_back({ v[i], Vector4(1,1,1,1) });
	}
	Mesh<PointCloudVertex, uint16_t> mesh;

	mesh.m_vertices = vertices;
	mesh.m_indices = indices;

	return mesh;
}


Mesh<SimpleVertex, uint16_t> GeometryGenerator::MakePoint()
{

	Mesh<SimpleVertex, uint16_t> mesh;
	std::vector< SimpleVertex> vertices;
	std::vector< uint16_t> indices;

	vertices.push_back({ Vector3(0,0,0) });
	mesh.m_vertices = vertices;
	mesh.m_indices = indices;

	return mesh;
}

Vector3 GeometryGenerator::CalculateTangent(const std::vector<PBRVertex>& vertices, int i0, int i1, int i2)
{
	Vector3 p0 = vertices[i0].position;
	Vector3 p1 = vertices[i1].position;
	Vector3 p2 = vertices[i2].position;

	Vector2 uv0 = vertices[i0].uv;
	Vector2 uv1 = vertices[i1].uv;
	Vector2 uv2 = vertices[i2].uv;

	Vector3 e0 = p1 - p0;
	Vector3 e1 = p2 - p0;
	

	if(e0.Cross(e1).LengthSquared() < 1e-5f)
		return Vector3(0.f, 0.f, 0.f);

	Vector2 delUV0 = uv1 - uv0;
	Vector2 delUV1 = uv2 - uv0;

	float a = delUV0.x;
	float b = delUV0.y;

	float c = delUV1.x;
	float d = delUV1.y;

	float det = a * d - b * c;
	if (std::abs(det) < 1e-8)
	{
		return Vector3(0.f, 0.f, 0.f);
	}

	const float r = 1.0f / det;

	Vector3 t = (e0 * d - e1 * b) * r;
	return t;
}
