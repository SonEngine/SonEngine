#pragma once

#include "PrimitiveComponent.h"
#include "ActorData.h"
#include "PBRHLSLCompat.h"

class StaticMesh;

class LightComponent : public PrimitiveComponent
{
public:
	LightComponent(Actor* owner);
	virtual ~LightComponent();

public:
	void SetMesh(std::shared_ptr<StaticMesh> newMesh);
	void InitLightInfo(const LightData& ld);
	//void InitLightInfo(float viewWidth, float vewHeight, float nearZ, float farZ);
	//void SetLightInfo(const PBRLightInfo& lightInfo);

	virtual void UpdateConstantLocation() override;

public:
	StaticMesh* GetMesh() const { return m_mesh.get(); }
	std::shared_ptr<StaticMesh> GetMeshPtr() const { return m_mesh; }
	std::shared_ptr <PBRLightInfo> GetLightInfo() const { return m_lightInfo; }

private:
	bool bUpdateDir = false;
	std::shared_ptr<StaticMesh> m_mesh;
	std::shared_ptr <PBRLightInfo> m_lightInfo;
	float m_viewWidth;
	float m_viewHeight;
	float m_nearZ;
	float m_farZ;
};