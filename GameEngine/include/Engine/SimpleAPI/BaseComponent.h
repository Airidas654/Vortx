#pragma once

#include "ECS/Entity.h"

class BaseComponent
{
public:
	Entity GetEntity() const { return entity; }

protected:
	Entity entity;

	virtual void _InternalCreate() = 0;

	BaseComponent(Entity entity) : entity(entity) {}
	~BaseComponent() = default;

};