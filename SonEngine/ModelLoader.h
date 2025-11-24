#pragma once

#include <assimp\Importer.hpp>
#include <assimp\postprocess.h>
#include <assimp\scene.h>

#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <unordered_map>

#include "Vertex.h"
#include "Asset.h"



template<typename V, typename I>
class ModelLoader {
public:
	ModelLoader() {
		basePath = std::filesystem::current_path().string();
		basePath += "/Models/";
	};
	~ModelLoader() {};

public:
	void Initialize();

public:
	std::string basePath;
	std::vector<Mesh<V, I>> GetMeshes(const std::string& assetName) const;
	DirectX::SimpleMath::Vector3 aiToVector3(aiVector3D vector);

private:
	std::unordered_map<std::string, Asset<V,I>> assets;

public:
	void Load(std::string filename, DirectX::SimpleMath::Matrix tr = DirectX::SimpleMath::Matrix());
	void ProcessNode(std::vector<Mesh<V, I>> & meshes, aiNode* node, const aiScene* scene, DirectX::SimpleMath::Matrix tr);
	void ProcessMesh(std::vector<Mesh<V, I>>& meshes, aiMesh* mesh, const aiScene* scene, DirectX::SimpleMath::Matrix tr);

};

template<typename V, typename I>
inline std::vector<Mesh<V, I>> ModelLoader<V, I>::GetMeshes(const std::string& assetName) const
{
	auto it = assets.find(assetName);
	if(it == assets.end())
		return std::vector<Mesh<V, I>>();

	return it->second.m_meshes;
}

template<typename V, typename I>
inline DirectX::SimpleMath::Vector3 ModelLoader<V, I>::aiToVector3(aiVector3D vector)
{
	DirectX::SimpleMath::Vector3 v(vector.x, vector.y, vector.z);
	return v;
}

template<typename V, typename I>
inline void ModelLoader<V,I>::Load(std::string filename, DirectX::SimpleMath::Matrix tr)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(basePath + filename,
		aiProcess_ConvertToLeftHanded | aiProcess_Triangulate);

	if (scene == nullptr)
	{
		return;
	}
	Asset<V, I> asset;
	ProcessNode(asset.m_meshes, scene->mRootNode, scene, tr);
	std::filesystem::path p = filename;
	assets[p.stem().string()] = asset;
}

template<typename V, typename I>
inline void ModelLoader<V, I>::ProcessNode(std::vector<Mesh<V, I>>& meshes, aiNode* node, const aiScene* scene, DirectX::SimpleMath::Matrix tr)
{
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		ProcessMesh(meshes, mesh, scene, tr);
	}

	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		ProcessNode(meshes, node->mChildren[i], scene, tr);
	}
}

template<typename V, typename I>
inline void ModelLoader<V, I>::ProcessMesh(std::vector<Mesh<V, I>>& meshes, aiMesh* mesh, const aiScene* scene, DirectX::SimpleMath::Matrix tr)
{
	Mesh<V, I> meshData;
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
