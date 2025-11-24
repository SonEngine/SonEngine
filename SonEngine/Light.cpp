#include "Light.h"

Light::Light()
	:Actor()
{
	std::shared_ptr<SceneComponent> c = std::make_shared<SceneComponent>(this);
	m_rootComponent = c;
}

Light::Light(std::string actorName, World* world)
	:Actor(actorName, world)
{
}

Light::~Light()
{
}
