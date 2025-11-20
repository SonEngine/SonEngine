#pragma once

#include <vector>
#include <memory>

#include "ActorComponent.h"
#include "directxtk12/SimpleMath.h"
#include "Transform.h"

// transform
class SceneComponent : public ActorComponent {
public:
	SceneComponent(Actor* owner);
	virtual ~SceneComponent();

public:
	void SetSpeed(const float& newSpeed);
	void SetRotateSpeed(const float& newSpeed);
	void SetLocation(const DirectX::SimpleMath::Vector3& newLocation);
	void SetRotation(const DirectX::SimpleMath::Quaternion& newQuat);
	void SetFrontDirection(const DirectX::SimpleMath::Vector3& newDir) { m_frontDirection = newDir; }
	void SetUpDirection(const DirectX::SimpleMath::Vector3& newDir) {m_upDirection = newDir;}
	void SetRightDirection(const DirectX::SimpleMath::Vector3& newDir) { m_rightDirection = newDir; }
	void AddLocation(const DirectX::SimpleMath::Vector3& delLocation);
	void AddRotation(const DirectX::SimpleMath::Quaternion& delQ);

public:
	float GetSpeed() const { return m_speed; }
	float GetRotateSpeed() const { return m_rotateSpeed; }
	DirectX::SimpleMath::Vector3 GetFrontDirection() const { return m_frontDirection; }
	DirectX::SimpleMath::Vector3 GetBaseFrontDirection() const { return m_baseFrontDirection; }
	DirectX::SimpleMath::Vector3 GetBaseUpDirection() const { return m_baseUpDirection; }
	DirectX::SimpleMath::Vector3 GetUpDirection() const { return m_upDirection; }
	DirectX::SimpleMath::Vector3 GetRightDirection()const { return m_rightDirection; }
	
	DirectX::SimpleMath::Vector3 GetLocation()const { return worldTransform.location + localTransform.location; }
	DirectX::SimpleMath::Quaternion GetRotation()const { return worldTransform.quat * localTransform.quat; }
	DirectX::SimpleMath::Matrix GetViewMatrix() const;
	void GetChildrenComponents(std::vector<std::shared_ptr<SceneComponent>>& children) const;


public:
	// world->actor 호출
	virtual void OnRegister();
	
protected:
	//DirectX::SimpleMath::Vector3 m_location;
	//DirectX::SimpleMath::Matrix m_rotation;
	Transform worldTransform;
	Transform localTransform;

	DirectX::SimpleMath::Vector3 m_baseUpDirection;
	DirectX::SimpleMath::Vector3 m_baseFrontDirection;
	DirectX::SimpleMath::Vector3 m_frontDirection;
	DirectX::SimpleMath::Vector3 m_upDirection;
	DirectX::SimpleMath::Vector3 m_rightDirection;

protected:
	float m_speed = 1.f;
	float m_rotateSpeed = 1.f;

protected:
	std::shared_ptr<SceneComponent> m_parent;
	std::vector<std::shared_ptr<SceneComponent>> m_children;
};