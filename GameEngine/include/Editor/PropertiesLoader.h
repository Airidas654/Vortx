#pragma once

#include "ECS/Entity.h"
#include "AssetID.h"

#include "entt.hpp"

#include <functional>
#include <unordered_map>
#include <optional>

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

	static std::unordered_map<entt::id_type, ComponentData> componentData;
	static std::unordered_map<entt::id_type, DrawFn> componentFunc;

	static void Reset();

	static void DrawPropertiesPanel(const AssetID& selected);

private:

	static void RegisterComponent(entt::id_type hash, const std::string& name, bool useHeader, DrawFn fn)
	{
		componentFunc[hash] = std::move(fn);
		componentData[hash] = { name, useHeader };
	}

	template<typename T>
	static void RegisterComponent(const std::string& name, bool useHeader, DrawFn fn)
	{
		RegisterComponent(entt::type_hash<T>::value(), name, useHeader, fn);
	}

	static void DrawAddComponentPopup(Entity entity);
	static std::string CleanScriptName(const std::string& raw);
};