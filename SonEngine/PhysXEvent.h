#pragma once

enum class PhysicsEventType
{
	PET_TriggerBegin,
	PET_TriggerEnd,
	PET_ContactBegin,
	PET_ContactEnd,

};

struct PhysicsEvent
{
	PhysicsEventType eventType;

	class PrimitiveComponent* triggerPrimitive;
	class PrimitiveComponent* otherPrimitive;

	class Actor* triggerActor;
	class Actor* otherActor;
};