#pragma once

#include "StaticMesh.h"
#include <variant>
#include "directxtk12/SimpleMath.h"

#include "PBRHLSLCompat.h"

enum MeshType {
	MT_primitive,
	MT_pointCloud,
	MT_cubeMap,
	MT_dot
};

struct CmdAddPrimitive {
	UINT id;
	std::shared_ptr<StaticMesh> mesh;
	LocalConstant constant;
	MeshType meshType;
	std::string name;
	std::string textureName;
	std::string psoName;
};

struct CmdUpdatePrimitive {
	UINT id;
	LocalConstant constant;
	MeshType meshType;
};

using RenderCmd = std::variant<
	CmdAddPrimitive,
	CmdUpdatePrimitive
>;
