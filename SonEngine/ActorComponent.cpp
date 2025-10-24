#include "ActorComponent.h"
#include "Actor.h"

ActorComponent::ActorComponent(Actor* owner)
	:m_owner(owner)
{
}

ActorComponent::~ActorComponent()
{
}

void ActorComponent::Initialize()
{
}

void ActorComponent::Tick(const float& deltaTime)
{
}
