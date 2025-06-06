#include "SimpleApp.h"

Core::SimpleApp::SimpleApp()
	:BaseApp()
{
}

Core::SimpleApp::SimpleApp(const int width, const int height)
	:BaseApp(width, height)
{
}

bool Core::SimpleApp::InitDirectX()
{
	return true;
}

bool Core::SimpleApp::InitGUI()
{
	return true;
}

void Core::SimpleApp::OnResize()
{
}

void Core::SimpleApp::Update(float deltaTime)
{
}

void Core::SimpleApp::UpdateGUI(float deltaTime)
{
}

void Core::SimpleApp::Render(float deltaTime)
{
}

void Core::SimpleApp::RenderGUI(float deltaTime)
{
}

bool Core::SimpleApp::FinDirectX()
{
	return true;
}

void Core::SimpleApp::Finalize(float deltaTime)
{
}
