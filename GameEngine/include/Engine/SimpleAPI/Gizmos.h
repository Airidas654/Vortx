#pragma once

#include "EngineAPI.h"

#include <glm/glm.hpp>

namespace Vortx
{

	class ENGINE_API Gizmos
	{
	public:
		static glm::vec4 color;

		static void DrawCircle(const glm::vec2& position, const float& radius);
		static void DrawLine(const glm::vec2 &p1, const glm::vec2 &p2);
		static void DrawRay(const glm::vec2 &position, const glm::vec2 &direction);
	};

}