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
	template<typename T>
	static void RegisterComponent(const std::string& name, DrawFn fn)
	{
		auto id = entt::type_hash<T>::value();
		componentFunc[id] = std::move(fn);
		componentNames[id] = name;
	}
};