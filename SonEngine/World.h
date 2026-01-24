#pragma once

#include <vector>
#include <memory>

#include "InputHelper.h"
#include "ViewProjInfo.h"

#include "d3d12.h"
#include "PBRHLSLCompat.h"
#include "Vertex.h"

class Actor;
class Camera;
class Light;
template<class V, class I> class ModelLoader;

class World {

public:
	World();
	virtual ~World();

public:
	// World 내의 카메라 및 actor 초기화
	void Initialize(int cameraWidth, int cameraHeight, class RenderEngine* renderEngine, ID3D12Device5* device, ID3D12GraphicsCommandList* commandList);
	void InitializePhysics(class PhysXEngine* renderEngine);

	void SpawnActor(std::shared_ptr<Actor> actor);

	//void SpawnActor(const std::shared_ptr<Actor>& actor);
	void InitCamera(int width, int height);

	void UpdateCamera(int width, int height);
	void UpdateMouse(int x, int y);

	void PrintCameraInfo();

	int m_cameraWidth;
	int m_cameraHeight;


public:
	void SetInputState(size_t key, bool isKeyDown);
	bool LoadLevel(const std::filesystem::path & levelPath);
	void SetFPSMode(bool newFpsMode) { isFPSMode = newFpsMode; }
	void SetFoucusMode(bool newFocusMode) { isFocused = newFocusMode; }

public:
	// 사용자 input에 따른 카메라 & 플레이어 이동 처리
	void Tick(float deltaTime);

public:
	ViewProjInfo GetViewProjInfo();
	//std::vector<PBRLightInfo> GetLightInfos() const;

public:
	//Actor* Getplayer() { return m_player.get(); }
	class PhysXEngine* GetPhysXEngine() const { return m_physXEngine; }

// actors
protected:
	//std::vector<std::shared_ptr<Actor>> m_actors;
	std::map<std::string, std::shared_ptr<Actor>> m_actors;
	std::vector<std::shared_ptr<Actor>> m_textActors;
	std::vector<std::shared_ptr<Actor>> m_addActors;
	
	std::shared_ptr<Actor> m_player;
	std::shared_ptr<Camera> m_camera;
	
public:
	void RegisterPrimitive(class PrimitiveComponent* primitive, bool usePhysX);

	// Tick 함수에서 이동시킨 플레이어와 같은
	// kinematic actor들 물리엔진에 동기화
	void SyncKinematicToPhysX();

public:
	Input::InputHelper m_inputHelper;

public:
	bool GetInputState(size_t key);
	bool GetGrabState();
	bool GetGrabDirty();
	void SetGrabDirty(bool newDity);

	bool GetInteractState();
	bool GetInteractDirty();

	void SetInteractDirty(bool newDity);


// mouse
private:
	int mouseDeltaX = 0;
	int mouseDeltaY = 0;

public:
	bool isFocused = true;
	bool isFPSMode = true;

public:
	std::atomic<bool> isRunning = true;

private:
	class RenderEngine* m_renderEngine;
	class PhysXEngine* m_physXEngine;

public:
	std::unique_ptr<ModelLoader<Vertex, uint16_t>> modelLoader;
	std::unique_ptr<ModelLoader<PositionVertex, uint16_t>> dotModelLoader;
	std::unique_ptr<ModelLoader<PBRVertex, uint16_t>> pbrModelLoader;
	std::unique_ptr<ModelLoader<SkinnedVertex, uint32_t>> skinnedMeshLoader;
	std::unique_ptr<ModelLoader<PointCloudVertex, uint16_t>> pcModelLoader;
	std::unique_ptr<ModelLoader<SimpleVertex, uint16_t>> simpleModelLoader;
	std::string levelPath;
	std::string finalRenderTexName;
};