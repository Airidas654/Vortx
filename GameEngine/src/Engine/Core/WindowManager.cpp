#include "Core/WindowManager.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
//#include "stb_image.h"

#include <iostream>

namespace Vortx
{
    GLFWwindow* WindowManager::window = nullptr;
    WindowDesc WindowManager::currentDesc;
    bool WindowManager::minimized = false;
    bool WindowManager::focused = true;

    int WindowManager::windowedX = 0;
    int WindowManager::windowedY = 0;
    int WindowManager::windowedWidth = 1280;
    int WindowManager::windowedHeight = 720;

    std::vector<WindowResizeCallback> WindowManager::resizeCallbacks;
    std::vector<WindowFocusCallback> WindowManager::focusCallbacks;
    std::vector<WindowIconifyCallback> WindowManager::iconifyCallbacks;
    WindowCloseCallback WindowManager::closeCallback;

    static void GlfwErrorCallback(int error, const char* description)
    {
        if (error == GLFW_INVALID_VALUE && std::string(description).find("scancode") != std::string::npos)
            return;

        std::cerr << "GLFW error " << error << ": " << description << "\n";
    }

    bool WindowManager::Init()
    {
        glfwSetErrorCallback(GlfwErrorCallback);
        return glfwInit() == GLFW_TRUE;
    }

    void WindowManager::Shutdown()
    {
        Destroy();
        glfwTerminate();
    }

    GLFWmonitor* WindowManager::GetMonitor(int index)
    {
        int count = 0;
        GLFWmonitor** monitors = glfwGetMonitors(&count);
        if (count == 0) return nullptr;

        if (index < 0 || index >= count)
            return glfwGetPrimaryMonitor();

        return monitors[index];
    }

    GLFWwindow* WindowManager::Create(const WindowDesc& desc)
    {
        currentDesc = desc;

        glfwWindowHint(GLFW_RESIZABLE, desc.resizable ? GLFW_TRUE : GLFW_FALSE);
        glfwWindowHint(GLFW_DECORATED, desc.decorated ? GLFW_TRUE : GLFW_FALSE);

        GLFWmonitor* monitor = GetMonitor(desc.monitorIndex);

        if (desc.mode == WindowMode::Fullscreen && monitor)
        {
            const GLFWvidmode* mode = glfwGetVideoMode(monitor);
            glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
            window = glfwCreateWindow(mode->width, mode->height, desc.title.c_str(), monitor, nullptr);
        }
        else if (desc.mode == WindowMode::BorderlessFullscreen && monitor)
        {
            const GLFWvidmode* mode = glfwGetVideoMode(monitor);
            glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
            window = glfwCreateWindow(mode->width, mode->height, desc.title.c_str(), nullptr, nullptr);
            if (window)
            {
                int mx, my;
                glfwGetMonitorPos(monitor, &mx, &my);
                glfwSetWindowPos(window, mx, my);
            }
        }
        else
        {
            window = glfwCreateWindow((int)desc.width, (int)desc.height, desc.title.c_str(), nullptr, nullptr);
        }

        if (!window)
            return nullptr;

        if (desc.minWidth > 0 || desc.minHeight > 0)
        {
            glfwSetWindowSizeLimits(window,
                desc.minWidth > 0 ? (int)desc.minWidth : GLFW_DONT_CARE,
                desc.minHeight > 0 ? (int)desc.minHeight : GLFW_DONT_CARE,
                GLFW_DONT_CARE, GLFW_DONT_CARE);
        }

        glfwGetWindowPos(window, &windowedX, &windowedY);
        glfwGetWindowSize(window, &windowedWidth, &windowedHeight);

        glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
        glfwSetWindowFocusCallback(window, WindowFocusCallbackFn);
        glfwSetWindowIconifyCallback(window, WindowIconifyCallbackFn);
        glfwSetWindowCloseCallback(window, WindowCloseCallbackFn);

        return window;
    }

    void WindowManager::Destroy()
    {
        if (!window) return;
        glfwDestroyWindow(window);
        window = nullptr;
        resizeCallbacks.clear();
        focusCallbacks.clear();
        iconifyCallbacks.clear();
        closeCallback = nullptr;
    }

    void WindowManager::PollEvents() { glfwPollEvents(); }
    bool WindowManager::ShouldClose() { return window && glfwWindowShouldClose(window); }
    void WindowManager::RequestClose() { if (window) glfwSetWindowShouldClose(window, GLFW_TRUE); }
    void WindowManager::CancelClose() { if (window) glfwSetWindowShouldClose(window, GLFW_FALSE); }

    uint32_t WindowManager::GetWidth()
    {
        int w = 0, h = 0;
        if (window) glfwGetWindowSize(window, &w, &h);
        return (uint32_t)w;
    }

    uint32_t WindowManager::GetHeight()
    {
        int w = 0, h = 0;
        if (window) glfwGetWindowSize(window, &w, &h);
        return (uint32_t)h;
    }

    void WindowManager::SetSize(uint32_t width, uint32_t height)
    {
        if (window) glfwSetWindowSize(window, (int)width, (int)height);
    }

    uint32_t WindowManager::GetFramebufferWidth()
    {
        int w = 0, h = 0;
        if (window) glfwGetFramebufferSize(window, &w, &h);
        return (uint32_t)w;
    }

    uint32_t WindowManager::GetFramebufferHeight()
    {
        int w = 0, h = 0;
        if (window) glfwGetFramebufferSize(window, &w, &h);
        return (uint32_t)h;
    }

    void WindowManager::GetContentScale(float& outX, float& outY)
    {
        outX = 1.0f; outY = 1.0f;
        if (window) glfwGetWindowContentScale(window, &outX, &outY);
    }

    void WindowManager::GetPosition(int& outX, int& outY)
    {
        outX = 0; outY = 0;
        if (window) glfwGetWindowPos(window, &outX, &outY);
    }

    void WindowManager::SetPosition(int x, int y)
    {
        if (window) glfwSetWindowPos(window, x, y);
    }

    void WindowManager::Center(int monitorIndex)
    {
        if (!window) return;

        GLFWmonitor* monitor = GetMonitor(monitorIndex);
        if (!monitor) return;

        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        int mx, my;
        glfwGetMonitorPos(monitor, &mx, &my);

        int w, h;
        glfwGetWindowSize(window, &w, &h);

        glfwSetWindowPos(window, mx + (mode->width - w) / 2, my + (mode->height - h) / 2);
    }

    void WindowManager::SetTitle(const std::string& title)
    {
        currentDesc.title = title;
        if (window) glfwSetWindowTitle(window, title.c_str());
    }

    //void WindowManager::SetIcon(const std::string& pngPath)
    //{
        /*if (!window) return;

        if (pngPath.empty())
        {
            glfwSetWindowIcon(window, 0, nullptr);
            return;
        }

        int w, h, channels;
        unsigned char* pixels = stbi_load(pngPath.c_str(), &w, &h, &channels, 4);
        if (!pixels)
        {
            std::cerr << "Failed to load icon '" << pngPath << "'\n";
            return;
        }

        GLFWimage image;
        image.width = w;
        image.height = h;
        image.pixels = pixels;

        glfwSetWindowIcon(window, 1, &image);
        stbi_image_free(pixels);*/
    //}

    void WindowManager::SetFullscreen(bool fullscreen, int monitorIndex)
    {
        if (!window) return;

        if (fullscreen)
        {
            if (currentDesc.mode == WindowMode::Windowed)
            {
                glfwGetWindowPos(window, &windowedX, &windowedY);
                glfwGetWindowSize(window, &windowedWidth, &windowedHeight);
            }

            GLFWmonitor* monitor = GetMonitor(monitorIndex);
            if (!monitor) return;

            const GLFWvidmode* mode = glfwGetVideoMode(monitor);
            glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
            currentDesc.mode = WindowMode::Fullscreen;
        }
        else
        {
            glfwSetWindowMonitor(window, nullptr, windowedX, windowedY, windowedWidth, windowedHeight, 0);
            currentDesc.mode = WindowMode::Windowed;
        }
    }

    void WindowManager::ToggleFullscreen()
    {
        SetFullscreen(currentDesc.mode == WindowMode::Windowed);
    }

    bool WindowManager::IsFullscreen()
    {
        return currentDesc.mode == WindowMode::Fullscreen || currentDesc.mode == WindowMode::BorderlessFullscreen;
    }

    void WindowManager::SetVSync(bool enabled)
    {
        currentDesc.vsync = enabled;
        glfwSwapInterval(enabled ? 1 : 0);
    }

    void WindowManager::SetCursorMode(CursorMode mode)
    {
        if (!window) return;

        int glfwMode = GLFW_CURSOR_NORMAL;
        if (mode == CursorMode::Hidden) glfwMode = GLFW_CURSOR_HIDDEN;
        else if (mode == CursorMode::Disabled) glfwMode = GLFW_CURSOR_DISABLED;

        glfwSetInputMode(window, GLFW_CURSOR, glfwMode);
    }

    int WindowManager::GetMonitorCount()
    {
        int count = 0;
        glfwGetMonitors(&count);
        return count;
    }

    void WindowManager::GetMonitorResolution(int monitorIndex, uint32_t& outWidth, uint32_t& outHeight)
    {
        outWidth = 0; outHeight = 0;
        GLFWmonitor* monitor = GetMonitor(monitorIndex);
        if (!monitor) return;

        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        outWidth = (uint32_t)mode->width;
        outHeight = (uint32_t)mode->height;
    }

    void WindowManager::AddResizeCallback(WindowResizeCallback callback) { resizeCallbacks.push_back(std::move(callback)); }
    void WindowManager::AddFocusCallback(WindowFocusCallback callback) { focusCallbacks.push_back(std::move(callback)); }
    void WindowManager::AddIconifyCallback(WindowIconifyCallback callback) { iconifyCallbacks.push_back(std::move(callback)); }
    void WindowManager::SetCloseCallback(WindowCloseCallback callback) { closeCallback = std::move(callback); }

    void WindowManager::FramebufferSizeCallback(GLFWwindow* win, int width, int height)
    {
        minimized = (width == 0 || height == 0);
        for (auto& cb : resizeCallbacks)
            cb((uint32_t)width, (uint32_t)height);
    }

    void WindowManager::WindowFocusCallbackFn(GLFWwindow* win, int focusedFlag)
    {
        focused = (focusedFlag == GLFW_TRUE);
        for (auto& cb : focusCallbacks)
            cb(focused);
    }

    void WindowManager::WindowIconifyCallbackFn(GLFWwindow* win, int iconifiedFlag)
    {
        minimized = (iconifiedFlag == GLFW_TRUE);
        for (auto& cb : iconifyCallbacks)
            cb(minimized);
    }

    void WindowManager::WindowCloseCallbackFn(GLFWwindow* win)
    {
        if (closeCallback && !closeCallback())
            glfwSetWindowShouldClose(win, GLFW_FALSE);
    }
}