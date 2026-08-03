#include "Project.h"

#include "Core/File.h"
#include "SimpleAPI/SceneManager.h"
#include "Core/AssetDatabase.h"

#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>

namespace Vortx
{
    Project Project::current;

	void Project::LoadProject(const std::string& path)
	{
		current.path = path;
		current.scenes.clear();
        current.scene_name_to_index.clear();

        if (std::filesystem::exists(std::filesystem::path(path)))
        {
            try {
                std::ifstream file(path);

                file >> current.json;

                file.close();
            }
            catch (nlohmann::json::exception e) {
                std::cerr << e.what() << std::endl;

                return;
            }
        }

        current.name = current.json.value("Name", "");

        if (current.json.contains("Scenes") && current.json["Scenes"].is_array())
        {
            uint32_t index = 0;
            for (auto& i : current.json["Scenes"])
            {
                std::filesystem::path path_scene = std::filesystem::path((std::string)i);
                current.scene_name_to_index.emplace(path_scene.filename().replace_extension().string(), index);
                current.scenes.emplace_back(i);
                index++;
            }
        }

        std::filesystem::path assetsRoot = std::filesystem::path(path).parent_path() / "Assets";
        AssetDatabase::Refresh(assetsRoot.string());

        std::string scene = (std::string)current.LoadEditorSettingValue("LastScene","");
        if (scene != "") {
            SceneManager::LoadScene(scene);
        }
        else {
            SceneManager::LoadScene(0);
        }
	}

    void Project::SaveJSON()
    {
        std::ofstream out(path);
        out << json.dump(4);
        out.close();
    }

    void Project::SaveSettingValue(const std::string& name, const nlohmann::json &value)
    {
        json["Settings"][name] = value;
    }

    void Project::SaveEditorSettingValue(const std::string& name, const nlohmann::json &value)
    {
        json["EditorTemp"]["Settings"][name] = value;
    }

    nlohmann::json Project::LoadEditorSettingValue(const std::string& name, const nlohmann::json& default)
    {
        nlohmann::json par = json["EditorTemp"]["Settings"];
        if (par.contains(name))
        {
            return par[name];
        }
        return default;
    }

    nlohmann::json Project::LoadSettingValue(const std::string& name, const nlohmann::json& default)
    {
        nlohmann::json par = json["Settings"];
        if (par.contains(name))
        {
            return par[name];
        }
        return default;
    }
}