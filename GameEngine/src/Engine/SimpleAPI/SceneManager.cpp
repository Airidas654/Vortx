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

	void SceneManager::NewScene()
	{
		current.reset(new Scene());
	}

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

	void SceneManager::LoadScene(const std::string& path)
	{
		std::filesystem::path inPath(path);

		if (std::filesystem::exists(inPath))
		{
			std::ifstream in(inPath, std::ios::binary);

			std::vector<uint8_t> bytes(
				std::istreambuf_iterator<char>(in),
				{}
			);

			current.reset(new Scene());

			current->Deserialize(bytes);

			current->path = path;
		}
	}

	void SceneManager::SaveScene(const std::string& path)
	{
		if (current != nullptr)
		{
			std::ofstream out(path, std::ios::binary);

			std::vector<uint8_t> data = current->Serialize();

			out.write(reinterpret_cast<char*>(data.data()), data.size());

			out.close();
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