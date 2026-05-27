#pragma once

#include "ECS/Entity.h"

class BaseComponent
{
public:
	Entity GetEntity() const { return entity; }

    template<typename T>
    T AddComponent()
    {
        return T(entity, true);
    }

    template<typename T>
    T GetComponent()
    {
        return T(entity);
    }

protected:
	Entity entity;

	virtual void _InternalCreate() = 0;

    BaseComponent() { entity = Entity(); };
	BaseComponent(Entity entity) : entity(entity) {}
	~BaseComponent() = default;
};