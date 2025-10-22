#include "InputHelper.h"
#include <iostream>



using DirectX::SimpleMath::Vector3;

Input::InputHelper::InputHelper()
{
}

void Input::InputHelper::SetInputState(size_t wParam, bool bKeyDown)
{
	InputStates[wParam] = bKeyDown;
	//std::cout << (char)wParam << '\n';
}
bool Input::InputHelper::GetInputState(size_t wParam)
{
	auto it = InputStates.find(wParam);
	return (it != InputStates.end()) ? it->second : false;
}

Vector3 Input::InputHelper::ExecuteCommands(const float& deltaTime ,const Actor* actor)
{
	Vector3 delPosition;
	Vector3 eyeDirection = actor->GetActorFrontDir();
	Vector3 upDirection = actor->GetActorUpDir();
	Vector3 rightDirection = actor->GetActorRightDir();
	float speed = actor->GetActorSpeed();

	if (GetInputState(upKey))
	{
		delPosition += upDirection * speed * deltaTime;
	}
	if (GetInputState(downKey))
	{
		delPosition += -upDirection * speed * deltaTime;
	}
	if (GetInputState(forwardKey))
	{
		delPosition += eyeDirection * speed * deltaTime;
	}
	if (GetInputState(backwardKey))
	{
		delPosition += -eyeDirection * speed * deltaTime;
	}
	if (GetInputState(rightKey))
	{
		delPosition += rightDirection * speed * deltaTime;
	}
	if (GetInputState(leftKey))
	{
		delPosition += -rightDirection * speed * deltaTime;
	}
	return delPosition;
}