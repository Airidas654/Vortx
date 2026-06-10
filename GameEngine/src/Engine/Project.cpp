#include "Project.h"

#include "Core/File.h"

#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>

#include "json.hpp"

namespace Vortx
{
    Project Project::current;

	void Project::LoadProject(const std::string& path)
	{
		current.path = path;
		current.scenes.clear();

        nlohmann::json j;

        if (std::filesystem::exists(std::filesystem::path(path)))
        {
            try {
                std::ifstream file(path);

                file >> j;

                file.close();
            }
            catch (nlohmann::json::exception e) {
                std::cerr << e.what() << std::endl;

                return;
            }
        }

        current.name = j.value("Name", "");

        if (j.contains("Scenes") && j["Scenes"].is_array())
        {
            for (auto& i : j["Scenes"])
            {
                current.scenes.emplace_back(i);
            }
        }
	}
}