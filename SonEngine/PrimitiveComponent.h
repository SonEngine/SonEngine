#pragma once
#include "SceneComponent.h"
#include "d3d12.h"
#include "TextureLoader.h"
#include "Delegate.h"
#include "physx\PxPhysicsAPI.h"
#include "PhysXMode.h"

DECLARE_DELEGATE_OneParam(ComponentBeginOverlapSignature, class PrimitiveComponent*);

class PrimitiveComponent : public SceneComponent {
public:
    PrimitiveComponent(Actor* owner);
    virtual ~PrimitiveComponent();

public:
    void SetVisible(bool visible) { m_visible = visible; }
    void SetPhysX(bool usePhysX) { m_usePhysX = usePhysX; }
    void SetPhysXMode(PhysXMode newMode) { m_physXMode = newMode; }

    bool IsVisible() const { return m_visible; }
    bool IsKinematic() const { return m_physXMode == PhysXMode::PM_Kinematic; }

public:
    ComponentBeginOverlapSignature OnComponentBeginOverlap;

public:
    class World* GetWorld() const;
    std::string GetName() const;
    PhysXMode GetPhysXMode() const { return m_physXMode; }
    physx::PxTransform GetPxTransform() const;

public:
    void OnRegister() override;

    // 물리 시뮬레이션 이후 transform 동기화
    void SyncFromPhysX(const physx::PxTransform& transform);

protected:
    bool m_visible;
    bool m_usePhysX = false;
    PhysXMode m_physXMode;
};