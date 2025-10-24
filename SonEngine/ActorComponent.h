#pragma once

class Actor;

class ActorComponent
{
public:
	ActorComponent(Actor* owner);
	virtual ~ActorComponent();

protected:
	virtual void Initialize();
	
public:
	virtual void Tick(const float& deltaTime);
	
private:
	Actor* m_owner;
};