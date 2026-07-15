#pragma once

#include "ECS/Entity.h"
#include "AssetID.h"

#include "entt.hpp"

#include <functional>
#include <unordered_map>
#include <optional>
#include <map>

using namespace Vortx;

class PropertiesLoader
{
	struct ComponentData
	{
		std::string name;
		bool useHeader;
	};

public:

	using DrawFn = std::function<void(Entity entity)>;
	using AddFn = std::function<void(Entity entity)>;

	static std::unordered_map<entt::id_type, ComponentData> componentData;
	static std::unordered_map<entt::id_type, DrawFn> componentFunc;
	static std::unordered_map<entt::id_type, AddFn> componentAddFunc;

	static void Reset();

	static void DrawPropertiesPanel(const AssetID& selected);

private:

	static std::map<std::string, std::vector<entt::id_type>> addFolderList;

	static void RegisterComponent(entt::id_type hash, const std::string& name, const std::string& folderPath, bool useHeader, DrawFn drawFn, AddFn addFn)
	{
		auto it = addFolderList.find(folderPath);

		if (it == addFolderList.end()) {
			addFolderList[folderPath] = {hash};
		}
		else {
			std::get<1>(*it).emplace_back(hash);
		}

		componentFunc[hash] = std::move(drawFn);
		componentAddFunc[hash] = std::move(addFn);
		componentData[hash] = { name, useHeader };
	}

	template<typename T>
	static void RegisterComponent(const std::string& name, const std::string& folderPath, bool useHeader, DrawFn drawFn, AddFn addFn)
	{
		RegisterComponent(entt::type_hash<T>::value(), name, folderPath, useHeader, drawFn, addFn);
	}

	static void DrawAddComponentPopup(Entity entity);
	static std::string CleanScriptName(const std::string& raw);
	static void AddComponent(Entity entity, entt::id_type hash);
};