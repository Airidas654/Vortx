#pragma once

#include "SimpleAPI/BaseComponent.h"
#include "SimpleAPI/GameObject.h"
#include "Physics/PhysicsTypes.h"

#include "EngineAPI.h"

#include <glm/glm.hpp>

namespace Vortx
{
    struct ENGINE_API BoxColliderData;

    class ENGINE_API BoxCollider : public BaseComponent
    {
    protected:
        void _InternalCreate() override;

    public:
        BoxCollider() = default;

        BoxCollider(const BoxCollider&) = default;
        BoxCollider& operator=(const BoxCollider&) = default;

        BoxCollider(BoxCollider&&) = default;
        BoxCollider& operator=(BoxCollider&&) = default;

        BoxCollider(Entity entity) : BaseComponent(entity) {}

        BoxCollider(Entity entity, bool create);

        BoxColliderData& Data();

        glm::vec2 GetOffset();
        void SetOffset(const glm::vec2& offset);

        glm::vec2 GetSize();
        void SetSize(const glm::vec2& size);

        bool IsTrigger();
        void SetTrigger(bool isTrigger);

        float GetDensity();
        void SetDensity(float density); // marks owning Rigidbody2D's mass dirty if useAutoMass is on

        float GetFriction();
        void SetFriction(float friction);
        float GetRestitution();
        void SetRestitution(float restitution);

        CollisionFilter GetFilter();
        void SetFilter(const CollisionFilter& filter);

        // World-space AABB accounting for Transform position/rotation/scale - for gizmos and quick overlap checks
        void GetWorldBounds(glm::vec2& outMin, glm::vec2& outMax);

        GameObject GetGameObject();

        bool operator== (BoxCollider& other) { return entity == other.entity; }
        bool operator== (const BoxCollider& other) const { return entity == other.entity; }

        bool operator!= (BoxCollider& other) { return entity != other.entity; }
        bool operator!= (const BoxCollider& other) const { return entity != other.entity; }
    };
}