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
	static int frame = 0;
	if (bUpdateAnim)
	{
		frame += 1;
		if (Graphics::world)
		{
			using namespace Graphics;
			m_skinnedLocalConstant = world->skinnedMeshLoader->GetCurrentSLC(frame, GetName(), 0);
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
