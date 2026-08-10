#include "GizmosManager.h"

#include "SimpleAPI/Gizmos.h"
#include "EditorController.h"
#include "SimpleAPI/Math.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>

GizmosManager::GizmosManager()
{
	Vortx::Gizmos::enabled = true;
}

void GizmosManager::UpdateParams(EditorController& editorController, const glm::vec2& screenOrigin, const glm::vec2& viewportSize)
{
	position = editorController.GetPosition();
	viewSize = editorController.GetViewSize();
	this->screenOrigin = screenOrigin;
	this->viewportSize = viewportSize;
}

void GizmosManager::Draw(ImDrawList* drawList)
{
	for (auto& data : Vortx::Gizmos::commands)
	{
		switch (data.type)
		{
		case Vortx::Gizmos::Shape::Circle:
		{
			DrawCircle(drawList, glm::vec2(data.data[0], data.data[1]), data.data[2], data.data[3], data.color);

			break;
		}
		case Vortx::Gizmos::Shape::Line:
		{
			DrawLine(
				drawList,
				glm::vec2(data.data[0], data.data[1]),
				glm::vec2(data.data[2], data.data[3]),
				data.data[4],
				data.color
			);
			break;
		}
		case Vortx::Gizmos::Shape::Box:
		{
			DrawBox(
				drawList,
				glm::vec2(data.data[0], data.data[1]),
				glm::vec2(data.data[2], data.data[3]),
				data.data[5],
				data.data[4],
				data.color
			);
		}
		default:
			break;
		}
	}

	Vortx::Gizmos::Clear();
}

void GizmosManager::DrawCircle(ImDrawList* drawList, const glm::vec2& position, const float& radius, const float& thickness, const glm::u8vec4& col)
{
	ImU32 color = IM_COL32(col.r, col.g, col.b, col.a);

	glm::vec2 screenLocal = WorldToScreen(position, viewportSize);
	ImVec2 center = { screenOrigin.x + screenLocal.x, screenOrigin.y + screenLocal.y };

	float radiusScreen = WorldToScreen(radius, viewportSize);

	drawList->AddCircle(center, radiusScreen, color, 32, thickness);
}

void GizmosManager::DrawBox(ImDrawList* drawList, const glm::vec2& position, const glm::vec2& size, const float& rotation, const float& thickness, const glm::u8vec4& col)
{
	ImU32 color = IM_COL32(col.r, col.g, col.b, col.a);

	glm::vec2 p1 = size/2.0f;
	glm::vec2 p2 = glm::vec2(-size.x, size.y)/2.0f;
	glm::vec2 p3 = size/2.0f;
	glm::vec2 p4 = glm::vec2(size.x, -size.y) / 2.0f;

	p1 = position - Vortx::Math::Rotate(p1, rotation);
	p2 = position + Vortx::Math::Rotate(p2, rotation);
	p3 = position + Vortx::Math::Rotate(p3, rotation);
	p4 = position + Vortx::Math::Rotate(p4, rotation);

	glm::vec2 p1Local = WorldToScreen(p1, viewportSize);
	ImVec2 p1Screen = { screenOrigin.x + p1Local.x, screenOrigin.y + p1Local.y };

	glm::vec2 p2Local = WorldToScreen(p2, viewportSize);
	ImVec2 p2Screen = { screenOrigin.x + p2Local.x, screenOrigin.y + p2Local.y };

	glm::vec2 p3Local = WorldToScreen(p3, viewportSize);
	ImVec2 p3Screen = { screenOrigin.x + p3Local.x, screenOrigin.y + p3Local.y };

	glm::vec2 p4Local = WorldToScreen(p4, viewportSize);
	ImVec2 p4Screen = { screenOrigin.x + p4Local.x, screenOrigin.y + p4Local.y };

	drawList->AddQuad(p1Screen, p2Screen, p3Screen, p4Screen, color, thickness);
}

void GizmosManager::DrawLine(ImDrawList* drawList, const glm::vec2& p1, const glm::vec2& p2, const float& thickness, const glm::u8vec4& col)
{
	ImU32 color = IM_COL32(col.r, col.g, col.b, col.a);

	glm::vec2 p1Local = WorldToScreen(p1, viewportSize);
	ImVec2 p1Screen = { screenOrigin.x + p1Local.x, screenOrigin.y + p1Local.y };

	glm::vec2 p2Local = WorldToScreen(p2, viewportSize);
	ImVec2 p2Screen = { screenOrigin.x + p2Local.x, screenOrigin.y + p2Local.y };

	drawList->AddLine(p1Screen, p2Screen, color, thickness);
}

glm::vec2 GizmosManager::WorldToScreen(const glm::vec2& worldPos, const glm::vec2& viewportSize) const
{
	float aspect = (viewportSize.y > 0.0f) ? viewportSize.x / viewportSize.y : 1.0f;
	float xSize = aspect * viewSize;

	glm::vec2 ndc;
	ndc.x = (xSize > 0.0f) ? (worldPos.x - position.x) / xSize : 0.0f;
	ndc.y = (viewSize > 0.0f) ? (worldPos.y - position.y) / viewSize : 0.0f;

	glm::vec2 local;
	local.x = (ndc.x + 1.0f) * 0.5f * viewportSize.x;
	local.y = (1.0f - ndc.y) * 0.5f * viewportSize.y;
	return local;
}

float GizmosManager::WorldToScreen(const float world, const glm::vec2& viewportSize) const
{
	return viewportSize.y / 2.0f / viewSize * world;
}