#pragma once

#include "AssetID.h"
#include "SimpleAPI/GameObject.h"

#include <glm/glm.hpp>

#include "entt.hpp"

#include <vector>
#include <optional>

struct TransformData
{
	glm::vec3 position;
	glm::vec1 rotation;
	glm::vec2 scale;

	std::optional<entt::entity> parent;
	std::vector<entt::entity> children;

	glm::mat4 matrix;

	TransformData()
	{
		position = glm::vec3(0);
		rotation = glm::vec1(0);
		scale = glm::vec2(1,1);

		parent = std::nullopt;
		matrix = glm::mat4(1.0f);
	}
};