#include "SkinnedMeshComponent.h"
#include "Actor.h"
#include "GraphicsCommon.h"
#include "ModelLoader.h"
#include "StaticMesh.h"

SkinnedMeshComponent::SkinnedMeshComponent(Actor* owner)
	:PrimitiveComponent(owner)
{
	m_mesh = std::make_shared<StaticMesh>();
}

SkinnedMeshComponent::~SkinnedMeshComponent()
{
}

void SkinnedMeshComponent::SetMesh(std::shared_ptr<StaticMesh> newMesh)
{
	m_mesh = std::move(newMesh);
}

void SkinnedMeshComponent::UpdateAnimation(const float& deltaTime)
{
	if (bUpdateAnim)
	{
		if (Graphics::world)
		{
			m_currentFrame += m_animationSpeed * deltaTime;
			using namespace Graphics;
			m_skinnedLocalConstant = world->skinnedMeshLoader->GetCurrentSLC(m_currentFrame, m_animationName, 0, bUpdateRoot);
		}
	}

	for (const auto& c : m_children)
	{
		if (SkinnedMeshComponent* comp = dynamic_cast<SkinnedMeshComponent*>(c.get()))
		{
			comp->UpdateAnimation(deltaTime);
		}
	}
}

void SkinnedMeshComponent::SetAnimationData(const AnimData& animData)
{
	SetAnimationSpeed(animData.animationSpeed);
	SetAnimationName(animData.name);
	SetPlayAnimation(animData.playAnimation);
}
