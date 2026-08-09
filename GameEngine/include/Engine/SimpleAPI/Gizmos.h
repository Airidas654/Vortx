#pragma once

#include "EngineAPI.h"

#include <glm/glm.hpp>

namespace Vortx
{

	class ENGINE_API Gizmos
	{

	public:

		enum Shape
		{
			Circle,
			Line,
			Triangle,
			Box
		};

		struct Command
		{
			Shape type;
			glm::u8vec4 color;
			float data[5];
		};

		static bool enabled;
		static std::vector<Command> commands;
		static glm::u8vec4 color;

		static void Clear();
		static void DrawCircle(const glm::vec2& position, const float& radius, const float thickness = 1);
		static void DrawBox(const glm::vec2& position, const glm::vec2& size, const float thickness = 1);
		static void DrawLine(const glm::vec2& p1, const glm::vec2& p2, const float thickness = 1);
		static void DrawRay(const glm::vec2 &position, const glm::vec2 &direction, const float thickness = 1);
	};

}