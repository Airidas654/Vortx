#pragma once

#include "ECS/Entity.h"

#include <type_traits>

//#include "Scripting\ScriptBase.h"

namespace Vortx
{

    class BaseComponent
    {
    public:
        Entity GetEntity() const { return entity; }

        virtual bool Exists() = 0;

        template<typename T>
        decltype(auto) AddComponent()
        {
            if constexpr (std::is_base_of_v<BaseComponent, T>)
            {
                return T(entity, true);
            }
            else
            {
                /*T& comp = entity.AddComponent<T>();
                if constexpr (std::is_base_of_v<ScriptBase<T>, T>)
                {
                    comp.InnerSetGameObject(GameObject(entity));
                }*/
                return comp;
            }
        }

        template<typename T>
        decltype(auto) GetComponent()
        {
            if constexpr (std::is_base_of_v<BaseComponent, T>)
            {
                return T(entity);
            }
            else
            {
                return entity.GetComponent<T>();
            }
        }

        template<typename T>
        bool HasComponent()
        {
            if constexpr (std::is_base_of_v<BaseComponent, T>)
            {
                return T(entity).Exists();
            }
            else
            {
                return entity.HasComponent<T>();
            }
        }

    protected:
        Entity entity;

        virtual void _InternalCreate() = 0;

        BaseComponent() { entity = Entity(); };
        BaseComponent(Entity entity) : entity(entity) {}
        ~BaseComponent() = default;
    };
}