#pragma once

#include "EngineAPI.h"

namespace Vortx
{
	class ENGINE_API Time
	{
	public:
		static float deltaTime;
		static float unscaledDeltaTime;
		static float fixedDeltaTime;
		static float unscaledFixedDeltaTime;
		static float timeScale;

		static void _Update();
		static void _SetDeltaTime(const float deltaTime);

	private:
		static float lastTime;

	};
}