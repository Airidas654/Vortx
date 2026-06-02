#include "SimpleAPI/Time.h"

#include <algorithm>
#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Vortx
{
	float Time::deltaTime = 0.0f;
	float Time::lastTime = 0.0f;

	float Time::timeScale = 1.0f;

	void Time::Update()
	{
		Time::timeScale = std::max(Time::timeScale, 0.0f);

		float time = glfwGetTime();

		Time::deltaTime = (time - Time::lastTime) * Time::timeScale;

		Time::lastTime = time;
	}
}