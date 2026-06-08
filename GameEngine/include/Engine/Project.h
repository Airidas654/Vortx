#pragma once

#include "EngineAPI.h"

#include <string>
#include <vector>

namespace Vortx
{
	class ENGINE_API Project
	{
	public:

		const std::string path;
		const std::string name;

		std::vector<std::string> scenes;

		static Project* current;

		static Project LoadProject(const std::string &path);
	};
}