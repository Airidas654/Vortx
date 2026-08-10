#include "SimpleAPI/Gizmos.h"

#include <iostream>

namespace Vortx
{
	bool Gizmos::enabled = false;
	glm::u8vec4 Gizmos::color = glm::u8vec4(255, 255, 255, 255);
	std::vector<Gizmos::Command> Gizmos::commands = std::vector<Gizmos::Command>();

	void Gizmos::Clear()
	{
		commands.clear();
	}

	void Gizmos::DrawCircle(const glm::vec2& position, const float& radius, const float thickness)
	{
		if (!enabled) return;

		Command com;
		com.type = Gizmos::Shape::Circle;
		com.data[0] = position.x;
		com.data[1] = position.y;
		com.data[2] = radius;
		com.data[3] = thickness;

		com.color = color;

		commands.emplace_back(com);
	}

	void Gizmos::DrawBox(const glm::vec2& position, const glm::vec2& size, const float &rotation, const float thickness)
	{
		if (!enabled) return;

		Command com;
		com.type = Gizmos::Shape::Box;
		com.data[0] = position.x;
		com.data[1] = position.y;
		com.data[2] = size.x;
		com.data[3] = size.y;
		com.data[4] = thickness;
		com.data[5] = rotation;

		com.color = color;

		commands.emplace_back(com);
	}

	void Gizmos::DrawLine(const glm::vec2& p1, const glm::vec2& p2, const float thickness)
	{
		if (!enabled) return;

		Command com;
		com.type = Gizmos::Shape::Line;
		com.data[0] = p1.x;
		com.data[1] = p1.y;
		com.data[2] = p2.x;
		com.data[3] = p2.y;
		com.data[4] = thickness;

		com.color = color;

		commands.emplace_back(com);
	}

	void Gizmos::DrawRay(const glm::vec2& position, const glm::vec2& direction, const float thickness)
	{
		if (!enabled) return;

		DrawLine(position, position + direction, thickness);
	}

}