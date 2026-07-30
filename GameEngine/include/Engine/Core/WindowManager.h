#pragma once

#include "EngineAPI.h"

#include <cstdint>
#include <string>
#include <vector>
#include <functional>

struct GLFWwindow;
struct GLFWmonitor;

namespace Vortx
{
    enum class WindowMode
    {
        Windowed,
        Fullscreen,
        BorderlessFullscreen
    };

    struct WindowDesc
    {
        std::string title = "Vortx";
        uint32_t width = 1280;
        uint32_t height = 720;
        WindowMode mode = WindowMode::Windowed;
        bool resizable = true;
        bool vsync = true;
        bool decorated = true;
        int monitorIndex = -1;
        uint32_t minWidth = 0;
        uint32_t minHeight = 0;
    };

    enum class CursorMode { Normal, Hidden, Disabled };

    using WindowResizeCallback = std::function<void(uint32_t width, uint32_t height)>;
    using WindowFocusCallback = std::function<void(bool focused)>;
    using WindowIconifyCallback = std::function<void(bool iconified)>;
    using WindowCloseCallback = std::function<bool()>;

    class ENGINE_API WindowManager
    {
    public:
        static bool Init();
        static void Shutdown();

        static GLFWwindow* Create(const WindowDesc& desc);
        static void Destroy();

        static GLFWwindow* GetHandle() { return window; }
        static bool IsValid() { return window != nullptr; }

        static void PollEvents();
        static bool ShouldClose();
        static void RequestClose();
        static void CancelClose();

        static uint32_t GetWidth();
        static uint32_t GetHeight();
        static void SetSize(uint32_t width, uint32_t height);

        static uint32_t GetFramebufferWidth();
        static uint32_t GetFramebufferHeight();

        static void GetContentScale(float& outX, float& outY);

        static bool IsMinimized() { return minimized; }
        static bool IsFocused() { return focused; }

        static void GetPosition(int& outX, int& outY);
        static void SetPosition(int x, int y);
        static void Center(int monitorIndex = -1);

        static void SetTitle(const std::string& title);
        //static void SetIcon(const std::string& pngPath);

        static WindowMode GetMode() { return currentDesc.mode; }
        static void SetFullscreen(bool fullscreen, int monitorIndex = -1);
        static void ToggleFullscreen();
        static bool IsFullscreen();

        static void SetVSync(bool enabled);
        static bool GetVSync() { return currentDesc.vsync; }

        static void SetCursorMode(CursorMode mode);

        static int GetMonitorCount();
        static void GetMonitorResolution(int monitorIndex, uint32_t& outWidth, uint32_t& outHeight);

        static void AddResizeCallback(WindowResizeCallback callback);
        static void AddFocusCallback(WindowFocusCallback callback);
        static void AddIconifyCallback(WindowIconifyCallback callback);
        static void SetCloseCallback(WindowCloseCallback callback);

    private:
        static GLFWwindow* window;
        static WindowDesc currentDesc;
        static bool minimized;
        static bool focused;

        static int windowedX;
        static int windowedY;
        static int windowedWidth;
        static int windowedHeight;

        static std::vector<WindowResizeCallback> resizeCallbacks;
        static std::vector<WindowFocusCallback> focusCallbacks;
        static std::vector<WindowIconifyCallback> iconifyCallbacks;
        static WindowCloseCallback closeCallback;

        static GLFWmonitor* GetMonitor(int index);

        static void FramebufferSizeCallback(GLFWwindow* win, int width, int height);
        static void WindowFocusCallbackFn(GLFWwindow* win, int focusedFlag);
        static void WindowIconifyCallbackFn(GLFWwindow* win, int iconifiedFlag);
        static void WindowCloseCallbackFn(GLFWwindow* win);
    };
}