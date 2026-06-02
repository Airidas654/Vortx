#pragma once

#include <string>

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