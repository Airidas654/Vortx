#include "SimpleAPI/Gizmos.h"

#include <iostream>

namespace Vortx
{

	glm::vec4 Gizmos::color = glm::vec4(1);

	void Gizmos::DrawCircle(const glm::vec2& position, const float& radius)
	{

	}

	void Gizmos::DrawLine(const glm::vec2& p1, const glm::vec2& p2)
	{
		std::cout << "line" << std::endl;
	}

	void Gizmos::DrawRay(const glm::vec2& position, const glm::vec2& direction)
	{
		DrawLine(position, position + direction);
	}

}