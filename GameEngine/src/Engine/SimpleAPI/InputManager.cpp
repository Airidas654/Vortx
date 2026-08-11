#include "SimpleAPI/InputManager.h"

#include "Renderer/GameViewport.h"
#include "SimpleAPI/Camera.h"
#include "Core/WindowManager.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "json.hpp"

#include <fstream>
#include <iostream>
#include <cstring>

namespace Vortx
{
    GLFWwindow* InputManager::window = nullptr;

    bool InputManager::keyCurr[kMaxTrackedKeys] = {};
    bool InputManager::keyPrev[kMaxTrackedKeys] = {};

    bool InputManager::mouseCurr[kMaxMouseButtons] = {};
    bool InputManager::mousePrev[kMaxMouseButtons] = {};

    bool InputManager::gamepadCurr[kMaxGamepads][kMaxGamepadButtons] = {};
    bool InputManager::gamepadPrev[kMaxGamepads][kMaxGamepadButtons] = {};
    float InputManager::gamepadAxes[kMaxGamepads][6] = {};

    glm::vec2 InputManager::mousePos = glm::vec2(0.0f);
    glm::vec2 InputManager::lastMousePos = glm::vec2(0.0f);
    glm::vec2 InputManager::mouseDelta = glm::vec2(0.0f);

    glm::vec2 InputManager::scrollAccum = glm::vec2(0.0f);
    glm::vec2 InputManager::scrollDelta = glm::vec2(0.0f);

    bool InputManager::firstMouseUpdate = true;

    std::unordered_map<std::string, InputKey> InputManager::bindings;
    std::unordered_map<std::string, InputKey> InputManager::defaultBindings;

    void InputManager::Init(GLFWwindow* win)
    {
        window = win;

        GLFWscrollfun previous = glfwSetScrollCallback(window, ScrollCallback);
        (void)previous;

        if (window)
        {
            double x, y;
            glfwGetCursorPos(window, &x, &y);
            mousePos = { (float)x, (float)y };
            lastMousePos = mousePos;
        }

        firstMouseUpdate = true;
    }

    void InputManager::Shutdown()
    {
        if (window)
            glfwSetScrollCallback(window, nullptr);
        window = nullptr;
    }

    void InputManager::ScrollCallback(GLFWwindow*, double xoffset, double yoffset)
    {
        scrollAccum.x += (float)xoffset;
        scrollAccum.y += (float)yoffset;
    }

    void InputManager::Update()
    {
        if (!window) return;

        // ---- Keyboard: poll every trackable key (cheap, ~350 calls) ----
        std::memcpy(keyPrev, keyCurr, sizeof(keyCurr));
        for (int k = GLFW_KEY_SPACE; k < kMaxTrackedKeys; k++)
        {
            if (glfwGetKeyName(k, 0) == nullptr && k != GLFW_KEY_ESCAPE &&
                (k < GLFW_KEY_F1 || k > GLFW_KEY_F25) &&
                (k < GLFW_KEY_LEFT_SHIFT || k > GLFW_KEY_MENU))
            {
                continue;
            }
            
            keyCurr[k] = glfwGetKey(window, k) == GLFW_PRESS;
        }

        std::memcpy(mousePrev, mouseCurr, sizeof(mouseCurr));
        for (int b = 0; b < kMaxMouseButtons; b++)
            mouseCurr[b] = glfwGetMouseButton(window, b) == GLFW_PRESS;

        double x, y;
        glfwGetCursorPos(window, &x, &y);
        mousePos = { (float)x, (float)y };

        if (firstMouseUpdate)
        {
            lastMousePos = mousePos;
            firstMouseUpdate = false;
        }

        mouseDelta = mousePos - lastMousePos;
        lastMousePos = mousePos;

        scrollDelta = scrollAccum;
        scrollAccum = glm::vec2(0.0f);

        for (int id = 0; id < kMaxGamepads; id++)
        {
            std::memcpy(gamepadPrev[id], gamepadCurr[id], sizeof(gamepadCurr[id]));

            if (!glfwJoystickIsGamepad(id))
            {
                std::memset(gamepadCurr[id], 0, sizeof(gamepadCurr[id]));
                std::memset(gamepadAxes[id], 0, sizeof(gamepadAxes[id]));
                continue;
            }

            GLFWgamepadstate state;
            if (!glfwGetGamepadState(id, &state)) continue;

            for (int b = 0; b <= GLFW_GAMEPAD_BUTTON_LAST; b++)
                gamepadCurr[id][b] = state.buttons[b] == GLFW_PRESS;

            for (int a = 0; a <= GLFW_GAMEPAD_AXIS_LAST; a++)
                gamepadAxes[id][a] = state.axes[a];
        }
    }

    bool InputManager::GetKey(int glfwKey)
    {
        if (glfwKey < 0 || glfwKey >= kMaxTrackedKeys) return false;
        return keyCurr[glfwKey];
    }

    bool InputManager::GetKeyDown(int glfwKey)
    {
        if (glfwKey < 0 || glfwKey >= kMaxTrackedKeys) return false;
        return keyCurr[glfwKey] && !keyPrev[glfwKey];
    }

    bool InputManager::GetKeyUp(int glfwKey)
    {
        if (glfwKey < 0 || glfwKey >= kMaxTrackedKeys) return false;
        return !keyCurr[glfwKey] && keyPrev[glfwKey];
    }

    void InputManager::RegisterBinding(const std::string& name, InputKey defaultKey)
    {
        defaultKey.name = name;
        defaultBindings[name] = defaultKey;

        if (bindings.find(name) == bindings.end())
            bindings[name] = defaultKey;
    }

    bool InputManager::IsHeld(const InputKey& key, int gamepadId)
    {
        switch (key.device)
        {
        case InputDeviceType::Keyboard:
            return GetKey(key.code);
        case InputDeviceType::MouseButton:
            return GetMouseButton(key.code);
        case InputDeviceType::GamepadButton:
            return GetGamepadButton(key.code, gamepadId);
        case InputDeviceType::GamepadAxis:
        {
            float v = GetGamepadAxis(key.code, gamepadId);
            return key.axisPositive ? (v >= key.axisThreshold) : (v <= -key.axisThreshold);
        }
        }
        return false;
    }

    bool InputManager::WasHeldPrev(const InputKey& key, int gamepadId)
    {
        switch (key.device)
        {
        case InputDeviceType::Keyboard:
            return key.code >= 0 && key.code < kMaxTrackedKeys && keyPrev[key.code];
        case InputDeviceType::MouseButton:
            return key.code >= 0 && key.code < kMaxMouseButtons && mousePrev[key.code];
        case InputDeviceType::GamepadButton:
            return key.code >= 0 && key.code < kMaxGamepadButtons && gamepadPrev[gamepadId][key.code];
        case InputDeviceType::GamepadAxis:
            return false;
        }
        return false;
    }

    bool InputManager::GetPressed(const std::string& name)
    {
        auto it = bindings.find(name);
        if (it == bindings.end()) return false;
        return IsHeld(it->second, 0);
    }

    bool InputManager::GetPressedDown(const std::string& name)
    {
        auto it = bindings.find(name);
        if (it == bindings.end()) return false;
        return IsHeld(it->second, 0) && !WasHeldPrev(it->second, 0);
    }

    bool InputManager::GetPressedUp(const std::string& name)
    {
        auto it = bindings.find(name);
        if (it == bindings.end()) return false;
        return !IsHeld(it->second, 0) && WasHeldPrev(it->second, 0);
    }

    void InputManager::SetBinding(const std::string& name, const InputKey& key)
    {
        InputKey k = key;
        k.name = name;
        bindings[name] = k;
    }

    const InputKey* InputManager::GetBinding(const std::string& name)
    {
        auto it = bindings.find(name);
        return it != bindings.end() ? &it->second : nullptr;
    }

    const std::unordered_map<std::string, InputKey>& InputManager::GetAllBindings()
    {
        return bindings;
    }

    void InputManager::ResetBindingsToDefault()
    {
        bindings = defaultBindings;
    }

    static const char* DeviceTypeToString(InputDeviceType t)
    {
        switch (t)
        {
        case InputDeviceType::Keyboard: return "Keyboard";
        case InputDeviceType::MouseButton: return "MouseButton";
        case InputDeviceType::GamepadButton: return "GamepadButton";
        case InputDeviceType::GamepadAxis: return "GamepadAxis";
        }
        return "Keyboard";
    }

    static InputDeviceType DeviceTypeFromString(const std::string& s)
    {
        if (s == "MouseButton") return InputDeviceType::MouseButton;
        if (s == "GamepadButton") return InputDeviceType::GamepadButton;
        if (s == "GamepadAxis") return InputDeviceType::GamepadAxis;
        return InputDeviceType::Keyboard;
    }

    bool InputManager::SaveBindings(const std::string& path)
    {
        nlohmann::json j;
        j["Bindings"] = nlohmann::json::array();

        for (auto& [name, key] : bindings)
        {
            nlohmann::json entry;
            entry["Name"] = name;
            entry["Device"] = DeviceTypeToString(key.device);
            entry["Code"] = key.code;
            entry["AxisThreshold"] = key.axisThreshold;
            entry["AxisPositive"] = key.axisPositive;
            j["Bindings"].push_back(entry);
        }

        std::ofstream out(path);
        if (!out.is_open()) return false;
        out << j.dump(4);
        out.close();
        return true;
    }

    bool InputManager::LoadBindings(const std::string& path)
    {
        std::ifstream in(path);
        if (!in.is_open()) return false;

        nlohmann::json j;
        try
        {
            in >> j;
        }
        catch (nlohmann::json::exception& e)
        {
            std::cerr << "Failed to parse '" << path << "': " << e.what() << "\n";
            return false;
        }

        if (!j.contains("Bindings") || !j["Bindings"].is_array()) return false;

        for (auto& entry : j["Bindings"])
        {
            std::string name = entry.value("Name", "");
            if (name.empty()) continue;

            InputKey key;
            key.name = name;
            key.device = DeviceTypeFromString(entry.value("Device", "Keyboard"));
            key.code = entry.value("Code", -1);
            key.axisThreshold = entry.value("AxisThreshold", 0.5f);
            key.axisPositive = entry.value("AxisPositive", true);

            bindings[name] = key;
        }

        return true;
    }

    bool InputManager::AnyKeyPressed(InputKey& outKey, int gamepadId)
    {
        for (int k = GLFW_KEY_SPACE; k < kMaxTrackedKeys; k++)
        {
            if (keyCurr[k] && !keyPrev[k])
            {
                outKey = InputKey("", InputDeviceType::Keyboard, k);
                return true;
            }
        }

        for (int b = 0; b < kMaxMouseButtons; b++)
        {
            if (mouseCurr[b] && !mousePrev[b])
            {
                outKey = InputKey("", InputDeviceType::MouseButton, b);
                return true;
            }
        }

        if (glfwJoystickIsGamepad(gamepadId))
        {
            for (int b = 0; b < kMaxGamepadButtons; b++)
            {
                if (gamepadCurr[gamepadId][b] && !gamepadPrev[gamepadId][b])
                {
                    outKey = InputKey("", InputDeviceType::GamepadButton, b);
                    return true;
                }
            }

            for (int a = 0; a <= GLFW_GAMEPAD_AXIS_LAST; a++)
            {
                float v = gamepadAxes[gamepadId][a];
                if (std::fabs(v) >= 0.5f)
                {
                    outKey = InputKey("", InputDeviceType::GamepadAxis, a);
                    outKey.axisPositive = v > 0.0f;
                    return true;
                }
            }
        }

        return false;
    }

    bool InputManager::AnyKeyPressed()
    {
        InputKey dummy;
        return AnyKeyPressed(dummy);
    }

    glm::vec2 InputManager::GetMousePosition() { return mousePos; }
    glm::vec2 InputManager::GetMouseWorldPosition() 
    { 
        Camera cam = Camera::Main();

        if (cam == Camera()) return glm::vec2(0.0f);

        float size = cam.GetSize();

        uint32_t width = GameViewport::GetWidth();
        uint32_t height = GameViewport::GetHeight();

        int x, y;

        WindowManager::GetPosition(x,y);

        std::cout << x << "  " << y << "  " << mousePos.x << " " << mousePos.y << std::endl;

        return mousePos; 
    }
    glm::vec2 InputManager::GetMouseDelta() { return mouseDelta; }
    glm::vec2 InputManager::GetScrollDelta() { return scrollDelta; }

    bool InputManager::GetMouseButton(int b)
    {
        if (b < 0 || b >= kMaxMouseButtons) return false;
        return mouseCurr[b];
    }

    bool InputManager::GetMouseButtonDown(int b)
    {
        if (b < 0 || b >= kMaxMouseButtons) return false;
        return mouseCurr[b] && !mousePrev[b];
    }

    bool InputManager::GetMouseButtonUp(int b)
    {
        if (b < 0 || b >= kMaxMouseButtons) return false;
        return !mouseCurr[b] && mousePrev[b];
    }

    void InputManager::SetCursorMode(int mode)
    {
        if (window) glfwSetInputMode(window, GLFW_CURSOR, mode);
    }

    bool InputManager::IsGamepadConnected(int gamepadId)
    {
        return gamepadId >= 0 && gamepadId < kMaxGamepads && glfwJoystickIsGamepad(gamepadId);
    }

    std::string InputManager::GetGamepadName(int gamepadId)
    {
        if (!IsGamepadConnected(gamepadId)) return "";
        const char* name = glfwGetGamepadName(gamepadId);
        return name ? name : "";
    }

    bool InputManager::GetGamepadButton(int b, int gamepadId)
    {
        if (gamepadId < 0 || gamepadId >= kMaxGamepads) return false;
        if (b < 0 || b >= kMaxGamepadButtons) return false;
        return gamepadCurr[gamepadId][b];
    }

    bool InputManager::GetGamepadButtonDown(int b, int gamepadId)
    {
        if (gamepadId < 0 || gamepadId >= kMaxGamepads) return false;
        if (b < 0 || b >= kMaxGamepadButtons) return false;
        return gamepadCurr[gamepadId][b] && !gamepadPrev[gamepadId][b];
    }

    bool InputManager::GetGamepadButtonUp(int b, int gamepadId)
    {
        if (gamepadId < 0 || gamepadId >= kMaxGamepads) return false;
        if (b < 0 || b >= kMaxGamepadButtons) return false;
        return !gamepadCurr[gamepadId][b] && gamepadPrev[gamepadId][b];
    }

    float InputManager::GetGamepadAxis(int axis, int gamepadId)
    {
        if (gamepadId < 0 || gamepadId >= kMaxGamepads) return 0.0f;
        if (axis < 0 || axis > GLFW_GAMEPAD_AXIS_LAST) return 0.0f;
        return gamepadAxes[gamepadId][axis];
    }
}