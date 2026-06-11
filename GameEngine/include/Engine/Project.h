#pragma once

#include "EngineAPI.h"

#include <string>
#include <vector>
#include <unordered_map>

namespace Vortx
{
	class ENGINE_API Project
	{
	public:

		std::string path;
		std::string name;

		std::vector<std::string> scenes;

		std::unordered_map<std::string, uint32_t> scene_name_to_index;

		static Project current;

		static void LoadProject(const std::string &path);
	};
}