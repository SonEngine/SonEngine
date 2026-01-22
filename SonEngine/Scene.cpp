#include "Scene.h"

Scene::Scene(const uint32_t frCount):
	m_frCount(frCount)
{
}

void Scene::Apply(const AddCmd& cmd)
{
	std::visit([&](auto&& c) { ApplyAddImpl(c); }, cmd);
}

void Scene::Apply(const UpdateCmd& cmd)
{
	std::visit([&](auto&& c) { ApplyUpdateImpl(c); }, cmd);
}

void Scene::ApplyAddImpl(const CmdAddPrimitive& c)
{
	PrimitiveProxy p;
	p.id = c.id;
	p.constant = c.constant;
	p.mesh = c.mesh;
	p.addDirtyFlags.resize(m_frCount, true);
	p.updateDirtyFlags.resize(m_frCount, false);
	p.name = c.name;
	p.textureName = c.textureName;
	p.psoName = c.psoName;

	m_proxies[c.meshType][c.id] = p;
}

void Scene::ApplyAddImpl(const CmdAddSkinnedMesh& c)
{
	PrimitiveProxy p;
	p.id = c.id;
	p.constant = c.constant;
	p.skinnedlc = c.skinnedlc;
	p.mesh = c.mesh;
	p.addDirtyFlags.resize(m_frCount, true);
	p.updateDirtyFlags.resize(m_frCount, false);
	p.name = c.name;
	p.textureName = c.textureName;
	p.psoName = c.psoName;

	m_proxies[c.meshType][c.id] = p;
}


void Scene::ApplyUpdateImpl(const CmdUpdatePrimitive& c)
{
	auto it = m_proxies[c.meshType].find(c.id);
	if (it != m_proxies[c.meshType].end())
	{
		it->second.constant = c.constant;
		//it->second.type = DFT_update;
		for (auto& f : it->second.updateDirtyFlags)
			f = true;
	}
}

void Scene::ApplyUpdateImpl(const CmdUpdateSkinnedMesh& c)
{
	auto it = m_proxies[c.meshType].find(c.id);
	if (it != m_proxies[c.meshType].end())
	{
		it->second.constant = c.constant;
		it->second.skinnedlc = c.skinnedlc;

		//it->second.type = DFT_update;
		for (auto& f : it->second.updateDirtyFlags)
			f = true;
	}
}
