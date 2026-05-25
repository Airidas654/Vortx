#pragma once

#include "ECS/Entity.h"

class GameObject
{
public:
    GameObject(Entity entity)
        : entity(entity)
    {
    }

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

private:
    Entity entity;
};