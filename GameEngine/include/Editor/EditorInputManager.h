#pragma once

#include <unordered_map>
#include <algorithm>
#include <utility>
#include <functional>

#include <glm/glm.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

typedef void(functionKey)(GLFWwindow* window, int, int, int, int, void*);
typedef void(functionMouse)(GLFWwindow* window, int, int, int, void*);

class EditorInputManager {

public:
	static const int MOUSE_INDEX_OFFSET = 1000;

	std::vector<std::pair<std::function<functionKey>, void*>> eventKey;
	std::vector<std::pair<std::function<functionMouse>, void*>> eventMouse;
	static EditorInputManager* instance;

	std::unordered_map<int, int> keys;

	EditorInputManager(GLFWwindow* window);
	~EditorInputManager();

	void RegisterEventKey(std::function<functionKey> func, void* priv_data);
	void RegisterEventMouse(std::function<functionMouse> func, void* priv_data);
};

