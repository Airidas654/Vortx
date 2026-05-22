#include <glm/glm.hpp>

#include <vector>

struct Transform
{
	glm::vec3 position;
	glm::vec rotation;
	glm::vec2 scale;

	AssetID parent;
	std::vector<AssetID> children;

	glm::mat4 matrix;

	Transform()
	{
		position = glm::vec3(0);
		rotation = glm::vec(0);
		scale = glm::vec2(1,1);

		parent = AssetID(0);
		matrix = glm::mat4();
	}
};