#pragma once

#include "Actor.h"

class Light : public Actor
{
public:
	Light();
	Light(std::string actorName, World* world);

	virtual ~Light();

	void SetBrightness(const DirectX::SimpleMath::Vector4& newBright) { brightness = newBright; }
	DirectX::SimpleMath::Vector4 GetBrightness() const { return brightness; }

private:
	DirectX::SimpleMath::Vector4 brightness;
};