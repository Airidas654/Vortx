#include "SimpleAPI/SceneManager.h"

#include "Project.h"

#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <tuple>

namespace Vortx
{
	std::unique_ptr<Scene> SceneManager::current = nullptr;

	void SceneManager::LoadScene(const uint32_t index)
	{
		if (Project::current.scenes.size() >= index) return;

		std::string path = Project::current.scenes[index];

		std::filesystem::path pathObj = std::filesystem::path(path);

		if (std::filesystem::exists(pathObj) && pathObj.extension().string() == ".scene")
		{
			current.reset(new Scene());
		}
	}

	void SceneManager::LoadScene(const std::string& name)
	{
		uint32_t index = GetSceneIndex(name);

		if (index != -1)
		{
			LoadScene(index);
		}
	}

	uint32_t SceneManager::GetSceneIndex(const std::string& name)
	{
		auto it = Project::current.scene_name_to_index.find(name);

		if (it != Project::current.scene_name_to_index.end())
		{
			return std::get<1>(*it);
		}

		return -1;
	}
}