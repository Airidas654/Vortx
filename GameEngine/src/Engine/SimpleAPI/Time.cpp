#include "SimpleAPI/Time.h"

#include <algorithm>
#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Vortx
{
	float Time::deltaTime = 0.0f;
	float Time::unscaledDeltaTime = 0.0f;

	float Time::fixedDeltaTime = 0.02f;
	float Time::unscaledFixedDeltaTime = 0.02f;

	float Time::timeScale = 1.0f;

	float Time::lastTime = 0.0f;	

	void Time::_SetDeltaTime(const float deltaTime)
	{
		timeScale = std::max(timeScale, 0.0f);

		float time = glfwGetTime();

		unscaledDeltaTime = deltaTime;

		Time::deltaTime = unscaledDeltaTime * timeScale;

		lastTime = time;
	}

	void Time::_Update()
	{
		timeScale = std::max(timeScale, 0.0f);

		float time = glfwGetTime();

		unscaledDeltaTime = (time - lastTime);

		deltaTime = unscaledDeltaTime * timeScale;

		lastTime = time;
	}
}