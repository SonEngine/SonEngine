#include "ModelLoader.h"
#include  "GeometryGenerator.h"

void ModelLoader<Vertex, uint16_t>::Initialize(ID3D12Device5* device,
	ID3D12GraphicsCommandList* commandList)
{
	Asset<Vertex, uint16_t> cube;
	cube.m_meshes.push_back({ GeometryGenerator::MakeCube(1.f, 1.f, 1.f) });
	assets["cube"] = cube;

	float sphereRadius = 0.5f;
	int sphereDetail = 30;
	Asset<Vertex, uint16_t> sphere;
	sphere.m_meshes.push_back({ GeometryGenerator::MakeSphere(sphereDetail, sphereRadius) });

	int planeSize = 100;
	Asset<Vertex, uint16_t> plane;
	plane.m_meshes.push_back({ GeometryGenerator::MakePlane((float)planeSize, (float)planeSize, 50) });

	std::shared_ptr<StaticMesh> cubeMesh = std::make_shared<StaticMesh>();
	cubeMesh->Initialize<Vertex, uint16_t>(device, commandList, cube.m_meshes);
	
	std::shared_ptr<StaticMesh> torusMesh = std::make_shared<StaticMesh>();
	torusMesh->Initialize<Vertex, uint16_t>(device, commandList, assets["tours"].m_meshes);

	std::shared_ptr<StaticMesh> planeMesh = std::make_shared<StaticMesh>();
	planeMesh->Initialize<Vertex, uint16_t>(device, commandList, plane.m_meshes);

	std::shared_ptr<StaticMesh> sphereMesh = std::make_shared<StaticMesh>();
	sphereMesh->Initialize<Vertex, uint16_t>(device, commandList, sphere.m_meshes);


	meshesMap["cube"] = cubeMesh;
	meshesMap["tours"] = torusMesh;
	meshesMap["plane"] = planeMesh;
	meshesMap["sphere"] = sphereMesh;
	
}

void ModelLoader<PBRVertex, uint16_t>::Initialize(ID3D12Device5* device,
	ID3D12GraphicsCommandList* commandList)
{
	float sphereRadius = 0.5f;
	int sphereDetail = 100;
	Asset<PBRVertex, uint16_t> sphere;
	sphere.m_meshes.push_back({ GeometryGenerator::MakePBRSphere(sphereDetail, sphereRadius) });

	std::shared_ptr<StaticMesh> sphereMesh = std::make_shared<StaticMesh>();
	sphereMesh->Initialize<PBRVertex, uint16_t>(device, commandList, sphere.m_meshes);

	meshesMap["sphere"] = sphereMesh;
}