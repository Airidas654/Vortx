#pragma once

#include "EngineAPI.h"

#include "entt.hpp"

class Entity;

class ENGINE_API Registry
{
private:
	entt::registry registry;

public:

	Registry() = default;
	~Registry() = default;

	Entity Create();

	auto Storage()
	{
		return registry.storage();
	}

	void Clear()
	{
		registry.clear();
	}

	template<typename T>
	auto View()
	{
		return registry.view<T>();
	}

	template<typename T>
	T& AddComponent(Entity& entity)
	{
		return registry.emplace<T>(entity.GetValue());
	}

	template<typename T>
	T& GetComponent(Entity& entity)
	{
		return registry.get<T>(entity.GetValue());
	}

};