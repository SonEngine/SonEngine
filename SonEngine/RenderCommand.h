#pragma once

#include "StaticMesh.h"
#include <variant>
#include "directxtk12/SimpleMath.h"

#include "PBRHLSLCompat.h"

enum MeshType {
	MT_primitive,
	MT_pointCloud,
	MT_cubeMap,
	MT_light,
	MT_skinnedMesh,
	MT_finalize
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

struct CmdAddSkinnedMesh {
	UINT id;
	std::shared_ptr<StaticMesh> mesh;
	LocalConstant constant;
	MeshType meshType;
	std::string name;
	std::string textureName;
	std::string psoName;

	SkinnedLocalConstant skinnedlc;
};

struct CmdUpdatePrimitive {
	UINT id;
	LocalConstant constant;
	MeshType meshType;
};

struct CmdUpdateSkinnedMesh {
	UINT id;
	LocalConstant constant;
	MeshType meshType;
	SkinnedLocalConstant skinnedlc;
};

//using RenderCmd = std::variant<
//	CmdAddPrimitive,
//	CmdAddSkinnedMesh,
//	CmdUpdatePrimitive,
//	CmdUpdateSkinnedMesh
//>;
using AddCmd = std::variant<
	CmdAddPrimitive,
	CmdAddSkinnedMesh
>;

using UpdateCmd = std::variant<
	CmdUpdatePrimitive,
	CmdUpdateSkinnedMesh
>;