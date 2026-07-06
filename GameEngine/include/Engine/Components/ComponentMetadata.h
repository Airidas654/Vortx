#pragma once

#include "AssetID.h"

#include <string>

namespace Vortx
{

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
}