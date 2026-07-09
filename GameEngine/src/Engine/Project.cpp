#include "Project.h"

#include "Core/File.h"
#include "SimpleAPI/SceneManager.h"
#include "Core/AssetDatabase.h"

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
        current.scene_name_to_index.clear();

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
            uint32_t index = 0;
            for (auto& i : j["Scenes"])
            {
                std::filesystem::path path_scene = std::filesystem::path((std::string)i);
                current.scene_name_to_index.emplace(path_scene.filename().replace_extension().string(), index);
                current.scenes.emplace_back(i);
                index++;
            }
        }

        std::filesystem::path assetsRoot = std::filesystem::path(path).parent_path() / "Assets";
        AssetDatabase::Refresh(assetsRoot.string());

        SceneManager::LoadScene(0);
	}
}