#pragma once

#include "ECS/Entity.h"

#include <type_traits>

#include "Scripting\ScriptBase.h"

class BaseComponent
{
public:
	Entity GetEntity() const { return entity; }

    template<typename T>
    T AddComponent()
    {
        if constexpr (std::is_base_of_v<ScriptBase<T>, T>)
        {
            T& script = entity.AddComponent<T>();
            script.InnerSetGameObject(GameObject(entity));
            return script;
        }
        else
        {
            return T(entity, true);
        }
    }

    template<typename T>
    T GetComponent()
    {
        if constexpr (std::is_base_of_v<ScriptBase<T>, T>)
        {
            return entity.GetComponent<T>();
        }
        else
        {
            return T(entity);
        }
    }

protected:
	Entity entity;

	virtual void _InternalCreate() = 0;

    BaseComponent() { entity = Entity(); };
	BaseComponent(Entity entity) : entity(entity) {}
	~BaseComponent() = default;
};