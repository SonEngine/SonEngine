#include "PrimitiveComponent.h"
#include "Actor.h"
#include "World.h"
#include "PhysXEngine.h"

PrimitiveComponent::PrimitiveComponent(Actor* owner)
    : SceneComponent(owner), m_visible(true)
{

}

PrimitiveComponent::~PrimitiveComponent()
{
}

World* PrimitiveComponent::GetWorld() const
{
    return m_owner ? m_owner->GetWorld() : nullptr;
}
std::string PrimitiveComponent::GetName() const
{
	return m_owner ? m_owner->GetName() : "";
}

physx::PxTransform PrimitiveComponent::GetPxTransform() const
{
    // TODO : quat 변경
    DirectX::SimpleMath::Vector3 loc = GetLocation();
    return physx::PxTransform(
        physx::PxVec3(loc.x, loc.y, loc.z)   // 위치
    );
}

void PrimitiveComponent::OnRegister()
{
    SceneComponent::OnRegister();

    World* world = GetWorld();
    if (world)
    {
        world->RegisterPrimitive(this, m_usePhysX);
    }
}

void PrimitiveComponent::SyncFromPhysX(const physx::PxTransform& transform)
{
    DirectX::SimpleMath::Vector3 loc(transform.p.x, transform.p.y, transform.p.z);
    SetLocation(loc);

}
