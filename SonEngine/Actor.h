#pragma once

#include <memory>
#include "d3d12.h"
#include "directxtk12\SimpleMath.h"

#include "TextureLoader.h"
#include "SceneComponent.h"

class Actor
{
public:
	Actor();
	Actor(std::string actorName);

protected:
	virtual void Initialize();

public:
	virtual void Tick(const float& deltaTime);

public:
	void SetActorLocation(const DirectX::SimpleMath::Vector3& newLocation);
	void UpdateActorLocation(const DirectX::SimpleMath::Vector3& delLocation);
	void SetActorSpeed(const float& newSpeed);
	void SetRootComponent(std::shared_ptr<SceneComponent> newRootComponent);

public:
	DirectX::SimpleMath::Vector3 GetActorLocation() const;
	DirectX::SimpleMath::Vector3 GetActorFrontDir() const;
	DirectX::SimpleMath::Vector3 GetActorUpDir() const;
	DirectX::SimpleMath::Vector3 GetActorRightDir() const;
	float GetActorSpeed() const;
	DirectX::SimpleMath::Matrix GetViewMatrix() const;

protected:
	std::shared_ptr<SceneComponent> m_rootComponent;
	std::string m_name;
};