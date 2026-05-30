#pragma once

#include "ECS/Entity.h"

#include "entt.hpp"

#include <functional>
#include <unordered_map>
#include <optional>

static class PropertiesLoader
{
public:

	using DrawFn = std::function<void(Entity entity)>;

	static std::unordered_map<entt::id_type, std::string> componentNames;
	static std::unordered_map<entt::id_type, DrawFn> componentFunc;

	static void Reset();

	static void DrawPropertiesPanel(std::optional<Entity>& selected);

private:

	static void RegisterComponent(entt::id_type hash, const std::string& name, DrawFn fn)
	{
		componentFunc[hash] = std::move(fn);
		componentNames[hash] = name;
	}

	template<typename T>
	static void RegisterComponent(const std::string& name, DrawFn fn)
	{
		RegisterComponent(entt::type_hash<T>::value(), name, fn);
	}
};