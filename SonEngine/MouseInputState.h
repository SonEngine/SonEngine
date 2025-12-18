#pragma once

struct MouseInputState
{
	bool lmbDown = false;
	int mouseX = 0;
	int mouseY = 0;
	int prevMouseX = 0;
	int prevMouseY = 0;
};

class MouseInputStateHelper
{
public:
	MouseInputStateHelper();
	~MouseInputStateHelper() {};

	void Update(bool IslmouseDown, int x, int y);
	void UpdateMousePos(int x, int y);
	void UpdatePrevMousePos(int x, int y);
	void Update(bool IslmouseDown);
	bool IsFirstFrame = false;

	MouseInputState GetInputState() { return state; }

private:
	MouseInputState state;

};