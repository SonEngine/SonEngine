#include "LightComponent.h"
#include "Actor.h"
#include "StaticMesh.h"

LightComponent::LightComponent(Actor* owner)
	:PrimitiveComponent(owner)
{
	m_mesh = std::make_shared<StaticMesh>();
	m_lightInfo = std::make_shared<PBRLightInfo>();
}

LightComponent::~LightComponent()
{
}


void LightComponent::SetMesh(std::shared_ptr<StaticMesh> newMesh)
{
	m_mesh = std::move(newMesh);
}

void LightComponent::InitLightInfo(const LightData& ld)
{
	m_viewWidth = ld.viewWidth;
	m_viewHeight = ld.viewHeight;
	m_nearZ = ld.nearZ;
	m_farZ = ld.farZ;
	m_lightInfo->brightness = ld.brightness;
	m_lightInfo->color = ld.color;
	m_lightInfo->intensity = ld.intensity;

	if (ld.dir == Vector3::Zero)
		bUpdateDir = true;
	else
	{
		bUpdateDir = false;
		m_lightInfo->direction = ld.dir;
	}
}

//void LightComponent::SetLightInfo(const PBRLightInfo& lightInfo)
//{
//	m_lightInfo = lightInfo;
//}

void LightComponent::UpdateConstantTransform()
{
	SceneComponent::UpdateConstantTransform();
	
	Vector3 newLocation = SceneComponent::GetLocation();
	m_lightInfo->location = newLocation;
	if (bUpdateDir)
	{
		Vector3 dir = -newLocation;
		dir.Normalize();
		m_lightInfo->direction = dir;
	}
	float fov = DirectX::XM_PIDIV2;
	DirectX::SimpleMath::Matrix lightProjMatrix = DirectX::XMMatrixOrthographicLH(
		m_viewWidth, m_viewHeight, m_nearZ, m_farZ);

	DirectX::SimpleMath::Matrix viewMatrix = XMMatrixLookToLH(m_lightInfo->location, m_lightInfo->direction, Vector3(0, 1, 0));
	m_lightInfo->proj = lightProjMatrix.Transpose();
	m_lightInfo->view = viewMatrix.Transpose();

}

