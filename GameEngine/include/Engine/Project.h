#pragma once

#include "EngineAPI.h"

#include <string>
#include <vector>

namespace Vortx
{
	class ENGINE_API Project
	{
	public:

		std::string path;
		std::string name;

		std::vector<std::string> scenes;

		static Project current;

		static void LoadProject(const std::string &path);
	};
}