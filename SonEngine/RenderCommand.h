#pragma once

#include "StaticMesh.h"
#include <variant>

enum MeshType {
	MT_primitive,
	MT_pointCloud,
	MT_cubeMap,
	MT_dot
};

struct CmdAddPrimitive {
	long long id;
	std::shared_ptr<StaticMesh> mesh;
	LocalConstant constant;
	MeshType meshType;
	std::string name;
};

struct CmdUpdatePrimitive {
	long long id;
	LocalConstant constant;
	MeshType meshType;
};

using RenderCmd = std::variant<
	CmdAddPrimitive,
	CmdUpdatePrimitive
>;
