#pragma once

#include "PrimitiveComponent.h"

class ShapeComponent : public PrimitiveComponent {
public:
	ShapeComponent(Actor* owner):
		PrimitiveComponent(owner)
	{};
	virtual ~ShapeComponent() {};
};