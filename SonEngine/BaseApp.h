#include <wrl.h>
#include <iostream>

#include "dxgi1_6.h"
#include "imgui.h"
#include "imgui_impl_win32.h"

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
        static BaseApp* m_AppPtr;
        Core::Timer m_Timer;

    protected:
        int m_Width;
        int m_Height;
        HWND m_MainWnd;
    };
}