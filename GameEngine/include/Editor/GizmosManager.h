#pragma once

#include <glm/glm.hpp>
#include "imgui.h"

struct ImDrawList;
class EditorController;

class GizmosManager
{
public:
	GizmosManager();
	void UpdateParams(EditorController& editorController, const glm::vec2& screenOrigin, const glm::vec2& viewportSize);
	void Draw(ImDrawList* drawList);

private:
	glm::vec2 position{ 0.0f, 0.0f };
	float viewSize = 5.0f;
	glm::vec2 screenOrigin;
	glm::vec2 viewportSize;

	void DrawCircle(ImDrawList* drawList, const glm::vec2& position, const float& radius, const float& thickness, const glm::u8vec4& col);
	void DrawLine(ImDrawList* drawList, const glm::vec2& p1, const glm::vec2& p2, const float& thickness, const glm::u8vec4& col);

	glm::vec2 WorldToScreen(const glm::vec2& worldPos, const glm::vec2& viewportSize) const;
	float WorldToScreen(const float world, const glm::vec2& viewportSize) const;
};