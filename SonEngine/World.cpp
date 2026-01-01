#include "World.h"

#include "Renderer.h"
#include "StaticMesh.h"
#include "GeometryGenerator.h"

#include "Actor.h"
#include "Camera.h"
#include "Light.h"

#include "RenderEngine.h"
#include "PhysXEngine.h"

#include "ATriggerBox.h"
#include "AMovingPlatform.h"
#include "ModelLoader.h"

using namespace Graphics;

World::World()
{
	//m_camera = std::make_shared<Camera>();
}

World::~World()
{
}

void World::Initialize(int cameraWidth, int cameraHeight, RenderEngine* renderEngine, ID3D12Device5* device, ID3D12GraphicsCommandList* commandList)
{
	m_renderEngine = renderEngine;

	m_camera = std::make_shared<Camera>();
	InitCamera(cameraWidth, cameraHeight);

	std::shared_ptr<Light> l = utility->CreateActor<Light, Vertex, uint16_t>(
		"plane",
		std::vector{ GeometryGenerator::MakeSphere(10,0.3f) },
		"ComTex",
		{ 0,5,-5 },
		this
	);

	l->SetBrightness({ 0.8f,0.8f,0.8f,1.f });

	LightInfo lInfo;
	lInfo.location = l->GetActorLocation();
	lInfo.brightness = l->GetBrightness();

	m_lightInfos.push_back(lInfo);


	ModelLoader<Vertex, uint16_t> modelLoader;

	modelLoader.Initialize();
	modelLoader.Load("torus.fbx");

	ModelLoader<PointCloudVertex, uint16_t> pcModelLoader;
	auto mat = DirectX::XMMatrixRotationZ(3.141592f) * DirectX::XMMatrixRotationX(-3.14f / 12.f) * DirectX::XMMatrixTranslationFromVector(Vector3(0.f,0.2f,5.f));
	pcModelLoader.LoadPointCloud("map.ply", mat);

	int planeSize = 100;
	m_player = utility->CreateActor(
		"player",
		modelLoader.GetMeshes("cube"),
		"pavement_03_albedo",
		{ -1.5f, 0.5f, -1.5f },
		this,
		true,
		PhysXMode::PM_Kinematic
	);

	m_player->SetActorSpeed(1.f);

	std::shared_ptr<Actor> plane = utility->CreateActor(
		"plane",
		std::vector{ GeometryGenerator::MakePlane((float)planeSize, (float)planeSize, 1) },
		//"ComTex",
		"8k_earth_albedo",
		{ 0.f,0.f,0.f },
		this,
		false,
		PM_Default,
		1
	);

	std::shared_ptr<Actor> pointCloud = utility->CreatePCActor(
		"pointCloud",
		pcModelLoader.GetMeshes("map"),
		"",
		this
	);

	std::shared_ptr<Actor> dot = utility->CreateActor2<SimpleVertex, uint16_t,StaticMesh, DotComponent>(
		"dot",
		std::vector{GeometryGenerator::MakePoint()},
		"ComTex",
		{ 0.f,0.f,0.f },
		this
	);
	std::shared_ptr<Actor> cubeMap = utility->CreateActor2<SimpleVertex, uint16_t, StaticMesh, CubeMapComponent>(
		"cubeMap",
		std::vector{ GeometryGenerator::MakeSimpleCube(100,100,100) },
		"CubeMap_SkyEnvHDR",
		{ 0.f,0.f,0.f },
		this
	);

	//std::shared_ptr<Actor> dynamicBox = utility->CreateActor(
	//	"dynamicBox",
	//	std::vector{ GeometryGenerator::MakeCube(1, 1, 1) },
	//	"pavement_03_albedo",
	//	{ 2.f,2.f,0.f },
	//	this,
	//	true,
	//	PM_Dynamic
	//);
	//dynamicBox->UpdateActorRotation(DirectX::SimpleMath::Quaternion::CreateFromAxisAngle(Vector3(1, 0, 0), DirectX::XM_PI / 4.f));

	//std::shared_ptr<Actor> torus = utility->CreateActor(
	//	"torus",
	//	modelLoader.GetMeshes("torus"),
	//	"pavement_03_albedo",
	//	{ 0.f,1.f,-1.f },
	//	this
	//);

	
	std::shared_ptr<ATriggerBox> triggerBox = std::make_shared<ATriggerBox>("triggerBox", this);

	triggerBox->Initialize(device, commandList, "pavement_03_albedo", DirectX::XMMatrixTranslation(0.f, 0.6f, 0.f));
	//triggerBox->UpdateActorRotation(DirectX::SimpleMath::Quaternion::CreateFromAxisAngle(Vector3(1, 0, 0), DirectX::XM_PI / 5.f));

	std::shared_ptr<AMovingPlatform> movingPlatform = std::make_shared<AMovingPlatform>("movingPlatform", this);
	movingPlatform->Initialize(device, commandList, "pavement_03_albedo", DirectX::XMMatrixTranslation(-2.f, 0.6f, 0.f));

	triggerBox->SetTarget(movingPlatform.get());

	SpawnActor(plane);
	SpawnActor(m_player);
	SpawnActor(l);
	SpawnActor(pointCloud);
	SpawnActor(dot);
	SpawnActor(cubeMap);

	//SpawnActor(torus);
	//SpawnActor(dynamicBox);
	SpawnActor(triggerBox);
	SpawnActor(movingPlatform);

	//int x = 3;
	//int z = 3;
	//float delX = (float)planeSize / x;
	//float delZ = -(float)planeSize / z;
	//Vector3 basePos = Vector3(-delX * (0.5f * (x - 1)), 0.1f, -delZ * (0.5f * (z - 1)));
	//bool breakFlag = false;
	//float margin = 0.2f;
	//float xSize = planeSize / (float)x - margin;
	//float zSize = planeSize / (float)z - margin;
	//for (int i = 0; i < z; i++)
	//{
	//	if (breakFlag)
	//		break;
	//	for (int j = 0; j < x; j++)
	//	{
	//		int idx = x * i + j;
	//		if (idx == textCount)
	//		{
	//			breakFlag = true;
	//			break;
	//		}
	//		Vector3 pos = basePos + Vector3(j * delX, 0.f, i * delZ);
	//		std::string name = "plane" + std::to_string(idx);
	//		std::shared_ptr<Actor> textPlane = utility->CreateActor(
	//			name,
	//			GeometryGenerator::MakePlane(xSize, zSize, 1),
	//			"8k_earth_albedo",
	//			pos
	//		);
	//		m_textActors.push_back(textPlane);
	//	}
	//}
}

void World::InitializePhysics(PhysXEngine* engine)
{
	m_physXEngine = engine;
}

void World::SpawnActor(const std::shared_ptr<Actor>& actor)
{
	m_actors.push_back(actor);
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
	m_camera->SetActorLocation({ 0.f, 5.f, -5.f });
	m_camera->UpdateCameraRotation(0, 70);
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

		m_camera->UpdateActorLocation(m_inputHelper.ExecuteCommands(deltaTime, m_camera.get()));
	}
	else
	{
		m_player->UpdateActorLocation(m_inputHelper.ExecuteCommands(deltaTime, m_player.get()));
	}

	for (auto& A : m_actors)
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

std::vector<LightInfo> World::GetLightInfos() const
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
