#include "SceneComponent.h"
#include "directxtk12/SimpleMath.h"
#include "StaticMeshComponent.h"

using DirectX::SimpleMath::Vector3;

SceneComponent::SceneComponent(Actor* owner)
	:ActorComponent(owner),
	m_frontDirection(Vector3(0, 0, 1)),
	m_baseFrontDirection(Vector3(0, 0, 1)),
	m_upDirection(Vector3(0, 1, 0)),
	m_baseUpDirection(Vector3(0, 1, 0)),
	m_rightDirection(Vector3(1, 0, 0))
{
}

SceneComponent::~SceneComponent()
{
}

void SceneComponent::SetSpeed(const float& newSpeed)
{
	m_speed = newSpeed;
}
void SceneComponent::SetRotateSpeed(const float& newSpeed)
{
	m_rotateSpeed = newSpeed;
}

void SceneComponent::SetLocation(const DirectX::SimpleMath::Vector3& newLocation)
{
	localTransform.location = newLocation;
	UpdateConstantLocation();
}

void SceneComponent::SetRotation(const DirectX::SimpleMath::Quaternion& newQuat)
{
	localTransform.quat = newQuat;
	UpdateConstantRotation();
}

void SceneComponent::SetCubeMapMipLevel(const int& newCubeMapMipLevel)
{
	localConstant.cubeMipLevel = newCubeMapMipLevel;
}

void SceneComponent::AddLocation(const DirectX::SimpleMath::Vector3& delLocation)
{
	localTransform.location += delLocation;
	UpdateConstantLocation();
}
void SceneComponent::AddRotation(const DirectX::SimpleMath::Quaternion& delQ)
{
	localTransform.quat *= delQ;
	UpdateConstantRotation();
}

DirectX::SimpleMath::Matrix SceneComponent::GetViewMatrix() const
{
	return XMMatrixLookToLH(GetLocation(), m_frontDirection, m_upDirection);
}

void SceneComponent::GetChildrenComponents(std::vector<std::shared_ptr<SceneComponent>>& children) const
{
	children = m_children;
}

void SceneComponent::OnRegister() 
{
	for (const auto & c : m_children)
	{
		c->OnRegister();
	}
}

void SceneComponent::UpdateConstantLocation()
{
	auto loc = GetLocation();
	localConstant.model.m[3][0] = loc.x;
	localConstant.model.m[3][1] = loc.y;
	localConstant.model.m[3][2] = loc.z;
	localConstant.modelInvTranspose = localConstant.model.Invert().Transpose();;
}

void SceneComponent::UpdateConstantRotation()
{
	auto q = GetRotation();
	DirectX::SimpleMath::Matrix mat = DirectX::XMMatrixRotationQuaternion(q);
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			localConstant.model.m[i][j] = mat.m[i][j];
		}
	}
	localConstant.modelInvTranspose = localConstant.model.Invert().Transpose();

}
void SceneComponent::UpdateMipState(int newForceMip0)
{
	localConstant.forceMip0 = newForceMip0;
}
void SceneComponent::UpdateUseReflect(int newUseReflect)
{
	localConstant.useReflect = newUseReflect;
}
