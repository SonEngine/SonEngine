#pragma once

#include <iostream>

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"

#include "Timer.h"


namespace Core {
    class BaseApp {

    public:
        BaseApp();
        BaseApp(int width, int height);
        virtual ~BaseApp();

        virtual bool Initialize();
        LRESULT MainProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
        int Run();

    private:
        virtual bool InitDirectX() = 0;
        virtual bool FinDirectX() = 0;
        virtual bool InitGUI() = 0;

        // Called when the window is resized
        virtual void OnResize() = 0;
        bool InitWindow();

        virtual void Update(float deltaTime) = 0;
        virtual void UpdateGUI(float deltaTime) = 0;

        virtual void Render(float deltaTime) = 0;
        virtual void RenderGUI(float deltaTime) = 0;
        virtual void Finalize(float deltaTime) = 0;

    public:
        static BaseApp* m_appPtr;
        Core::Timer m_timer;

    protected:
        int m_width;
        int m_height;
        HWND m_mainWnd;
    };
}