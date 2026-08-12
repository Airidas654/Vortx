#include "Physics/Physics.h"
#include "Components/ComponentRigidbody.h"
#include "Components/ComponentBoxCollider.h"
#include "Components/ComponentCircleCollider.h"
#include "SimpleAPI/Transform.h"
#include "SimpleAPI/GameObject.h"
#include "Scene.h"
#include "SimpleAPI/Math.h"

#include <algorithm>
#include <unordered_map>
#include <array>
#include <cmath>
#include <iostream>

namespace Vortx
{
    namespace
    {

        glm::vec2 InverseRotate(const glm::vec2& v, float angle) { return Math::Rotate(v, -angle); }

        struct ShapeProxy
        {
            Entity entity;
            bool isCircle;
            glm::vec2 center;
            float rotation;
            glm::vec2 halfExtents;
            float radius;
            bool isTrigger;
            PhysicsMaterial material;
            CollisionFilter filter;
            RigidbodyData* body;
        };

        struct ManifoldPoint
        {
            glm::vec2 point{ 0.0f };
            float penetration = 0.0f;
            float normalImpulse = 0.0f;
            float tangentImpulse = 0.0f;
        };

        struct Manifold
        {
            bool hit = false;
            glm::vec2 normal{ 0.0f };
            int pointCount = 0;
            ManifoldPoint points[2];
        };

        Manifold CircleVsCircle(const ShapeProxy& a, const ShapeProxy& b)
        {
            Manifold m;
            glm::vec2 d = b.center - a.center;
            float rSum = a.radius + b.radius;
            float distSq = glm::dot(d, d);
            if (distSq >= rSum * rSum) return m;

            float dist = sqrtf(distSq);
            glm::vec2 normal;
            if (dist > 1e-6f) normal = d / dist;
            else { normal = { 1.0f, 0.0f }; dist = 0.0f; }

            m.hit = true;
            m.normal = normal;
            m.pointCount = 1;
            m.points[0].penetration = rSum - dist;
            m.points[0].point = a.center + normal * a.radius;
            return m;
        }

        Manifold BoxVsCircle(const ShapeProxy& box, const ShapeProxy& circle)
        {
            Manifold m;

            glm::vec2 localCircle = InverseRotate(circle.center - box.center, box.rotation);
            glm::vec2 clamped = glm::clamp(localCircle, -box.halfExtents, box.halfExtents);

            bool inside = (clamped == localCircle);
            glm::vec2 localNormal;
            float penetration;

            if (!inside)
            {
                glm::vec2 diff = localCircle - clamped;
                float distSq = glm::dot(diff, diff);
                if (distSq >= circle.radius * circle.radius) return m;

                float dist = sqrtf(distSq);
                localNormal = diff / dist;
                penetration = circle.radius - dist;
            }
            else
            {
                glm::vec2 toEdge = box.halfExtents - glm::abs(localCircle);
                if (toEdge.x < toEdge.y)
                {
                    localNormal = { localCircle.x < 0.0f ? -1.0f : 1.0f, 0.0f };
                    penetration = toEdge.x + circle.radius;
                }
                else
                {
                    localNormal = { 0.0f, localCircle.y < 0.0f ? -1.0f : 1.0f };
                    penetration = toEdge.y + circle.radius;
                }
            }

            glm::vec2 normal = Math::Rotate(localNormal, box.rotation);

            m.hit = true;
            m.normal = normal;
            m.pointCount = 1;
            m.points[0].penetration = penetration;
            m.points[0].point = circle.center - normal * circle.radius;
            return m;
        }

        std::array<glm::vec2, 4> GetCorners(const ShapeProxy& box)
        {
            glm::vec2 bx = Math::Rotate({ 1,0 }, box.rotation);
            glm::vec2 by = Math::Rotate({ 0,1 }, box.rotation);
            glm::vec2 hx = bx * box.halfExtents.x;
            glm::vec2 hy = by * box.halfExtents.y;
            return {
                box.center - hx - hy,
                box.center + hx - hy,
                box.center + hx + hy,
                box.center - hx + hy
            };
        }

        // Two-point clipped manifold (SAT, then clip the incident edge against
    // the reference face's side planes - the standard box2d-lite/Catto
    // recipe). Specialized for rectangles: a box's face containing its
    // support vertex along any axis is always its two corners with the
    // extreme projection onto that axis, so reference/incident face
    // vertices can be found by sorting projections instead of a face-
    // normal lookup table.
    //
    // Fixes the earlier single-point bug too: that version always used
    // a.center's face regardless of which box the axis actually came
    // from, and ignored the incident box's tangential position entirely.
        Manifold BoxVsBox(const ShapeProxy& a, const ShapeProxy& b)
        {
            Manifold m;

            glm::vec2 axes[4] =
            {
                Math::Rotate({1,0}, a.rotation), Math::Rotate({0,1}, a.rotation),
                Math::Rotate({1,0}, b.rotation), Math::Rotate({0,1}, b.rotation)
            };

            glm::vec2 d = b.center - a.center;

            auto ProjectRadius = [](const ShapeProxy& box, const glm::vec2& axis)
                {
                    glm::vec2 bx = Math::Rotate({ 1,0 }, box.rotation);
                    glm::vec2 by = Math::Rotate({ 0,1 }, box.rotation);
                    return box.halfExtents.x * fabsf(glm::dot(bx, axis)) +
                        box.halfExtents.y * fabsf(glm::dot(by, axis));
                };

            float minOverlap = FLT_MAX;
            glm::vec2 minAxis{ 0.0f };
            bool refIsA = true;

            for (int i = 0; i < 4; i++)
            {
                const glm::vec2& axis = axes[i];

                float ra = ProjectRadius(a, axis);
                float rb = ProjectRadius(b, axis);
                float dist = fabsf(glm::dot(d, axis));
                float overlap = ra + rb - dist;

                if (overlap < 0.0f) return m;

                if (overlap < minOverlap)
                {
                    minOverlap = overlap;
                    minAxis = (glm::dot(d, axis) < 0.0f) ? -axis : axis;
                    refIsA = (i < 2);
                }
            }

            const ShapeProxy& refBox = refIsA ? a : b;
            const ShapeProxy& incBox = refIsA ? b : a;
            glm::vec2 normal = minAxis; // A -> B
            glm::vec2 refFaceNormal = refIsA ? normal : -normal; // outward from refBox

            std::array<glm::vec2, 4> refCorners = GetCorners(refBox);
            std::array<glm::vec2, 4> incCorners = GetCorners(incBox);

            // Reference face = refBox's 2 corners with LARGEST projection onto
            // refFaceNormal (the face nearest the incident box).
            int refI0 = 0, refI1 = 1;
            {
                float best0 = -FLT_MAX, best1 = -FLT_MAX;
                for (int i = 0; i < 4; i++)
                {
                    float p = glm::dot(refCorners[i], refFaceNormal);
                    if (p > best0) { best1 = best0; refI1 = refI0; best0 = p; refI0 = i; }
                    else if (p > best1) { best1 = p; refI1 = i; }
                }
            }

            // Incident face = incBox's 2 corners with SMALLEST projection onto
            // refFaceNormal (the face burrowing furthest into refBox).
            int incI0 = 0, incI1 = 1;
            {
                float worst0 = FLT_MAX, worst1 = FLT_MAX;
                for (int i = 0; i < 4; i++)
                {
                    float p = glm::dot(incCorners[i], refFaceNormal);
                    if (p < worst0) { worst1 = worst0; incI1 = incI0; worst0 = p; incI0 = i; }
                    else if (p < worst1) { worst1 = p; incI1 = i; }
                }
            }

            glm::vec2 r1 = refCorners[refI0];
            glm::vec2 r2 = refCorners[refI1];
            glm::vec2 tangent(-refFaceNormal.y, refFaceNormal.x);

            float tanA = glm::dot(r1, tangent);
            float tanB = glm::dot(r2, tangent);
            float tanMin = std::min(tanA, tanB);
            float tanMax = std::max(tanA, tanB);
            float refFaceOffset = 0.5f * (glm::dot(r1, refFaceNormal) + glm::dot(r2, refFaceNormal));

            glm::vec2 incidentPts[2] = { incCorners[incI0], incCorners[incI1] };

            m.hit = true;
            m.normal = normal;
            m.pointCount = 0;

            for (int i = 0; i < 2; i++)
            {
                glm::vec2 raw = incidentPts[i];
                float penetration = refFaceOffset - glm::dot(raw, refFaceNormal);
                if (penetration <= 0.0f) continue; // not actually behind the reference face

                float tanCoord = std::clamp(glm::dot(raw, tangent), tanMin, tanMax);
                glm::vec2 clipped = refFaceNormal * refFaceOffset + tangent * tanCoord;

                ManifoldPoint& mp = m.points[m.pointCount++];
                mp.point = clipped;
                mp.penetration = penetration;
            }

            // SAT confirmed overlap, so at least one point should survive; if
            // clipping degenerates to zero at a glancing corner, fall back to
            // the single deepest raw vertex so hit=true never implies
            // pointCount=0.
            if (m.pointCount == 0)
            {
                int deepest = (glm::dot(incidentPts[0], refFaceNormal) < glm::dot(incidentPts[1], refFaceNormal)) ? 0 : 1;
                m.points[0].point = incidentPts[deepest];
                m.points[0].penetration = minOverlap;
                m.pointCount = 1;
            }

            return m;
        }

        struct PairEntities { Entity a, b; };

        struct ContactCache
        {
            Entity a, b;
            glm::vec2 normal{ 0.0f };
            int pointCount = 0;
            float normalImpulse[2]{};
            float tangentImpulse[2]{};
        };

        glm::vec2 s_gravity{ 0.0f, -9.81f };
        int s_velocityIterations = 8;
        int s_positionIterations = 3;

        // Sleeping thresholds - in the spirit of Box2D's defaults: ~1cm/s
        // linear, ~2deg/s angular, held for half a second.
        constexpr float kLinearSleepToleranceSq = 0.01f * 0.01f;
        constexpr float kAngularSleepTolerance = 2.0f * 0.0174533f;
        constexpr float kTimeToSleep = 0.5f;

        std::vector<Contact> s_enters, s_stays, s_exits;
        std::vector<std::pair<Entity, Entity>> s_triggerEnters, s_triggerStays, s_triggerExits;

        using PairKey = uint64_t;
        PairKey MakeKey(entt::entity a, entt::entity b)
        {
            uint32_t lo = std::min((uint32_t)a, (uint32_t)b);
            uint32_t hi = std::max((uint32_t)a, (uint32_t)b);
            return (uint64_t(hi) << 32) | uint64_t(lo);
        }

        std::unordered_map<PairKey, ContactCache> s_activeContacts;
        std::unordered_map<PairKey, PairEntities> s_activeTriggers;

        void ApplyPositionalCorrection(Transform tA, Transform tB, float invMassA, float invMassB,
            const glm::vec2& normal, float penetration)
        {
            constexpr float kSlop = 0.01f;
            constexpr float kPercent = 0.2f;

            float invMassSum = invMassA + invMassB;
            if (invMassSum <= 0.0f) return;

            float correctionMag = std::max(penetration - kSlop, 0.0f) / invMassSum * kPercent;
            glm::vec2 correction = correctionMag * normal;

            if (invMassA > 0.0f) tA.SetPosition(tA.Position() - correction * invMassA);
            if (invMassB > 0.0f) tB.SetPosition(tB.Position() + correction * invMassB);
        }

        // Applies last frame's cached impulse for this point immediately -
    // "warm starting". Seeds the solver near the converged answer instead
    // of from zero, which is most of what makes stacking stable at low
    // iteration counts.
        void ApplyWarmStart(RigidbodyData* bodyA, RigidbodyData* bodyB,
            const glm::vec2& posA, const glm::vec2& posB, const glm::vec2& normal, ManifoldPoint& mp)
        {
            float invMassA = bodyA ? bodyA->inverseMass : 0.0f;
            float invMassB = bodyB ? bodyB->inverseMass : 0.0f;
            float invInertiaA = bodyA ? bodyA->inverseInertia : 0.0f;
            float invInertiaB = bodyB ? bodyB->inverseInertia : 0.0f;
            if (invMassA == 0.0f && invMassB == 0.0f) return;
            if (mp.normalImpulse == 0.0f && mp.tangentImpulse == 0.0f) return;

            glm::vec2 rA = mp.point - posA;
            glm::vec2 rB = mp.point - posB;
            glm::vec2 tangent(-normal.y, normal.x);

            glm::vec2 impulse = mp.normalImpulse * normal + mp.tangentImpulse * tangent;

            if (bodyA) { bodyA->velocity -= impulse * invMassA; bodyA->angularVelocity -= invInertiaA * Math::Cross(rA, impulse); }
            if (bodyB) { bodyB->velocity += impulse * invMassB; bodyB->angularVelocity += invInertiaB * Math::Cross(rB, impulse); }
        }

        // Solves one point's normal + friction impulse in place, accumulating
    // into mp.normalImpulse/tangentImpulse across every call this step
    // (i.e. across velocity iterations, seeded by ApplyWarmStart above).
        void SolveVelocityConstraint(RigidbodyData* bodyA, RigidbodyData* bodyB,
            const glm::vec2& posA, const glm::vec2& posB, const glm::vec2& normal,
            ManifoldPoint& mp, float friction, float restitution, float restitutionThreshold)
        {
            float invMassA = bodyA ? bodyA->inverseMass : 0.0f;
            float invMassB = bodyB ? bodyB->inverseMass : 0.0f;
            float invInertiaA = bodyA ? bodyA->inverseInertia : 0.0f;
            float invInertiaB = bodyB ? bodyB->inverseInertia : 0.0f;
            if (invMassA == 0.0f && invMassB == 0.0f) return;

            glm::vec2 rA = mp.point - posA;
            glm::vec2 rB = mp.point - posB;
            glm::vec2 tangent(-normal.y, normal.x);

            auto RelativeVelocity = [&]() -> glm::vec2
                {
                    glm::vec2 velA = bodyA ? bodyA->velocity : glm::vec2(0.0f);
                    glm::vec2 velB = bodyB ? bodyB->velocity : glm::vec2(0.0f);
                    float wA = bodyA ? bodyA->angularVelocity : 0.0f;
                    float wB = bodyB ? bodyB->angularVelocity : 0.0f;
                    return (velB + Math::Cross(wB, rB)) - (velA + Math::Cross(wA, rA));
                };

            // --- Normal ---
            float raCrossN = Math::Cross(rA, normal);
            float rbCrossN = Math::Cross(rB, normal);
            float normalMassSum = invMassA + invMassB +
                invInertiaA * raCrossN * raCrossN + invInertiaB * rbCrossN * rbCrossN;

            if (normalMassSum > 0.0f)
            {
                float vn = glm::dot(RelativeVelocity(), normal);

                float e = restitution;
                if (fabsf(vn) < restitutionThreshold) e = 0.0f;

                float lambda = -(1.0f + e) * vn / normalMassSum;

                float newImpulse = std::max(mp.normalImpulse + lambda, 0.0f);
                lambda = newImpulse - mp.normalImpulse;
                mp.normalImpulse = newImpulse;

                glm::vec2 impulse = lambda * normal;
                if (bodyA) { bodyA->velocity -= impulse * invMassA; bodyA->angularVelocity -= invInertiaA * Math::Cross(rA, impulse); }
                if (bodyB) { bodyB->velocity += impulse * invMassB; bodyB->angularVelocity += invInertiaB * Math::Cross(rB, impulse); }
            }

            // --- Friction, clamped against THIS point's accumulated normal impulse ---
            float raCrossT = Math::Cross(rA, tangent);
            float rbCrossT = Math::Cross(rB, tangent);
            float tangentMassSum = invMassA + invMassB +
                invInertiaA * raCrossT * raCrossT + invInertiaB * rbCrossT * rbCrossT;

            if (tangentMassSum > 0.0f)
            {
                float vt = glm::dot(RelativeVelocity(), tangent);
                float lambda = -vt / tangentMassSum;

                float maxFriction = friction * mp.normalImpulse;
                float newImpulse = std::clamp(mp.tangentImpulse + lambda, -maxFriction, maxFriction);
                lambda = newImpulse - mp.tangentImpulse;
                mp.tangentImpulse = newImpulse;

                glm::vec2 impulse = lambda * tangent;
                if (bodyA) { bodyA->velocity -= impulse * invMassA; bodyA->angularVelocity -= invInertiaA * Math::Cross(rA, impulse); }
                if (bodyB) { bodyB->velocity += impulse * invMassB; bodyB->angularVelocity += invInertiaB * Math::Cross(rB, impulse); }
            }
        }

        /*void ResolveContact(RigidbodyData* bodyA, RigidbodyData* bodyB,
            const glm::vec2& posA, const glm::vec2& posB, const Manifold& man,
            float frictionA, float frictionB, float restitutionA, float restitutionB,
            float restitutionThreshold)
        {
            static RigidbodyData s_staticDummy; 

            float invMassA = bodyA ? bodyA->inverseMass : 0.0f;
            float invMassB = bodyB ? bodyB->inverseMass : 0.0f;
            float invInertiaA = bodyA ? bodyA->inverseInertia : 0.0f;
            float invInertiaB = bodyB ? bodyB->inverseInertia : 0.0f;

            if (invMassA == 0.0f && invMassB == 0.0f) return;

            glm::vec2 rA = man.point - posA;
            glm::vec2 rB = man.point - posB;

            glm::vec2 velA = bodyA ? bodyA->velocity : glm::vec2(0.0f);
            glm::vec2 velB = bodyB ? bodyB->velocity : glm::vec2(0.0f);
            float wA = bodyA ? bodyA->angularVelocity : 0.0f;
            float wB = bodyB ? bodyB->angularVelocity : 0.0f;

            glm::vec2 relVel = (velB + Math::Cross(wB, rB)) - (velA + Math::Cross(wA, rA));
            float velAlongNormal = glm::dot(relVel, man.normal);
            if (velAlongNormal > 0.0f) return;

            float raCrossN = Math::Cross(rA, man.normal);
            float rbCrossN = Math::Cross(rB, man.normal);
            float invMassSum = invMassA + invMassB +
                invInertiaA * raCrossN * raCrossN + invInertiaB * rbCrossN * rbCrossN;
            if (invMassSum <= 0.0f) return;

            float e = std::max(restitutionA, restitutionB);
            if (fabsf(velAlongNormal) < restitutionThreshold) e = 0.0f;

            float j = -(1.0f + e) * velAlongNormal / invMassSum;
            glm::vec2 impulse = j * man.normal;

            if (bodyA) { bodyA->velocity -= impulse * invMassA; bodyA->angularVelocity -= invInertiaA * Math::Cross(rA, impulse); }
            if (bodyB) { bodyB->velocity += impulse * invMassB; bodyB->angularVelocity += invInertiaB * Math::Cross(rB, impulse); }

            velA = bodyA ? bodyA->velocity : glm::vec2(0.0f);
            velB = bodyB ? bodyB->velocity : glm::vec2(0.0f);
            wA = bodyA ? bodyA->angularVelocity : 0.0f;
            wB = bodyB ? bodyB->angularVelocity : 0.0f;
            relVel = (velB + Math::Cross(wB, rB)) - (velA + Math::Cross(wA, rA));

            glm::vec2 tangent = relVel - glm::dot(relVel, man.normal) * man.normal;
            float tangentLen = glm::length(tangent);
            if (tangentLen > 1e-6f)
            {
                tangent /= tangentLen;

                float raCrossT = Math::Cross(rA, tangent);
                float rbCrossT = Math::Cross(rB, tangent);
                float invMassSumT = invMassA + invMassB +
                    invInertiaA * raCrossT * raCrossT + invInertiaB * rbCrossT * rbCrossT;

                if (invMassSumT > 0.0f)
                {
                    float jt = -glm::dot(relVel, tangent) / invMassSumT;
                    float mu = sqrtf(frictionA * frictionB);
                    jt = std::clamp(jt, -j * mu, j * mu);

                    glm::vec2 frictionImpulse = jt * tangent;
                    if (bodyA) { bodyA->velocity -= frictionImpulse * invMassA; bodyA->angularVelocity -= invInertiaA * Math::Cross(rA, frictionImpulse); }
                    if (bodyB) { bodyB->velocity += frictionImpulse * invMassB; bodyB->angularVelocity += invInertiaB * Math::Cross(rB, frictionImpulse); }
                }
            }
        }*/
    }

    void Physics::Init() { s_activeContacts.clear(); s_activeTriggers.clear(); }
    void Physics::Shutdown() { Reset(); }

    void Physics::Reset()
    {
        s_enters.clear(); s_stays.clear(); s_exits.clear();
        s_triggerEnters.clear(); s_triggerStays.clear(); s_triggerExits.clear();
        s_activeContacts.clear();
        s_activeTriggers.clear();
    }

    void Physics::SetGravity(const glm::vec2& gravity) { s_gravity = gravity; }
    glm::vec2 Physics::GetGravity() { return s_gravity; }
    void Physics::SetVelocityIterations(int iterations) { s_velocityIterations = std::max(1, iterations); }
    void Physics::SetPositionIterations(int iterations) { s_positionIterations = std::max(0, iterations); }

    void Physics::MarkMassDirty(Entity entity)
    {
        if (entity.HasComponent<RigidbodyData>())
            entity.GetComponent<RigidbodyData>().massDirty = true;
    }

    void Physics::RecomputeMassNow(Entity entity)
    {
        if (!entity.HasComponent<RigidbodyData>()) return;
        auto& body = entity.GetComponent<RigidbodyData>();
        if (!body.useAutoMass) { body.massDirty = false; return; }

        float totalMass = 0.0f;
        float totalInertia = 0.0f;
        glm::vec2 weightedCenter{ 0.0f };

        if (entity.HasComponent<BoxColliderData>())
        {
            auto& c = entity.GetComponent<BoxColliderData>();
            float area = c.size.x * c.size.y;
            float mass = c.density * area;
            totalMass += mass;
            weightedCenter += c.offset * mass;
            totalInertia += mass * (c.size.x * c.size.x + c.size.y * c.size.y) / 12.0f
                + mass * glm::dot(c.offset, c.offset);
        }
        if (entity.HasComponent<CircleColliderData>())
        {
            auto& c = entity.GetComponent<CircleColliderData>();
            float area = 3.14159265f * c.radius * c.radius;
            float mass = c.density * area;
            totalMass += mass;
            weightedCenter += c.offset * mass;
            totalInertia += mass * 0.5f * c.radius * c.radius + mass * glm::dot(c.offset, c.offset);
        }

        if (totalMass > 0.0f)
        {
            body.mass = totalMass;
            body.centerOfMass = weightedCenter / totalMass;
            body.inertia = totalInertia;
        }
        else
        {
            body.mass = 1.0f;
            body.centerOfMass = glm::vec2(0.0f);
            body.inertia = 1.0f;
        }

        body.inverseMass = (body.bodyType == BodyType::Dynamic && body.mass > 0.0f) ? 1.0f / body.mass : 0.0f;
        body.inverseInertia = (body.bodyType == BodyType::Dynamic && !body.fixedRotation && body.inertia > 0.0f)
            ? 1.0f / body.inertia : 0.0f;
        body.massDirty = false;
    }

    void Physics::Step(float dt)
    {
        if (!Scene::Current()) return;
        Registry& registry = Scene::Current()->GetRegistry();

        // --- 1. Recompute dirty masses, integrate forces -> velocity ---
        registry.View<RigidbodyData>().each([&](entt::entity e, RigidbodyData& body)
            {
                Entity entity(&registry, e);
                if (body.massDirty) RecomputeMassNow(entity);

                if (body.bodyType != BodyType::Dynamic || !body.isAwake) return;

                body.velocity += (s_gravity * body.gravityScale + body.forceAccum * body.inverseMass) * dt;
                body.angularVelocity += body.torqueAccum * body.inverseInertia * dt;

                body.velocity *= 1.0f / (1.0f + dt * body.linearDamping);
                body.angularVelocity *= 1.0f / (1.0f + dt * body.angularDamping);

                body.forceAccum = glm::vec2(0.0f);
                body.torqueAccum = 0.0f;
            });

        // --- 2. Build shape proxies ---
        std::vector<ShapeProxy> proxies;

        auto BuildProxy = [&](entt::entity e, bool isCircle) -> ShapeProxy
            {
                Entity entity(&registry, e);
                Transform t(entity);
                ShapeProxy p;
                p.entity = entity;
                p.isCircle = isCircle;
                p.center = t.Position();
                p.rotation = t.Rotation();
                p.body = entity.HasComponent<RigidbodyData>() ? &entity.GetComponent<RigidbodyData>() : nullptr;

                if (isCircle)
                {
                    auto& c = entity.GetComponent<CircleColliderData>();
                    p.center += Math::Rotate(c.offset, p.rotation) * t.Scale();
                    p.radius = c.radius * t.Scale().x;
                    p.isTrigger = c.isTrigger; p.material = c.material; p.filter = c.filter;
                }
                else
                {
                    auto& c = entity.GetComponent<BoxColliderData>();
                    p.center += Math::Rotate(c.offset, p.rotation) * t.Scale();
                    p.halfExtents = c.size * 0.5f * t.Scale();
                    p.isTrigger = c.isTrigger; p.material = c.material; p.filter = c.filter;
                }
                return p;
            };

        registry.View<BoxColliderData>().each([&](entt::entity e, BoxColliderData&) { proxies.push_back(BuildProxy(e, false)); });
        registry.View<CircleColliderData>().each([&](entt::entity e, CircleColliderData&) { proxies.push_back(BuildProxy(e, true)); });

        // --- 3. Broad + narrow phase, wake propagation, pair collection ---
        struct PendingContact { ShapeProxy a, b; Manifold manifold; PairKey key; };
        std::vector<PendingContact> contacts;
        std::unordered_map<PairKey, PairEntities> frameContacts;
        std::unordered_map<PairKey, PairEntities> frameTriggers;

        auto IsMoving = [](const ShapeProxy& p)
            {
                if (!p.body) return false; // no rigidbody = static geometry, never "moving"
                if (p.body->bodyType == BodyType::Kinematic) return true;
                return p.body->bodyType == BodyType::Dynamic && p.body->isAwake;
            };

        for (size_t i = 0; i < proxies.size(); i++)
        {
            for (size_t j = i + 1; j < proxies.size(); j++)
            {
                ShapeProxy& a = proxies[i];
                ShapeProxy& b = proxies[j];

                bool filtered;
                if (a.filter.groupIndex != 0 && a.filter.groupIndex == b.filter.groupIndex)
                    filtered = a.filter.groupIndex < 0;
                else
                    filtered = !((a.filter.categoryBits & b.filter.maskBits) &&
                        (b.filter.categoryBits & a.filter.maskBits));
                if (filtered) continue;

                bool aDynamic = a.body && a.body->bodyType == BodyType::Dynamic;
                bool bDynamic = b.body && b.body->bodyType == BodyType::Dynamic;
                if (!aDynamic && !bDynamic) continue;

                Manifold m;
                if (a.isCircle && b.isCircle) m = CircleVsCircle(a, b);
                else if (!a.isCircle && !b.isCircle) m = BoxVsBox(a, b);
                else if (!a.isCircle && b.isCircle) m = BoxVsCircle(a, b);
                else { m = BoxVsCircle(b, a); m.normal = -m.normal; }

                if (!m.hit) continue;

                PairKey key = MakeKey(a.entity.GetValue(), b.entity.GetValue());

                if (a.isTrigger || b.isTrigger)
                {
                    frameTriggers[key] = { a.entity, b.entity };
                    continue; // sensors: overlap reporting only, no wake, no response
                }

                // Wake propagation: a sleeping Dynamic body touched by
                // something actually moving reactivates; contact with static
                // geometry never wakes anything, so resting stacks stay
                // asleep. This is a single-pass, non-island approximation -
                // a multi-body stack can take a couple of frames to fully
                // wake top-to-bottom. Proper wake islands are a v3 item.
                if (a.body && a.body->bodyType == BodyType::Dynamic && !a.body->isAwake && IsMoving(b))
                {
                    a.body->isAwake = true; a.body->sleepTime = 0.0f;
                }
                if (b.body && b.body->bodyType == BodyType::Dynamic && !b.body->isAwake && IsMoving(a))
                {
                    b.body->isAwake = true; b.body->sleepTime = 0.0f;
                }

                frameContacts[key] = { a.entity, b.entity };
                contacts.push_back({ a, b, m, key });
            }
        }

        // --- 4. Warm start: seed each point from last step's cache and apply
        //        it immediately, before iterating. ---
        for (PendingContact& c : contacts)
        {
            auto cacheIt = s_activeContacts.find(c.key);
            if (cacheIt != s_activeContacts.end() && cacheIt->second.pointCount == c.manifold.pointCount)
            {
                for (int p = 0; p < c.manifold.pointCount; p++)
                {
                    c.manifold.points[p].normalImpulse = cacheIt->second.normalImpulse[p];
                    c.manifold.points[p].tangentImpulse = cacheIt->second.tangentImpulse[p];
                }
            }
            // Mismatched or missing point count (new contact, or manifold
            // topology changed) - skip reuse, solve this contact from zero
            // this frame. Correct, just slightly less optimal for one step.

            for (int p = 0; p < c.manifold.pointCount; p++)
                ApplyWarmStart(c.a.body, c.b.body, c.a.center, c.b.center, c.manifold.normal, c.manifold.points[p]);
        }

        // --- 5. Velocity solver ---
        for (int it = 0; it < s_velocityIterations; it++)
            for (PendingContact& c : contacts)
                for (int p = 0; p < c.manifold.pointCount; p++)
                    SolveVelocityConstraint(c.a.body, c.b.body, c.a.center, c.b.center, c.manifold.normal,
                        c.manifold.points[p],
                        sqrtf(c.a.material.friction * c.b.material.friction),
                        std::max(c.a.material.restitution, c.b.material.restitution),
                        std::max(c.a.material.restitutionThreshold, c.b.material.restitutionThreshold));

        // --- 6. Integrate positions ---
        registry.View<RigidbodyData>().each([&](entt::entity e, RigidbodyData& body)
            {
                if (body.bodyType == BodyType::Static || !body.isAwake) return;
                Entity entity(&registry, e);
                Transform t(entity);
                t.SetPosition(t.Position() + body.velocity * dt);
                if (!body.fixedRotation)
                    t.SetRotation(t.Rotation() + body.angularVelocity * dt);
            });

        // --- 7. Position correction ---
        for (int it = 0; it < s_positionIterations; it++)
            for (PendingContact& c : contacts)
            {
                float invMassA = c.a.body ? c.a.body->inverseMass : 0.0f;
                float invMassB = c.b.body ? c.b.body->inverseMass : 0.0f;
                if (invMassA == 0.0f && invMassB == 0.0f) continue;

                for (int p = 0; p < c.manifold.pointCount; p++)
                    ApplyPositionalCorrection(Transform(c.a.entity), Transform(c.b.entity),
                        invMassA, invMassB, c.manifold.normal, c.manifold.points[p].penetration);
            }

        // --- 8. Sleeping - runs after solving so it reflects this step's
        //        final velocities. ---
        registry.View<RigidbodyData>().each([&](entt::entity e, RigidbodyData& body)
            {
                if (body.bodyType != BodyType::Dynamic || !body.isAwake) return;

                if (!body.allowSleep) { body.sleepTime = 0.0f; return; }

                bool underThreshold = glm::dot(body.velocity, body.velocity) < kLinearSleepToleranceSq
                    && fabsf(body.angularVelocity) < kAngularSleepTolerance;

                body.sleepTime = underThreshold ? (body.sleepTime + dt) : 0.0f;

                if (body.sleepTime >= kTimeToSleep)
                {
                    body.isAwake = false;
                    body.sleepTime = 0.0f;
                    body.velocity = glm::vec2(0.0f);
                    body.angularVelocity = 0.0f;
                }
            });

        // --- 9. Event bookkeeping + warm-start cache update ---
        s_enters.clear(); s_stays.clear(); s_exits.clear();
        s_triggerEnters.clear(); s_triggerStays.clear(); s_triggerExits.clear();

        std::unordered_map<PairKey, ContactCache> nextActiveContacts;
        nextActiveContacts.reserve(contacts.size());

        for (PendingContact& c : contacts)
        {
            // Public Contact is still single-point; report the deepest point
            // as the representative one for enter/stay.
            int deepest = 0;
            for (int p = 1; p < c.manifold.pointCount; p++)
                if (c.manifold.points[p].penetration > c.manifold.points[deepest].penetration)
                    deepest = p;

            Contact contact{ c.a.entity, c.b.entity, c.manifold.points[deepest].point, c.manifold.normal,
                -c.manifold.points[deepest].penetration };
            (s_activeContacts.count(c.key) ? s_stays : s_enters).push_back(contact);

            ContactCache cache;
            cache.a = c.a.entity; cache.b = c.b.entity;
            cache.normal = c.manifold.normal;
            cache.pointCount = c.manifold.pointCount;
            for (int p = 0; p < c.manifold.pointCount; p++)
            {
                cache.normalImpulse[p] = c.manifold.points[p].normalImpulse;
                cache.tangentImpulse[p] = c.manifold.points[p].tangentImpulse;
            }
            nextActiveContacts[c.key] = cache;
        }

        for (auto& [key, cache] : s_activeContacts)
            if (!frameContacts.count(key))
                // Bodies may have already separated/been destroyed by the time
                // this fires; point/normal aren't physically meaningful on
                // exit (same as Unity's OnCollisionExit contract) - only
                // entity identity is guaranteed.
                s_exits.push_back({ cache.a, cache.b, glm::vec2(0.0f), cache.normal, 0.0f });

        for (auto& [key, ents] : frameTriggers)
            (s_activeTriggers.count(key) ? s_triggerStays : s_triggerEnters).push_back({ ents.a, ents.b });

        for (auto& [key, ents] : s_activeTriggers)
            if (!frameTriggers.count(key))
                s_triggerExits.push_back({ ents.a, ents.b });

        s_activeContacts = std::move(nextActiveContacts);
        s_activeTriggers = std::move(frameTriggers);
    }

    const std::vector<Contact>& Physics::GetCollisionEnters() { return s_enters; }
    const std::vector<Contact>& Physics::GetCollisionStays() { return s_stays; }
    const std::vector<Contact>& Physics::GetCollisionExits() { return s_exits; }
    const std::vector<std::pair<Entity, Entity>>& Physics::GetTriggerEnters() { return s_triggerEnters; }
    const std::vector<std::pair<Entity, Entity>>& Physics::GetTriggerStays() { return s_triggerStays; }
    const std::vector<std::pair<Entity, Entity>>& Physics::GetTriggerExits() { return s_triggerExits; }

    uint32_t Physics::GetActiveBodyCount()
    {
        uint32_t count = 0;
        if (Scene::Current())
            Scene::Current()->GetRegistry().View<RigidbodyData>().each([&](RigidbodyData& b) { if (b.isAwake) count++; });
        return count;
    }

    // TODO
    bool Physics::Raycast(const glm::vec2&, const glm::vec2&, float, RaycastHit&, uint32_t) { return false; }
    std::vector<RaycastHit> Physics::RaycastAll(const glm::vec2&, const glm::vec2&, float, uint32_t) { return {}; }
    bool Physics::OverlapPoint(const glm::vec2&, Entity&, uint32_t) { return false; }
    std::vector<Entity> Physics::OverlapCircle(const glm::vec2&, float, uint32_t) { return {}; }
    std::vector<Entity> Physics::OverlapBox(const glm::vec2&, const glm::vec2&, glm::vec1, uint32_t) { return {}; }
}