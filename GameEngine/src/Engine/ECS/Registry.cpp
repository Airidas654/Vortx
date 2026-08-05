#include "ECS/Entity.h"
#include "ECS/Registry.h"

#include <iostream>

namespace Vortx
{

	Entity Registry::Create()
	{
		return Entity(this, registry.create());
	}

	bool Registry::HasComponent(Entity& entity, entt::id_type id)
	{
		auto* storage = registry.storage(id);
		return storage && storage->contains(entity.GetValue());
	}

	void Registry::Destroy(Entity& entity)
	{
		registry.destroy(entity.GetValue());
	}
}