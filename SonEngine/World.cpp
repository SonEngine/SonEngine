#include "World.h"

#include "Renderer.h"
#include "StaticMesh.h"
#include "GeometryGenerator.h"

#include "Camera.h"
#include "Light.h"

#include "RenderEngine.h"
#include "PhysXEngine.h"

#include "ATriggerBox.h"
#include "AMovingPlatform.h"
#include "ModelLoader.h"

#include "ActorData.h"
#include "JsonHelper.h"
#include <filesystem>
#include <fstream>

using namespace Graphics;

World::World()
	: modelLoader(std::make_unique<ModelLoader<Vertex, std::uint16_t>>()),
	pbrModelLoader(std::make_unique<ModelLoader<PBRVertex, std::uint16_t>>())
{
	//m_camera = std::make_shared<Camera>();
	levelPath = "Levels/simpleLevel.json";
}

World::~World()
{
}

void World::Initialize(int cameraWidth, int cameraHeight, RenderEngine* renderEngine, ID3D12Device5* device, ID3D12GraphicsCommandList* commandList)
{
	m_renderEngine = renderEngine;
	//finalRenderTexName = "hdrTex";

	m_camera = std::make_shared<Camera>();
	InitCamera(cameraWidth, cameraHeight);

	// BOOKMARK lightSetting
	PBRLightInfo lInfo;
	lInfo.location = { -5,5,0 };
	Vector3 dir = -lInfo.location;
	dir.Normalize();
	lInfo.direction = dir;
	lInfo.brightness = { 0.8f,0.8f,0.8f,1.f };
	float fov = DirectX::XM_PIDIV2;
	DirectX::SimpleMath::Matrix lightProjMatrix = DirectX::XMMatrixPerspectiveFovLH(
		fov, 1.f, 0.1f, 100.f);

	DirectX::SimpleMath::Matrix viewMatrix = XMMatrixLookToLH(lInfo.location, lInfo.direction, Vector3(0, 1, 0));
	lInfo.proj = lightProjMatrix.Transpose();
	lInfo.view = viewMatrix.Transpose();

	m_lightInfos.push_back(lInfo);

	modelLoader->Load("torus.fbx");
	modelLoader->Initialize(device, commandList);

	pbrModelLoader->Load("sphere.glb");
	pbrModelLoader->Initialize(device, commandList);

	LoadLevel(levelPath);

	ModelLoader<PointCloudVertex, uint16_t> pcModelLoader;
	auto mat = DirectX::XMMatrixRotationZ(3.141592f) * 
		DirectX::XMMatrixRotationX(-3.14f / 12.f) *
		DirectX::XMMatrixTranslationFromVector(Vector3(0.f, 0.2f, 5.f));
	pcModelLoader.LoadPointCloud("map.ply", mat);
	
	std::shared_ptr<Actor> cubeMap = utility->CreateActor2<SimpleVertex, uint16_t, StaticMesh, CubeMapComponent>(
		"cubeMap",
		std::vector{ GeometryGenerator::MakeSimpleCube(100,100,100) },
		"CubeMap_SkyEnvHDR",
		{ 0.f,0.f,0.f },
		this
	);
	std::shared_ptr<Actor> pointCloud = utility->CreatePCActor(
		"pointCloud",
		pcModelLoader.GetAsset("map"),
		"",
		this
	);
	SpawnActor(pointCloud);
	
	std::shared_ptr<Actor> dot = utility->CreateActor2<SimpleVertex, uint16_t, StaticMesh, DotComponent>(
		"dot",
		std::vector{ GeometryGenerator::MakePoint() },
		"hdrTex",
		{ 0.f,0.f,0.f },
		this
	);
	SpawnActor(dot);

	SpawnActor(cubeMap);

}

void World::InitializePhysics(PhysXEngine* engine)
{
	m_physXEngine = engine;
}

void World::SpawnActor(const std::shared_ptr<Actor>& actor)
{
	m_actors[actor->GetName()] = actor;
	actor->OnRegister();
}

void World::InitCamera(int width, int height)
{
	if (m_camera == nullptr)
	{
		std::cout << "Camera가 할당되지 않았습니다\n";
		return;
	}
	m_camera->m_aspectRatio = width / (float)height;
	m_camera->m_width = width;
	m_camera->m_height = height;
	m_camera->SetCameraMode(CameraMode::CM_Perspective);
	m_camera->Initialize();
	/*m_camera->SetActorLocation({ 0.f, 0.f, -1.f });
	m_camera->UpdateCameraRotation(0, 0);*/
	// BOOKMARK	
	m_camera->SetActorLocation({ 0.f, 2.f, -1.5 });
	m_camera->UpdateCameraRotation(0, 80);

	m_camera->SetActorSpeed(5.f);
}

void World::UpdateCamera(int width, int height)
{
	if (m_camera == nullptr)
	{
		std::cout << "UpdateCamera() - Camera가 할당되지 않았습니다\n";
		return;
	}

	m_camera->m_width = width;
	m_camera->m_height = height;
	m_camera->m_aspectRatio = width / (float)height;
	m_camera->UpdateProjMatrix();
}

void World::PrintCameraInfo()
{
	if (m_camera == nullptr)
	{
		std::cout << "Camera가 할당되지 않았습니다\n";
		return;
	}

	auto loc = m_camera->GetActorLocation();
	float x = m_camera->GetXAngle();
	float y = m_camera->GetYAngle();
	std::cout << "\nCamera Location : x : " << loc.x << " y : " << loc.y <<
		" z : " << loc.z << "\nxAngle : " << x << ", yAngle : " << y << std::endl;
}

void World::UpdateMouse(int x, int y)
{
	mouseDeltaX = x;
	mouseDeltaY = y;
}
// BOOKMARK
void World::Tick(float deltaTime)
{
	// view 회전 업데이트
	if (isFocused && isFPSMode)
	{

		// update camera
		m_camera->UpdateCameraRotation(mouseDeltaX, mouseDeltaY);
		m_camera->UpdateCameraLocation(m_inputHelper.ExecuteCommands(deltaTime, m_camera.get()));

		mouseDeltaX = 0;
		mouseDeltaY = 0;

		//m_camera->UpdateActorLocation(m_inputHelper.ExecuteCommands(deltaTime, m_camera.get()));
	}
	else
	{
		auto p = m_actors.find("player");
		if (p != m_actors.end())
		{
			m_actors["player"]->UpdateActorLocation(m_inputHelper.ExecuteCommands(deltaTime, m_actors["player"].get()));
		}
	}

	for (auto& [name, A] : m_actors)
	{
		A->Tick(deltaTime);
	}
}

ViewProjInfo World::GetViewProjInfo()
{
	ViewProjInfo info;
	if (m_camera)
	{
		info = {
			m_camera->GetActorFrontDir(),
			m_camera->GetActorLocation(),
			m_camera->GetViewMatrix(),
			m_camera->GetProjMatrix()
		};
	}
	return info;
}

std::vector<PBRLightInfo> World::GetLightInfos() const
{
	return m_lightInfos;
}

void World::RegisterPrimitive(PrimitiveComponent* primitive, bool usePhysX)
{
	if (m_renderEngine)
	{
		m_renderEngine->RegisterPrimitive(primitive);
	}
	if (m_physXEngine)
	{
		m_physXEngine->RegisterPrimitive(primitive, usePhysX);
	}
}

void World::SyncKinematicToPhysX()
{
	if (m_physXEngine)
	{
		m_physXEngine->SyncKinematics();
	}
}

void World::SetInputState(size_t key, bool isKeyDown)
{
	m_inputHelper.SetInputState(key, isKeyDown);
}
// BOOKMARK
bool World::LoadLevel(const std::filesystem::path& levelPath)
{
	using json = nlohmann::json;
	std::ifstream ifs(levelPath, std::ios::binary);
	if (!ifs.is_open()) return false;

	json root;
	try { ifs >> root; }
	catch (const std::exception& e)
	{
		throw std::runtime_error("Invalid JSON: " + levelPath.string() + " (" + e.what() + ")");
	}

	if (root.contains("actors"))
	{
		for (const auto& a : root["actors"])
		{
			ActorData ad;
			std::string targetName;

			if (a.contains("name"))
				ad.name = a["name"].get<std::string>();
			if (a.contains("transform"))
			{
				auto t = a["transform"];
				if (t.contains("pos"))
				{
					ad.pos = ParseVec3(t["pos"]);
				}
			}
			if (a.contains("components"))
			{
				auto c = a["components"];
				for (const auto& comp : c)
				{
					std::string type = comp["type"].get<std::string>();
					if (type == "StaticMesh")
					{
						//std::cout << comp["mesh"].get<std::string>();
						ad.mesh = comp["mesh"].get<std::string>();
						ad.material = comp["material"].get<std::string>();
					}
					else if (type == "RigidBody")
					{
						ad.useSimulate = comp["simulate"].get<bool>();
						ad.mode = comp["mode"].get<PhysXMode>();
					}
					else if (type == "LocalConstant")
					{
						ad.forceMip0 = comp["forceMip0"].get<bool>();
						ad.updateConstants = comp["updateConstants"].get<bool>();
						ad.useReflect = comp["useReflect"].get<bool>();
						if(comp.contains("heightScale"))
							ad.heightScale = comp["heightScale"].get<float>();
						if (comp.contains("texTransform"))
						{
							auto& tt = comp["texTransform"];
							if (tt.contains("pos"))
							{
								Vector3 pos = ParseVec3(tt["pos"]);
								Vector3 scale = ParseVec3(tt["scale"]);
								Matrix texTransform = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);
								ad.lc.texTransform = texTransform;
							}
						}
					}
					else if (type == "RenderMode")
					{
						ad.psoName = comp["psoName"].get<std::string>();
					}
					else if (type == "trigger")
					{
						targetName = comp["target"].get<std::string>();
					}
				}
			}

			if (a.contains("class"))
			{
				const ActorType at = a["class"].get<ActorType>();
				switch (at)
				{
				case ActorType::AT_Actor:
				{
					std::shared_ptr<Actor> actor = std::make_shared<Actor>(ad.name, this);
					if(ad.psoName == "phongPSO")
						actor->Initialize(modelLoader->GetMeshes(ad.mesh), ad);
					else if (ad.psoName == "pbrPSO")
						actor->Initialize(pbrModelLoader->GetMeshes(ad.mesh), ad);

					SpawnActor(actor);
				}
				break;
				case ActorType::AT_ATriggerBox:
				{
					std::shared_ptr<ATriggerBox> actor = std::make_shared<ATriggerBox>(ad.name, this);
					actor->Initialize(modelLoader->GetMeshes(ad.mesh), ad);
					SpawnActor(actor);
					auto target = m_actors.find(targetName);
					if (target != m_actors.end())
						actor->SetTarget(target->second.get());
					else
					{
						std::cout << "target not founded\n";
					}
				}
				break;
				case ActorType::AT_AMovingPlatform:
				{
					std::shared_ptr<AMovingPlatform> actor = std::make_shared<AMovingPlatform>(ad.name, this);
					actor->Initialize(modelLoader->GetMeshes(ad.mesh), ad);

					SpawnActor(actor);
				}
				break;
				}
			}
		}
	}
	return true;
}