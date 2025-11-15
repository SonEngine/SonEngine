#pragma once
#include "SceneComponent.h"
#include "d3d12.h"
#include "TextureLoader.h"
#include "Delegate.h"

DECLARE_DELEGATE_OneParam(ComponentBeginOverlapSignature, class Actor*);

class PrimitiveComponent : public SceneComponent {
public:
    PrimitiveComponent(Actor* owner)
        : SceneComponent(owner), m_visible(true) {}

    virtual ~PrimitiveComponent();

    void SetVisible(bool visible) { m_visible = visible; }
    bool IsVisible() const { return m_visible; }

    ComponentBeginOverlapSignature OnComponentBeginOverlap;

protected:
    bool m_visible;
};