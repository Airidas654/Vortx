#include "EditorInputManager.h"

#include "imgui_impl_glfw.h"

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void MouseCallback(GLFWwindow* window, int button, int action, int mods);
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void CharCallback(GLFWwindow* window, unsigned int codepoint);

EditorInputManager* EditorInputManager::instance = nullptr;

EditorInputManager::EditorInputManager(GLFWwindow* window)
{
	if (instance == nullptr)
	{
		instance = this;
	}
	else
	{
		return;
	}

	glfwSetKeyCallback(window, KeyCallback);
	glfwSetMouseButtonCallback(window, MouseCallback);
	glfwSetScrollCallback(window, ScrollCallback);
	glfwSetCharCallback(window, CharCallback);
}

EditorInputManager::~EditorInputManager()
{
	if (instance == this) instance = nullptr;
}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (!EditorInputManager::instance) return;

	ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);

	if (action != 2)
	{
		EditorInputManager::instance->keys[key] = action;
	}

	for (auto i : EditorInputManager::instance->eventKey)
	{
		i.first(window, key, scancode, action, mods, i.second);
	}
}

void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
}

void CharCallback(GLFWwindow* window, unsigned int codepoint)
{
	ImGui_ImplGlfw_CharCallback(window, codepoint);
}

void MouseCallback(GLFWwindow* window, int button, int action, int mods)
{
	if (!EditorInputManager::instance) return;

	ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);

	if (action == GLFW_PRESS || action == GLFW_REPEAT)
	{
		ImGuiIO& io = ImGui::GetIO();
		if (io.WantCaptureMouse) return;
	}

	int button_final = button + EditorInputManager::MOUSE_INDEX_OFFSET;

	EditorInputManager::instance->keys[button_final] = action;

	for (auto i : EditorInputManager::instance->eventMouse)
	{
		i.first(window, button, action, mods, i.second);
	}
}

void EditorInputManager::RegisterEventKey(std::function<functionKey> func, void* priv_data)
{
	eventKey.emplace_back(func, priv_data);
}

void EditorInputManager::RegisterEventMouse(std::function<functionMouse> func, void* priv_data)
{
	eventMouse.emplace_back(func, priv_data);
}