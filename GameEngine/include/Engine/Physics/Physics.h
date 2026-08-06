#pragma once

#include "EngineAPI.h"
#include "ECS/Entity.h"
#include "Physics/PhysicsTypes.h"

#include <glm/glm.hpp>

#include <vector>
#include <functional>

// Physics2D is a static manager in the same vein as Time/PlayModeManager/
// ScriptSystem: no instances, engine-lifetime state, driven once per frame
// (or fixed tick) by the owner of the main loop.
//
// Threading & call-order contract:
//   - Not thread-safe. Call Step() and the query functions from the main
//     thread only, same assumption the rest of the engine already makes
//     (see Logger's unprotected-buffer issue for what happens otherwise).
//   - Physics2D does NOT own a fixed-timestep accumulator. The caller
//     (Editor's play-mode loop, same place ScriptSystem::Instance().Update()
//     is already called) is responsible for calling Step(Time::fixedDeltaTime)
//     once per fixed tick. This mirrors how Time::fixedDeltaTime already
//     exists but nothing currently consumes it.
//   - Step() must run BEFORE ScriptSystem::Instance().Update() in a given
//     tick if scripts should see this tick's collision events, since the
//     event buffers below are populated during Step() and cleared at the
//     start of the NEXT Step().

namespace Vortx
{
    // Result of a single raycast hit.
    struct ENGINE_API RaycastHit
    {
        Entity entity;
        glm::vec2 point{ 0.0f };    // world-space hit position
        glm::vec2 normal{ 0.0f };   // world-space surface normal at the hit, pointing back toward the ray origin
        float distance = 0.0f;      // distance from ray origin to point
        bool isTrigger = false;
    };

    // One resolved contact between two colliders in a given Step(). For
    // multi-point manifolds (box-box) this is the deepest point; that's
    // enough for gameplay-facing enter/exit events without exposing full
    // manifold internals in the public API.
    struct ENGINE_API Contact
    {
        Entity entityA;
        Entity entityB;
        glm::vec2 point{ 0.0f };     // world-space contact point
        glm::vec2 normal{ 0.0f };    // world-space, points from A to B
        float separation = 0.0f;     // negative = penetrating
    };

    class ENGINE_API Physics
    {
    public:
        // Allocates internal broad-phase/contact-cache state. Call once
        // after Engine::Init(), mirrors how Renderer2D::Init is paired
        // with Renderer2D::Shutdown.
        static void Init();
        static void Shutdown();

        // Clears cached contacts, sleep state, and per-body accumulators.
        // Call on scene load/unload and on PlayModeManager::ExitPlay so
        // stale contact pairs don't leak into the next play session -
        // same category of bug as the ScriptInstance leak on scene swap
        // noted for ScriptSystem.
        static void Reset();

        // Advances the simulation by exactly `fixedDeltaTime`. Internally:
        // integrate forces -> broad phase -> narrow phase (Box-Box,
        // Box-Circle, Circle-Circle - the only three pairs possible with
        // two shapes) -> solve velocity constraints -> solve position
        // (baumgarte/PGS) -> sleep bookkeeping -> populate contact event
        // buffers. Not reentrant; do not call from within a callback
        // triggered by a previous Step().
        static void Step(float fixedDeltaTime);

        // --- Global settings ---
        static void SetGravity(const glm::vec2& gravity);
        static glm::vec2 GetGravity();

        static void SetVelocityIterations(int iterations); // default 8, Box2D-style solver iteration count
        static void SetPositionIterations(int iterations);  // default 3

        // --- Mass invalidation hook ---
        // Called by Rigidbody2D::SetMass/SetAutoMass and by
        // BoxCollider2D::SetSize/SetDensity, CircleCollider2D::SetRadius/
        // SetDensity, etc. so Physics2D recomputes inverseMass/inertia/
        // centerOfMass for that body at the start of the next Step()
        // instead of every frame regardless of change.
        static void MarkMassDirty(Entity entity);

        // Force an immediate recompute instead of waiting for next Step(),
        // e.g. so a script can read GetInertia() right after changing a
        // collider and get a correct value this frame.
        static void RecomputeMassNow(Entity entity);

        // --- Queries ---
        // layerMask is matched against each candidate collider's
        // CollisionFilter2D.categoryBits (mask & categoryBits != 0);
        // groupIndex overrides are ignored for queries, only used during
        // Step()'s narrow phase between two simulated bodies.

        static bool Raycast(const glm::vec2& origin, const glm::vec2& direction, float maxDistance,
            RaycastHit& outHit, uint32_t layerMask = 0xFFFFFFFF);

        static std::vector<RaycastHit> RaycastAll(const glm::vec2& origin, const glm::vec2& direction,
            float maxDistance, uint32_t layerMask = 0xFFFFFFFF);

        static bool OverlapPoint(const glm::vec2& point, Entity& outEntity, uint32_t layerMask = 0xFFFFFFFF);

        static std::vector<Entity> OverlapCircle(const glm::vec2& center, float radius,
            uint32_t layerMask = 0xFFFFFFFF);

        static std::vector<Entity> OverlapBox(const glm::vec2& center, const glm::vec2& size, glm::vec1 rotation,
            uint32_t layerMask = 0xFFFFFFFF);

        // --- Collision / trigger events for this most recent Step() ---
        // Cleared and repopulated every Step(); a script polling these in
        // its Update() sees exactly the events from the Step() that ran
        // earlier this tick, not an accumulating history.
        static const std::vector<Contact>& GetCollisionEnters();
        static const std::vector<Contact>& GetCollisionStays();
        static const std::vector<Contact>& GetCollisionExits();

        // Trigger pairs report entities only (no contact point/normal -
        // triggers generate no collision response, so a manifold isn't
        // meaningful).
        static const std::vector<std::pair<Entity, Entity>>& GetTriggerEnters();
        static const std::vector<std::pair<Entity, Entity>>& GetTriggerStays();
        static const std::vector<std::pair<Entity, Entity>>& GetTriggerExits();

        // --- Debug ---
        // Number of active (non-sleeping) dynamic bodies this Step() -
        // exposed for an editor stats overlay, not used internally.
        static uint32_t GetActiveBodyCount();

    private:
        Physics() = delete;
    };
}