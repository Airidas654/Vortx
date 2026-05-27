#pragma once

#include <glm/glm.hpp>

struct MetadataData
{
	std::string name;
	std::string tag;

	MetadataData()
	{
		name = "GameObject";
		tag = "default";
	}
};