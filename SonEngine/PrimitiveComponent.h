#pragma once
#include "SceneComponent.h"
#include "d3d12.h"
#include "TextureLoader.h"

class PrimitiveComponent : public SceneComponent {
public:
    PrimitiveComponent(Actor* owner)
        : SceneComponent(owner), m_visible(true) {}

    void SetVisible(bool visible) { m_visible = visible; }
    bool IsVisible() const { return m_visible; }

protected:
    bool m_visible;
};