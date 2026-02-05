#include "MouseInputState.h"

MouseInputStateHelper::MouseInputStateHelper()
{
}

void MouseInputStateHelper::Update(bool IslmouseDown, int x, int y)
{
	state.lmbDown = IslmouseDown;
	state.mouseX = x;
	state.mouseY = y;
}

void MouseInputStateHelper::UpdateMousePos(int x, int y)
{
	state.mouseX = x;
	state.mouseY = y;
}

void MouseInputStateHelper::UpdatePrevMousePos(int x, int y)
{
	state.prevMouseX = x;
	state.prevMouseY = y;
}

void MouseInputStateHelper::Update(bool IslmouseDown)
{
	state.lmbDown = IslmouseDown;
	if (IslmouseDown)
		IsFirstFrame = true;
}

void MouseInputStateHelper::UpdateLMouseState(bool IslmouseDown)
{
	if (state.lmbDown != IslmouseDown)
	{
		state.lmbDown = IslmouseDown;
		lMouseDirty = true;
	}
	if (IslmouseDown)
		IsFirstFrame = true;
}
