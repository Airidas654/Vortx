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
	float Time::fixedTimeAccum = 0.0f;

	std::function<void()> Time::fixedUpdateFunc;

	void Time::_SetDeltaTime(const float deltaTime)
	{
		timeScale = std::max(timeScale, 0.0f);

		float time = glfwGetTime();

		unscaledDeltaTime = deltaTime;

		Time::deltaTime = unscaledDeltaTime * timeScale;

		lastTime = time;
	}

	void Time::_SetOnFixedUpdate(std::function<void()> func)
	{
		fixedUpdateFunc = move(func);
	}

	void Time::SetTimeScale(float timeScale)
	{
		if (Time::timeScale != 0) 
		{
			fixedTimeAccum /= Time::timeScale;
		}
		fixedTimeAccum *= timeScale;

		Time::timeScale = timeScale;
	}

	float Time::GetTimeScale()
	{
		return timeScale;
	}

	void Time::_Update()
	{
		timeScale = std::max(timeScale, 0.0f);

		float time = glfwGetTime();

		unscaledDeltaTime = (time - lastTime);

		deltaTime = unscaledDeltaTime * timeScale;

		fixedDeltaTime = timeScale * unscaledFixedDeltaTime;

		fixedTimeAccum += deltaTime;

		int fixedUpdateCnt = std::min(2, (int)(fixedTimeAccum / fixedDeltaTime));

		fixedTimeAccum -= fixedDeltaTime * fixedUpdateCnt;

		for (int i = 0; i < fixedUpdateCnt; i++)
		{
			if (fixedUpdateFunc)
			{
				fixedUpdateFunc();
			}
		}

		lastTime = time;
	}
}