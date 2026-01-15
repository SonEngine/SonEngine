#include "Actor.h"
#include "StaticMeshComponent.h"
#include "World.h"
#include "StaticMesh.h"

using DirectX::SimpleMath::Vector3;

Actor::Actor()
{
}

Actor::Actor(std::string actorName, World* world)
	:m_name(actorName),
	m_world(world)
{
}

void Actor::Initialize(std::shared_ptr<StaticMesh> mesh, const ActorData& ad)
{
	std::shared_ptr<StaticMeshComponent> cmp = std::make_shared<StaticMeshComponent>(this);
	cmp->SetMesh(mesh);
	cmp->SetActorData(ad);

	SetRootComponent(cmp);
	SetActorData(ad);
}

void Actor::Tick(const float& deltaTime)
{
}

void Actor::SetActorLocation(const DirectX::SimpleMath::Vector3& newLocation)
{
	if (m_rootComponent)
	{
		m_rootComponent->SetLocation(newLocation);
	}
}

void Actor::SetActorRotation(const DirectX::SimpleMath::Quaternion& newQuat)
{
	if (m_rootComponent)
	{
		m_rootComponent->SetRotation(newQuat);
	}
}

void Actor::UpdateActorLocation(const DirectX::SimpleMath::Vector3& delLocation)
{
	if (m_rootComponent)
	{
		m_rootComponent->AddLocation(delLocation);
	}
}

void Actor::UpdateActorRotation(const DirectX::SimpleMath::Quaternion& delQuat)
{
	if (m_rootComponent)
	{
		m_rootComponent->AddRotation(delQuat);
	}
}

void Actor::SetActorSpeed(const float& newSpeed)
{
	if (m_rootComponent)
	{
		m_rootComponent->SetSpeed(newSpeed);
	}
}

void Actor::SetRootComponent(std::shared_ptr<SceneComponent> newRootComponent)
{
	m_rootComponent = std::move(newRootComponent);
}

void Actor::OnRegister()
{
	if (m_rootComponent)
	{
		m_rootComponent->OnRegister();
	}
}

DirectX::SimpleMath::Vector3 Actor::GetActorLocation() const
{
	if (m_rootComponent)
	{
		return m_rootComponent->GetLocation();
	}
	else {
		return DirectX::SimpleMath::Vector3::Zero;
	}

}

DirectX::SimpleMath::Vector3 Actor::GetActorFrontDir() const
{
	if (m_rootComponent)
	{
		return m_rootComponent->GetFrontDirection();
	}
	else {
		return DirectX::SimpleMath::Vector3::Zero;
	}
}

DirectX::SimpleMath::Vector3 Actor::GetActorUpDir() const
{
	if (m_rootComponent)
	{
		return m_rootComponent->GetUpDirection();

	}
	else {
		return DirectX::SimpleMath::Vector3::Zero;
	}
}

DirectX::SimpleMath::Vector3 Actor::GetActorRightDir() const
{
	if (m_rootComponent)
	{
		return m_rootComponent->GetRightDirection();
	}
	else {
		return DirectX::SimpleMath::Vector3::Zero;
	}
}

float Actor::GetActorSpeed() const
{
	if (m_rootComponent)
	{
		return m_rootComponent->GetSpeed();
	}
	else {
		return 0.f;
	}
}

DirectX::SimpleMath::Matrix Actor::GetViewMatrix() const
{
	if (m_rootComponent)
	{
		return m_rootComponent->GetViewMatrix();
	}
	else {
		return DirectX::SimpleMath::Matrix();
	}
}

void Actor::UpdateMipState(int newForceMip0)
{
	if (m_rootComponent)
	{
		m_rootComponent->UpdateMipState(newForceMip0);
	}
}
void Actor::UpdateUseReflect(int newUseReflect)
{
	if (m_rootComponent)
	{
		m_rootComponent->UpdateUseReflect(newUseReflect);
	}
}


void Actor::SetUpdateConstant(bool newState)
{
	if (m_rootComponent)
	{
		if (PrimitiveComponent* comp = dynamic_cast<PrimitiveComponent*>(m_rootComponent.get()))
		{
			comp->SetUpdateConstant(newState);
		}
		
	}
}

void Actor::SetActorData(const ActorData& ad)
{
	SetActorLocation(ad.pos);
	UpdateMipState(ad.forceMip0);
	UpdateUseReflect(ad.useReflect);
}

void Actor::SetTextureName(const std::string& newName)
{
	if (m_rootComponent)
	{
		if (PrimitiveComponent* comp = dynamic_cast<PrimitiveComponent*>(m_rootComponent.get()))
		{
			comp->SetTextureName(newName);
		}
	}
}
void Actor::SetPSOName(const std::string& newName)
{
	if (m_rootComponent)
	{
		if (PrimitiveComponent* comp = dynamic_cast<PrimitiveComponent*>(m_rootComponent.get()))
		{
			comp->SetPSOName(newName);
		}
	}
}

void Actor::SetHeightScale(const float& heightScale)
{
	if (m_rootComponent)
	{
		if (PrimitiveComponent* comp = dynamic_cast<PrimitiveComponent*>(m_rootComponent.get()))
		{
			comp->SetHeightScale(heightScale);
		}
	}
}