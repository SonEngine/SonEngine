#pragma once

#include "Proxy.h"
#include "RenderCommand.h"
#include <unordered_map>

class Scene {
public:
	Scene(const uint32_t frCount = 3);

	void Apply(const AddCmd& cmd);

	void Apply(const UpdateCmd& cmd);

	//void Apply(const RenderCmd& cmd);

	void ApplyAddImpl(const CmdAddPrimitive& c);
	void ApplyAddImpl(const CmdAddSkinnedMesh& c);
	void ApplyUpdateImpl(const CmdUpdatePrimitive& c);

	void ApplyUpdateImpl(const CmdUpdateSkinnedMesh& c);

public:
	std::unordered_map<MeshType, std::unordered_map<uint32_t, PrimitiveProxy>> m_proxies;
	std::unordered_map<uint32_t, PrimitiveProxy> m_pcProxies;
	std::unordered_map<uint32_t, PrimitiveProxy> m_dotProxies;
	std::unordered_map<uint32_t, PrimitiveProxy> m_cubeMapProxies;

private:
	uint32_t m_frCount;
};