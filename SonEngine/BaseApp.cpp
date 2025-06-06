#include "BaseApp.h"

using namespace Core;

BaseApp* BaseApp::m_AppPtr = nullptr;

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	return BaseApp::m_AppPtr->MainProc(hWnd, msg, wParam, lParam);
}

BaseApp::BaseApp()
	:m_Width(1280),
	m_Height(720),
	m_MainWnd(NULL)
{
	if (m_AppPtr != nullptr) {
		delete m_AppPtr;
	}
	m_AppPtr = this;
	m_Timer = Timer();
}

BaseApp::BaseApp(int width, int height)
	:m_Width(width),
	m_Height(height),
	m_MainWnd(NULL)
{
	if (m_AppPtr != nullptr) {
		delete m_AppPtr;
	}
	m_AppPtr = this;
}

BaseApp::~BaseApp()
{
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	m_MainWnd = NULL;
	m_AppPtr = nullptr;
	std::cout << "Delete BaseApp\n";

}

bool BaseApp::Initialize()
{
	if (!InitWindow()) {
		std::cout << "InitWindow Failed\n";
		return false;
	}

	if (!InitDirectX()) {
		std::cout << "InitDirectX Failed\n";
		return false;
	}

	// Close commandList & Flush Queue
	OnResize();

	if (!InitGUI()) {
		std::cout << "InitGUI Failed\n";
		return false;
	}

	return true;
}

bool BaseApp::InitWindow()
{
	WNDCLASSEX wc;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = GetModuleHandle(NULL);
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = NULL;
	wc.lpszMenuName = nullptr;
	wc.lpszClassName = L"D3DApp";
	wc.hIconSm = NULL;

	RegisterClassEx(&wc);

	// Create window
	RECT rc = { 0, 0, (LONG)m_Width, (LONG)m_Height };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

	m_MainWnd = CreateWindow(wc.lpszClassName, wc.lpszClassName, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		(int)(rc.right - rc.left),
		(int)(rc.bottom - rc.top),
		NULL,
		NULL,
		wc.hInstance,
		nullptr);

	RAWINPUTDEVICE rawInputDevice;

	//The HID standard for mouse
	const uint16_t standardMouse = 0x02;

	rawInputDevice.usUsagePage = 0x01;
	rawInputDevice.usUsage = standardMouse;
	rawInputDevice.dwFlags = 0;
	rawInputDevice.hwndTarget = m_MainWnd;

	::RegisterRawInputDevices(&rawInputDevice, 1, sizeof(RAWINPUTDEVICE));


	ShowWindow(m_MainWnd, SW_SHOWDEFAULT);
	UpdateWindow(m_MainWnd);

	return true;
}

LRESULT BaseApp::MainProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	switch (msg) {
	case WM_DESTROY:
	{
		PostQuitMessage(0);
		return 0;
	}
	case WM_SIZE:
	{
		m_Width = LOWORD(lParam);
		m_Height = HIWORD(lParam);
		OnResize();

		return 0;
	}
	break;
	case WM_KEYUP:
	case WM_ACTIVATE:
	case WM_ACTIVATEAPP:
	case WM_INPUT:
	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MOUSEWHEEL:
	case WM_XBUTTONDOWN:
	case WM_XBUTTONUP:
	case WM_MOUSEACTIVATE:
		// When you click to activate the window, we want Mouse to ignore that event.
		return MA_ACTIVATEANDEAT;

	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

int BaseApp::Run()
{
	MSG msg = { };
	m_Timer.Reset();

	while (msg.message != WM_QUIT) {
		if (PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			m_Timer.Tick();
			float deltaTime = (float)m_Timer.GetDeltaTime();

			Update(deltaTime);
			Render(deltaTime);
			RenderGUI(deltaTime);
			Finalize(deltaTime);
		}
	}
	return (int)msg.wParam;
}
