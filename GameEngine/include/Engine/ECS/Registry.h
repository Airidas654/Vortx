#pragma once

#include "EngineAPI.h"

#include "entt.hpp"

namespace Vortx
{

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

		auto Storage(entt::id_type id)
		{
			return registry.storage(id);
		}

		void Clear()
		{
			registry.clear();
		}

		void Destroy(Entity& entity);
		

		template<typename T>
		auto View()
		{
			return registry.view<T>();
		}

		template<typename T1, typename T2>
		auto View()
		{
			return registry.view<T1,T2>();
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

		template<typename T>
		void RemoveComponent(Entity& entity)
		{
			registry.remove<T>(entity.GetValue());
		}

		template<typename T>
		bool HasComponent(Entity& entity)
		{
			if (registry.valid(entity.GetValue()))
			{
				if (registry.try_get<T>(entity.GetValue()))
				{
					return 1;
				}
			}
			return 0;
		}

		bool HasComponent(Entity& entity, entt::id_type id);

	};
}