#pragma once

#include <vector>
#include <memory>


#include "InputHelper.h"
#include "ViewProjInfo.h"

class Actor;
class Camera;

class World {

public:
	World();
	virtual ~World();

public:
	// World 내의 카메라 및 actor 초기화
	void Initialize(int cameraWidth, int cameraHeight);
	void InitCamera(int width, int height);

	void UpdateCamera(int width, int height);
	void UpdateMouse(int x, int y);

	void PrintCameraInfo();

public:
	void SetInputState(size_t key, bool isKeyDown);
	void SetFPSMode(bool newFpsMode) { isFPSMode = newFpsMode; }
	void SetFoucusMode(bool newFocusMode) { isFocused = newFocusMode; }

public:
	// 사용자 input에 따른 카메라 플레이어 이동 처리
	void Tick(float deltaTime);

public:
	ViewProjInfo GetViewProjInfo();

public:
	const std::vector<std::shared_ptr<Actor>>& GetActors() { return m_actors; }
	Actor* Getplayer() { return m_player.get(); }

// actors
protected:
	std::vector<std::shared_ptr<Actor>> m_actors;
	std::vector<std::shared_ptr<Actor>> m_textActors;
	std::vector<std::shared_ptr<Actor>> m_addActors;
	std::shared_ptr<Actor> m_player;
	std::shared_ptr<Camera> m_camera;

protected:
	Input::InputHelper m_inputHelper;

// mouse
private:
	int mouseDeltaX = 0;
	int mouseDeltaY = 0;

public:
	bool isFocused = false;
	bool isFPSMode = false;

public:
	std::atomic<bool> isRunning = true;
};