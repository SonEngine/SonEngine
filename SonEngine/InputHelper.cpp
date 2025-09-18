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

Vector3 Input::InputHelper::ExecuteCommands(const float& deltaTime , const Vector3& eyeDirection , const Vector3& upDirection, const Vector3& rightDirection)
{
	Vector3 delPosition;

	if (GetInputState(upKey))
	{
		delPosition += upDirection * cameraSpeed * deltaTime;
	}
	if (GetInputState(downKey))
	{
		delPosition += -upDirection * cameraSpeed * deltaTime;
	}
	if (GetInputState(forwardKey))
	{
		delPosition += eyeDirection * cameraSpeed * deltaTime;
	}
	if (GetInputState(backwardKey))
	{
		delPosition += -eyeDirection * cameraSpeed * deltaTime;
	}
	if (GetInputState(rightKey))
	{
		delPosition += rightDirection * cameraSpeed * deltaTime;
	}
	if (GetInputState(leftKey))
	{
		delPosition += -rightDirection * cameraSpeed * deltaTime;
	}
	return delPosition;
}