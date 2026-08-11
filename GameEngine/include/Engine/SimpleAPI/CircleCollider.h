#pragma once

#include "SimpleAPI/BaseComponent.h"
#include "SimpleAPI/GameObject.h"
#include "Physics/PhysicsTypes.h"

#include "EngineAPI.h"

#include <glm/glm.hpp>

namespace Vortx
{
    struct ENGINE_API CircleColliderData;

    class ENGINE_API CircleCollider : public BaseComponent
    {
    protected:
        void _InternalCreate() override;

    public:
        CircleCollider() = default;

        bool Exists() override;

        CircleCollider(const CircleCollider&) = default;
        CircleCollider& operator=(const CircleCollider&) = default;

        CircleCollider(CircleCollider&&) = default;
        CircleCollider& operator=(CircleCollider&&) = default;

        CircleCollider(Entity entity) : BaseComponent(entity) {}

        CircleCollider(Entity entity, bool create);

        CircleColliderData& Data();

        glm::vec2 GetOffset();
        void SetOffset(const glm::vec2& offset);

        float GetRadius();
        void SetRadius(float radius);

        bool IsTrigger();
        void SetTrigger(bool isTrigger);

        float GetDensity();
        void SetDensity(float density);

        float GetFriction();
        void SetFriction(float friction);
        float GetRestitution();
        void SetRestitution(float restitution);

        glm::vec2 ClosestPoint(const glm::vec2& point);

        CollisionFilter GetFilter();
        void SetFilter(const CollisionFilter& filter);

        void GetWorldBounds(glm::vec2& outMin, glm::vec2& outMax);

        GameObject GetGameObject();

        bool operator== (CircleCollider& other) { return entity == other.entity; }
        bool operator== (const CircleCollider& other) const { return entity == other.entity; }

        bool operator!= (CircleCollider& other) { return entity != other.entity; }
        bool operator!= (const CircleCollider& other) const { return entity != other.entity; }
    };
}