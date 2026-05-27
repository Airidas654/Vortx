#pragma once

#include "AssetID.h"
#include "SimpleAPI/GameObject.h"
#include "SimpleAPI/Transform.h"

#pragma once

#include <glm/glm.hpp>

#include <vector>
#include <optional>

struct TransformData
{
	glm::vec2 position;
	glm::vec1 rotation;
	glm::vec2 scale;

	std::optional<Transform> parent;
	std::vector<Transform> children;

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