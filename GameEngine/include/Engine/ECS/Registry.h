#pragma once

#include "entt.hpp"

class Entity;

class Registry
{
private:
	entt::registry registry;

public:

	Registry() = default;
	~Registry() = default;

	Entity Create();

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