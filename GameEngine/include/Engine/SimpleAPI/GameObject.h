#pragma once

#include "entt.hpp"

class GameObject
{
public:
    GameObject(entt::entity handle, entt::registry& registry)
        : entity(handle), registry(registry)
    {
    }

    template<typename T, typename... Args>
    T& AddComponent(Args&&... args)
    {
        return T(entity, registry, std::forward<Args>(args)...);
        //return registry.emplace<T>(entity, *this, std::forward<Args>(args)...);
    }

    template<typename T>
    T& GetComponent()
    {
        return registry.get<T>(entity);
    }

private:
    entt::entity entity;
    entt::registry& registry;
};