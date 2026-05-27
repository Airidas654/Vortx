#pragma once

#include "ECS/Registry.h"

class Entity
{
private:

	entt::entity entity;
	Registry* registry;

public:

	Entity() = default;

	Entity(const Entity&) = default;
	Entity& operator=(const Entity&) = default;

	Entity(Registry* registry, entt::entity entity) : registry(registry), entity(entity) {}
	~Entity() = default;

	template<typename T>
	T& AddComponent()
	{
		return registry->AddComponent<T>(*this);
	}

	template<typename T>
	T& GetComponent()
	{
		return registry->GetComponent<T>(*this);
	}

	entt::entity GetValue() { return entity; }

	Registry& GetRegistry() { return *registry; }

	bool operator== (Entity& other) { return entity == other.entity; }
	bool operator== (const Entity& other) const { return entity == other.entity; }

	bool operator!= (Entity& other) { return entity != other.entity; }
	bool operator!= (const Entity& other) const { return entity != other.entity; }
};
