#pragma once

#include "SimpleAPI/BaseComponent.h"
#include "SimpleAPI/GameObject.h"
#include "Physics/PhysicsTypes.h"

#include "EngineAPI.h"

#include <glm/glm.hpp>

namespace Vortx
{
    struct ENGINE_API RigidbodyData;

    class ENGINE_API Rigidbody : public BaseComponent
    {
    protected:
        void _InternalCreate() override;

    public:
        Rigidbody() = default;

        bool Exists() override;

        Rigidbody(const Rigidbody&) = default;
        Rigidbody& operator=(const Rigidbody&) = default;

        Rigidbody(Rigidbody&&) = default;
        Rigidbody& operator=(Rigidbody&&) = default;

        Rigidbody(Entity entity) : BaseComponent(entity) {}

        Rigidbody(Entity entity, bool create);

        RigidbodyData& Data();

        BodyType GetBodyType();
        void SetBodyType(BodyType type);

        float GetMass();
        void SetMass(float mass);
        void SetAutoMass(bool enabled);
        bool GetAutoMass();
        float GetInverseMass();
        float GetInertia();
        glm::vec2 GetLocalCenterOfMass();
        glm::vec2 GetWorldCenterOfMass();

        glm::vec2 GetVelocity();
        void SetVelocity(const glm::vec2& velocity);
        float GetAngularVelocity();
        void SetAngularVelocity(float angularVelocity);

        float GetGravityScale();
        void SetGravityScale(float scale);

        float GetLinearDamping();
        void SetLinearDamping(float damping);
        float GetAngularDamping();
        void SetAngularDamping(float damping);

        bool IsFixedRotation();
        void SetFixedRotation(bool fixedRotation);

        bool IsSleepingAllowed();
        void SetSleepingAllowed(bool allowed);
        bool IsAwake();
        void WakeUp();
        void Sleep();

        void ApplyForce(const glm::vec2& force);
        void ApplyForceAtPoint(const glm::vec2& force, const glm::vec2& worldPoint);
        void ApplyTorque(float torque);

        void ApplyImpulse(const glm::vec2& impulse);
        void ApplyImpulseAtPoint(const glm::vec2& impulse, const glm::vec2& worldPoint);
        void ApplyAngularImpulse(float impulse);

        glm::vec2 GetPointVelocity(const glm::vec2& worldPoint);

        void MarkMassDirty();

        GameObject GetGameObject();

        bool operator== (Rigidbody& other) { return entity == other.entity; }
        bool operator== (const Rigidbody& other) const { return entity == other.entity; }

        bool operator!= (Rigidbody& other) { return entity != other.entity; }
        bool operator!= (const Rigidbody& other) const { return entity != other.entity; }
    };
}