#pragma once

#include "AssetID.h"

#include <string>

struct MetadataData
{
	std::string name;
	std::string tag;
	AssetID id;

	MetadataData()
	{
		name = "GameObject";
		tag = "default";
	}
};