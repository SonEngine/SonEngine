#include "Scene.h"

Scene::Scene(const uint32_t frCount):
	m_frCount(frCount)
{
}

void Scene::Apply(const RenderCmd& cmd)
{
	std::visit([&](auto&& c) { ApplyImpl(c); }, cmd);
}

void Scene::ApplyImpl(const CmdAddPrimitive& c)
{
	PrimitiveProxy p;
	p.id = c.id;
	p.constant = c.constant;
	p.mesh = c.mesh;
	p.addDirtyFlags.resize(m_frCount, true);
	p.updateDirtyFlags.resize(m_frCount, false);
	p.name = c.name;
	p.textureName = c.textureName;

	m_proxies[c.meshType][c.id] = p;
}

void Scene::ApplyImpl(const CmdUpdatePrimitive& c)
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
