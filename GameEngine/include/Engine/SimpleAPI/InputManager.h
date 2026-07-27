#pragma once

#include "EngineAPI.h"

#include <glm/glm.hpp>

#include <string>
#include <unordered_map>
#include <vector>
#include <cstdint>

struct GLFWwindow;

namespace Vortx
{
    enum InputDeviceType : uint8_t
    {
        Keyboard,
        MouseButton,
        GamepadButton,
        GamepadAxis
    };

    struct ENGINE_API InputKey
    {
        std::string name;
        InputDeviceType device = InputDeviceType::Keyboard;
        int code = -1;

        float axisThreshold = 0.5f;
        bool axisPositive = true;

        InputKey() = default;
        InputKey(const std::string& name, InputDeviceType device, int code) : name(name), device(device), code(code) {}
    };

    class ENGINE_API InputManager
    {
    public:
        static void Init(GLFWwindow* window);
        static void Shutdown();

        static void Update();

        static bool GetKey(int glfwKey);
        static bool GetKeyDown(int glfwKey);
        static bool GetKeyUp(int glfwKey);


        static void RegisterBinding(const std::string& name, InputKey defaultKey);

        static bool GetPressed(const std::string& name);
        static bool GetPressedDown(const std::string& name);
        static bool GetPressedUp(const std::string& name);

        static void SetBinding(const std::string& name, const InputKey& key);
        static const InputKey* GetBinding(const std::string& name);
        static const std::unordered_map<std::string, InputKey>& GetAllBindings();
        static void ResetBindingsToDefault();

        static bool SaveBindings(const std::string& path);
        static bool LoadBindings(const std::string& path);


        static bool AnyKeyPressed(InputKey& outKey, int gamepadId = 0);
        static bool AnyKeyPressed();

        static glm::vec2 GetMousePosition();
        static glm::vec2 GetMouseDelta();
        static glm::vec2 GetScrollDelta();

        static bool GetMouseButton(int glfwMouseButton);
        static bool GetMouseButtonDown(int glfwMouseButton);
        static bool GetMouseButtonUp(int glfwMouseButton);

        static void SetCursorMode(int glfwCursorMode); // GLFW_CURSOR_NORMAL/HIDDEN/DISABLED


        static bool IsGamepadConnected(int gamepadId = 0);
        static std::string GetGamepadName(int gamepadId = 0);

        static bool GetGamepadButton(int glfwGamepadButton, int gamepadId = 0);
        static bool GetGamepadButtonDown(int glfwGamepadButton, int gamepadId = 0);
        static bool GetGamepadButtonUp(int glfwGamepadButton, int gamepadId = 0);

        static float GetGamepadAxis(int glfwGamepadAxis, int gamepadId = 0);

    private:
        static GLFWwindow* window;

        static constexpr int kMaxTrackedKeys = 349;     // GLFW_KEY_LAST + 1
        static constexpr int kMaxMouseButtons = 8;       // GLFW_MOUSE_BUTTON_LAST + 1
        static constexpr int kMaxGamepads = 16;          // GLFW_JOYSTICK_LAST + 1
        static constexpr int kMaxGamepadButtons = 15;    // GLFW_GAMEPAD_BUTTON_LAST + 1

        static bool keyCurr[kMaxTrackedKeys];
        static bool keyPrev[kMaxTrackedKeys];

        static bool mouseCurr[kMaxMouseButtons];
        static bool mousePrev[kMaxMouseButtons];

        static bool gamepadCurr[kMaxGamepads][kMaxGamepadButtons];
        static bool gamepadPrev[kMaxGamepads][kMaxGamepadButtons];
        static float gamepadAxes[kMaxGamepads][6];

        static glm::vec2 mousePos;
        static glm::vec2 lastMousePos;
        static glm::vec2 mouseDelta;

        static glm::vec2 scrollAccum;   
        static glm::vec2 scrollDelta;

        static bool firstMouseUpdate;

        static std::unordered_map<std::string, InputKey> bindings;
        static std::unordered_map<std::string, InputKey> defaultBindings;

        static bool IsHeld(const InputKey& key, int gamepadId);
        static bool WasHeldPrev(const InputKey& key, int gamepadId);

        static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    };
}