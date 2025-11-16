#include "World.h"
#include "Renderer.h"
#include "StaticMesh.h"
#include "GeometryGenerater.h"
#include "Actor.h"
#include "Camera.h"

using namespace Graphics;

World::World()
{
	m_camera = std::make_shared<Camera>();
}

World::~World()
{
}

void World::Initialize(int cameraWidth, int cameraHeight)
{
	m_camera = std::make_shared<Camera>();
	InitCamera(cameraWidth, cameraHeight);	
	int planeSize = 6;
	m_player = utility->CreateActor(
		"player",
		GeometryGenerator::MakeCube(1.f, 1.f, 1.f),
		"pavement_03_albedo",
		{ -1.5f, 0.5f, -1.5f }
	);
	m_player->SetActorSpeed(1.f);

	std::shared_ptr<Actor> plane = utility->CreateActor(
		"plane",
		GeometryGenerator::MakePlane((float)planeSize, (float)planeSize, 1),
		"ComTex",
		{ 0.f,0.f,0.f }
	);
	m_actors.push_back(plane);

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

void World::InitCamera(int width, int height)
{
	if (m_camera == nullptr)
	{
		std::cout << "Camera가 할당되지 않았습니다\n";
		return;
	}

	m_camera->m_aspectRatio = width / (float)height;
	m_camera->m_width = 1280;
	m_camera->m_height = 720;
	m_camera->SetCameraMode(CameraMode::CM_Perspective);
	m_camera->Initialize();
	m_camera->SetActorLocation({ 0.f, 5.2f, 0.f });
	m_camera->UpdateCameraRotation(0, 178);
}

void World::UpdateCamera(int width, int height)
{
	if (m_camera == nullptr)
	{
		std::cout << "Camera가 할당되지 않았습니다\n";
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
void World::SetInputState(size_t key, bool isKeyDown)
{
	m_inputHelper.SetInputState(key, isKeyDown);
}
