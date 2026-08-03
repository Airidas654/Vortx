#pragma once

#include "EngineAPI.h"

#include <string>
#include <vector>
#include <unordered_map>

#include "json.hpp"

namespace Vortx
{
	class ENGINE_API Project
	{
	private:
		nlohmann::json json;
	public:

		std::string path;
		std::string name;
		
		std::vector<std::string> scenes;

		std::unordered_map<std::string, uint32_t> scene_name_to_index;

		static Project current;

		static void LoadProject(const std::string &path);

		void SaveJSON();

		void SaveEditorSettingValue(const std::string& name, const nlohmann::json &value);
		void SaveSettingValue(const std::string &name, const nlohmann::json &value);

		nlohmann::json LoadEditorSettingValue(const std::string& name, const nlohmann::json& default = {});
		nlohmann::json LoadSettingValue(const std::string& name, const nlohmann::json& default = {});
	};
}