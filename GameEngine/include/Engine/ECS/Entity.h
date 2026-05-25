#pragma once

#include "ECS/Registry.h"

class Entity
{
private:

	entt::entity entity;
	Registry& registry;

public:

	Entity(Registry& registry, entt::entity entity) : registry(registry), entity(entity) {}
	~Entity() = default;

	template<typename T>
	T& AddComponent()
	{
		return registry.AddComponent<T>(*this);
	}

	template<typename T>
	T& GetComponent()
	{
		return registry.GetComponent<T>(*this);
	}

	entt::entity GetValue() { return entity; }

	bool operator== (Entity& other) { return entity == other.entity; }
};
