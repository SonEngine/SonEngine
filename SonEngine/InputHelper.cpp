#include "InputHelper.h"
#include <iostream>

using DirectX::SimpleMath::Vector3;

Input::InputHelper::InputHelper()
{
	InputStates[grabKey] = false;
}

void Input::InputHelper::SetInputState(size_t wParam, bool newState)
{
	if (wParam == grabKey)
	{
		bool prevState = InputStates[wParam];
		if (prevState != newState)
		{
			grabDirty = true;
			InputStates[wParam] = newState;
		}
	}
	else
		InputStates[wParam] = newState;

}
bool Input::InputHelper::GetInputState(size_t wParam)
{
	auto it = InputStates.find(wParam);
	return (it != InputStates.end()) ? it->second : false;
}

Vector3 Input::InputHelper::ExecuteCommands(const float& deltaTime ,const Actor* actor)
{
	Vector3 delPosition;
	Vector3 upDirection = actor->GetActorUpDir();
	Vector3 rightDirection = actor->GetActorRightDir();
	Vector3 eyeDirection = rightDirection.Cross(upDirection);

	//float speed = actor->GetActorSpeed();

	/*if (GetInputState(upKey))
	{
		delPosition += upDirection * speed * deltaTime;
	}
	if (GetInputState(downKey))
	{
		delPosition += -upDirection * speed * deltaTime;
	}*/
	Vector3 dir = Vector3::Zero;
	if (GetInputState(forwardKey))
	{
		dir += eyeDirection;
		//delPosition += eyeDirection * speed * deltaTime;
	}
	if (GetInputState(backwardKey))
	{
		dir += -eyeDirection;
		//delPosition += -eyeDirection * speed * deltaTime;
	}
	if (GetInputState(rightKey))
	{
		dir += +rightDirection;
		//delPosition += rightDirection * speed * deltaTime;
	}
	if (GetInputState(leftKey))
	{
		dir += -rightDirection;
	}
	dir.Normalize();
	//delPosition += dir * speed * deltaTime;
	return dir;
}