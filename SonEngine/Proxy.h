#pragma once

#include "StaticMesh.h"

enum DirtyFlagType {
	DFT_add,
	DFT_update,
	DFT_remove
};

struct Proxy {
	UINT id = 0;
	StaticMesh* mesh = nullptr;
	LocalConstant constant;
};

struct PrimitiveProxy {
	UINT id = 0;
	std::shared_ptr<StaticMesh> mesh;
	LocalConstant constant;
	std::vector<bool> addDirtyFlags;
	std::vector<bool> updateDirtyFlags;
	std::string name;
	std::string textureName;
	std::string psoName;
};

struct TextProxy {
	StaticMesh* mesh = nullptr;
	std::wstring str = L"hello";
};