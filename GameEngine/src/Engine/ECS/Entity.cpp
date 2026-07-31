#include "ECS/Entity.h"
namespace Vortx
{

	Entity::Entity()
	{
		entity = (entt::entity)UINT32_MAX;
		registry = nullptr;
	}

}