#include "Actor.h"

using DirectX::SimpleMath::Vector3;

Actor::Actor()
{
}
void Actor::Initialize()
{
}
void Actor::Tick(const float& deltaTime)
{
}
void Actor::Render(ID3D12GraphicsCommandList* commandList, const TextureLoader* textureLoader)
{
	
}

void Actor::SetActorLocation(const DirectX::SimpleMath::Vector3& newLocation)
{
	if (m_rootComponent)
	{
		m_rootComponent->SetLocation(newLocation);
	}
}

void Actor::UpdateActorLocation(const DirectX::SimpleMath::Vector3& delLocation)
{
	if (m_rootComponent)
	{
		m_rootComponent->AddLocation(delLocation);
	}
}

void Actor::SetActorSpeed(const float& newSpeed)
{
	if (m_rootComponent)
	{
		m_rootComponent->SetSpeed(newSpeed);
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
