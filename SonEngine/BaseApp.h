#pragma once

#include <iostream>

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"

#include "Timer.h"
#include "InputHelper.h"

namespace Core {
    class BaseApp {

    public:
        BaseApp();
        BaseApp(int width, int height);
        virtual ~BaseApp();

        virtual bool Initialize();
        LRESULT MainProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
        virtual int Run() = 0;

    private:
        virtual bool InitDirectX() = 0;
        virtual bool FinDirectX() = 0;
        virtual bool InitGUI() = 0;

        // Called when the window is resized
        virtual void OnResize() = 0;
        bool InitWindow();

    protected:
        bool IsWindowFocused();

    public:
        static BaseApp* m_appPtr;
        Core::Timer m_timer;
        Input::InputHelper m_inputHelper;

    protected:
        int m_width;
        int m_height;
        HWND m_mainWnd;
        bool isFocused = true;
        bool isFPSMode = true;

    protected:
        bool bMouseFlag = false;
        int mouseDeltaX = 0;
        int mouseDeltaY = 0;
    };
}