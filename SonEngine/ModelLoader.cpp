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
// BOOKMARK
void ModelLoader<PBRVertex, uint16_t>::Initialize(ID3D12Device5* device,
	ID3D12GraphicsCommandList* commandList)
{
	float sphereRadius = 0.5f;
	int sphereDetail = 100;

	Asset<PBRVertex, uint16_t> sphere;
	//sphere.m_meshes.push_back({ GeometryGenerator::MakePBRSphere(sphereDetail, sphereRadius) });
	sphere.m_meshes.push_back({ GeometryGenerator::PbrSphere(0.5f, 100, 100) });

	float planeSize = 2.f * 100.f;
	int div = 200;

	Asset<PBRVertex, uint16_t> plane;
	plane.m_meshes.push_back({ GeometryGenerator::PBRPlane(planeSize, planeSize, div, div) });

	Asset<PBRVertex, uint16_t> cube;
	cube.m_meshes.push_back({ GeometryGenerator::PBRCube(0.5,0.5,0.5,40,40,40) });

	Asset<PBRVertex, uint16_t> simpleCube;
	simpleCube.m_meshes.push_back({ GeometryGenerator::PBRCube(0.5,0.5,0.5,1,1,1) });

	std::shared_ptr<StaticMesh> sphereMesh = std::make_shared<StaticMesh>();
	sphereMesh->Initialize<PBRVertex, uint16_t>(device, commandList, sphere.m_meshes);

	std::shared_ptr<StaticMesh> planeMesh = std::make_shared<StaticMesh>();
	planeMesh->Initialize<PBRVertex, uint16_t>(device, commandList, plane.m_meshes);

	std::shared_ptr<StaticMesh> sphereTanMesh = std::make_shared<StaticMesh>();
	sphereTanMesh->Initialize<PBRVertex, uint16_t>(device, commandList, assets["sphere"].m_meshes);

	std::shared_ptr<StaticMesh> cubeMesh = std::make_shared<StaticMesh>();
	cubeMesh->Initialize<PBRVertex, uint16_t>(device, commandList, cube.m_meshes);

	std::shared_ptr<StaticMesh> simpleCubeMesh = std::make_shared<StaticMesh>();
	simpleCubeMesh->Initialize<PBRVertex, uint16_t>(device, commandList, simpleCube.m_meshes);

	for (size_t i = 0; i < assets["large_castle_door_4k"].m_meshes.size(); i++)
	{
		std::string name = "door" + std::to_string(i);
		std::shared_ptr<StaticMesh> doorMesh = std::make_shared<StaticMesh>();
		doorMesh->Initialize<PBRVertex, uint16_t>(device, commandList, {assets["large_castle_door_4k"].m_meshes[i]});
		meshesMap[name] = doorMesh;
	}

	std::shared_ptr<StaticMesh> shieldMesh = std::make_shared<StaticMesh>();
	shieldMesh->Initialize<PBRVertex, uint16_t>(device, commandList, assets["SF_Demon_head_shield_NakedSingularity"].m_meshes);

	meshesMap["shield"] = shieldMesh;
	meshesMap["sphereTan"] = sphereTanMesh;
	meshesMap["sphere"] = sphereMesh;
	meshesMap["plane"] = planeMesh;
	meshesMap["cube"] = cubeMesh;
	meshesMap["simpleCube"] = simpleCubeMesh;
}


void ModelLoader<PointCloudVertex, uint16_t>::Initialize(ID3D12Device5* device,
	ID3D12GraphicsCommandList* commandList)
{

	std::shared_ptr<StaticMesh> mapMesh = std::make_shared<StaticMesh>();
	mapMesh->InitializePC<PointCloudVertex, uint16_t>(device, commandList, assets["map"].m_meshes);

	meshesMap["map"] = mapMesh;
}

void ModelLoader<SimpleVertex, uint16_t>::Initialize(ID3D12Device5* device,
	ID3D12GraphicsCommandList* commandList)
{
	int cubemapSize = 200;
	Asset<SimpleVertex, uint16_t> cube;
	cube.m_meshes.push_back({ GeometryGenerator::MakeSimpleCube(200,200,200) });

	Asset<SimpleVertex, uint16_t> point;
	point.m_meshes.push_back({ GeometryGenerator::MakePoint() });


	std::shared_ptr<StaticMesh> cubeMapMesh = std::make_shared<StaticMesh>();
	cubeMapMesh->Initialize<SimpleVertex, uint16_t>(device, commandList, cube.m_meshes);

	std::shared_ptr<StaticMesh> pointMesh = std::make_shared<StaticMesh>();
	pointMesh->InitializePC<SimpleVertex, uint16_t>(device, commandList, point.m_meshes);

	meshesMap["cubeMap"] = cubeMapMesh;
	meshesMap["point"] = pointMesh;
}

void ModelLoader<Vertex, uint16_t>::ProcessMesh(std::vector<Mesh<Vertex, uint16_t>>& meshes, aiMesh* mesh, const aiScene* scene, DirectX::SimpleMath::Matrix tr)
{
	Mesh<Vertex, uint16_t> meshData;
	for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
		aiVector3D v = mesh->mVertices[i];
		aiVector3D n = aiVector3D(0, 1, 0);
		if (mesh->HasNormals())
		{
			n = mesh->mNormals[i];
		}

		meshData.m_vertices.push_back({
				aiToVector3(v),
				aiToVector3(n),
				Vector2(0,0)
			});
	}

	for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
		aiFace face = mesh->mFaces[i];

		for (unsigned int j = 0; j < face.mNumIndices; j++) {
			meshData.m_indices.push_back(face.mIndices[j]);
		}
	}

	meshes.push_back(meshData);
}


void ModelLoader<PBRVertex, uint16_t>::ProcessMesh(std::vector<Mesh<PBRVertex, uint16_t>>& meshes, aiMesh* mesh, const aiScene* scene, DirectX::SimpleMath::Matrix tr)
{
	Mesh<PBRVertex, uint16_t> meshData;
	for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
		aiVector3D v = mesh->mVertices[i];
		aiVector3D n = aiVector3D(0, 1, 0);
		aiVector3D t = aiVector3D(1, 0, 0);
		aiVector3D uv = aiVector3D(0, 0, 0);
		if (mesh->HasNormals())
		{
			n = mesh->mNormals[i];
		}
		if (mesh->HasTangentsAndBitangents())
		{
			t = mesh->mTangents[i];
		}
		if (mesh->HasTextureCoords(0))
		{
			uv = mesh->mTextureCoords[0][i];
		}
		Vector3 vecUV = aiToVector3(uv);

		Vector3 pos = aiToVector3(v);
		Vector3 normal = aiToVector3(n);
		Vector3 tangent = aiToVector3(t);

		Matrix invTranspose = tr.Invert().Transpose();
		pos = Vector3::Transform(pos, tr);
		normal = Vector3::Transform(normal, invTranspose);
		tangent = Vector3::Transform(tangent, tr);

		normal.Normalize();
		tangent.Normalize();

		meshData.m_vertices.push_back({
			pos, normal, tangent, Vector2(vecUV.x,vecUV.y)
		});
	}

	for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
		aiFace face = mesh->mFaces[i];

		for (unsigned int j = 0; j < face.mNumIndices; j++) {
			meshData.m_indices.push_back(face.mIndices[j]);
		}
	}

	meshes.push_back(meshData);
}