#pragma once

#include "EngineAPI.h"

#include "functional"

namespace Vortx
{
	class ENGINE_API Time
	{
	public:
		static float deltaTime;
		static float unscaledDeltaTime;
		static float fixedDeltaTime;
		static float unscaledFixedDeltaTime;

		static void _Update();
		static void _SetOnFixedUpdate(std::function<void()> func);
		static void _SetDeltaTime(const float deltaTime);

		void SetTimeScale(float timeScale);
		float GetTimeScale();

	private:
		static float timeScale;
		static float lastTime;
		static float fixedTimeAccum;
		static std::function<void()> fixedUpdateFunc;
	};
}