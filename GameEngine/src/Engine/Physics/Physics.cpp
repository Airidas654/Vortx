#include "Physics/Physics.h"
#include "Components/ComponentRigidbody.h"
#include "Components/ComponentBoxCollider.h"
#include "Components/ComponentCircleCollider.h"
#include "SimpleAPI/Transform.h"
#include "SimpleAPI/GameObject.h"
#include "Scene.h"
#include "SimpleAPI/Math.h"

#include <algorithm>
#include <unordered_set>
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

        struct Manifold
        {
            bool hit = false;
            glm::vec2 point{ 0.0f };
            glm::vec2 normal{ 0.0f };
            float penetration = 0.0f;
        };

        Manifold CircleVsCircle(const ShapeProxy& a, const ShapeProxy& b)
        {
            Manifold m;
            glm::vec2 d = b.center - a.center;
            float rSum = a.radius + b.radius;
            float distSq = glm::dot(d, d);
            if (distSq >= rSum * rSum) return m;

            float dist = sqrtf(distSq);
            m.hit = true;
            if (dist > 1e-6f)
            {
                m.normal = d / dist;
            }
            else
            {
                m.normal = { 1.0f, 0.0f };
                dist = 0.0f;
            }
            m.penetration = rSum - dist;
            m.point = a.center + m.normal * a.radius;
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

            m.hit = true;
            m.normal = Math::Rotate(localNormal, box.rotation);
            m.penetration = penetration;
            m.point = circle.center - m.normal * circle.radius;
            return m;
        }

        Manifold BoxVsBox(const ShapeProxy& a, const ShapeProxy& b)
        {
            Manifold m;

            glm::vec2 axes[4] =
            {
                Math::Rotate({1,0}, a.rotation), Math::Rotate({0,1}, a.rotation),
                Math::Rotate({1,0}, b.rotation), Math::Rotate({0,1}, b.rotation)
            };

            glm::vec2 d = b.center - a.center;

            // Projection radius along an *explicit* axis (generalized from the
            // old closure-captured version so it can also be used for the
            // tangent-axis clamp below, not just the SAT test loop).
            auto ProjectRadius = [](const ShapeProxy& box, const glm::vec2& axis)
                {
                    glm::vec2 bx = Math::Rotate({ 1,0 }, box.rotation);
                    glm::vec2 by = Math::Rotate({ 0,1 }, box.rotation);
                    return box.halfExtents.x * fabsf(glm::dot(bx, axis)) +
                        box.halfExtents.y * fabsf(glm::dot(by, axis));
                };

            float minOverlap = FLT_MAX;
            glm::vec2 minAxis{ 0.0f };
            bool refIsA = true; // which box's face the chosen separating axis came from

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

            // The incident box's deepest corner along -normal - the actual vertex
            // poking into the reference box. This is the information the old
            // formula never looked at.
            auto GetSupportVertex = [](const ShapeProxy& box, const glm::vec2& dir) -> glm::vec2
                {
                    glm::vec2 bx = Math::Rotate({ 1,0 }, box.rotation);
                    glm::vec2 by = Math::Rotate({ 0,1 }, box.rotation);
                    glm::vec2 result = box.center;
                    result += (glm::dot(bx, dir) >= 0.0f ? box.halfExtents.x : -box.halfExtents.x) * bx;
                    result += (glm::dot(by, dir) >= 0.0f ? box.halfExtents.y : -box.halfExtents.y) * by;
                    return result;
                };

            glm::vec2 incidentVertex = GetSupportVertex(incBox, -normal);

            // Clamp that vertex's tangential position to the reference face's own
            // width, then project it onto the reference face - i.e. clip the
            // incident corner to the reference face instead of discarding its
            // tangential position entirely.
            glm::vec2 tangent(-normal.y, normal.x);

            float refTangentExtent = ProjectRadius(refBox, tangent);
            float refCenterTangent = glm::dot(refBox.center, tangent);
            float incidentTangent = glm::dot(incidentVertex, tangent);
            float clampedTangent = std::clamp(incidentTangent,
                refCenterTangent - refTangentExtent, refCenterTangent + refTangentExtent);

            float refNormalRadius = ProjectRadius(refBox, normal);
            glm::vec2 refFaceOrigin = refIsA
                ? refBox.center + normal * refNormalRadius
                : refBox.center - normal * refNormalRadius;

            m.hit = true;
            m.normal = normal;
            m.penetration = minOverlap;
            m.point = refFaceOrigin + tangent * (clampedTangent - refCenterTangent);
            return m;
        }

        glm::vec2 s_gravity{ 0.0f, -9.81f };
        int s_velocityIterations = 8;
        int s_positionIterations = 3;

        std::vector<Contact> s_enters, s_stays, s_exits;
        std::vector<std::pair<Entity, Entity>> s_triggerEnters, s_triggerStays, s_triggerExits;

        using PairKey = uint64_t;
        PairKey MakeKey(entt::entity a, entt::entity b)
        {
            uint32_t lo = std::min((uint32_t)a, (uint32_t)b);
            uint32_t hi = std::max((uint32_t)a, (uint32_t)b);
            return (uint64_t(hi) << 32) | uint64_t(lo);
        }

        std::unordered_set<PairKey> s_activeContactPairs;
        std::unordered_set<PairKey> s_activeTriggerPairs;

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

        void ResolveContact(RigidbodyData* bodyA, RigidbodyData* bodyB,
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
        }
    }

    void Physics::Init() { s_activeContactPairs.clear(); s_activeTriggerPairs.clear(); }
    void Physics::Shutdown() { Reset(); }

    void Physics::Reset()
    {
        s_enters.clear(); s_stays.clear(); s_exits.clear();
        s_triggerEnters.clear(); s_triggerStays.clear(); s_triggerExits.clear();
        s_activeContactPairs.clear();
        s_activeTriggerPairs.clear();
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

        struct PendingContact { ShapeProxy a, b; Manifold manifold; };
        std::vector<PendingContact> contacts;
        std::unordered_set<PairKey> frameContactPairs, frameTriggerPairs;

        for (size_t i = 0; i < proxies.size(); i++)
        {
            for (size_t j = i + 1; j < proxies.size(); j++)
            {
                ShapeProxy& a = proxies[i];
                ShapeProxy& b = proxies[j];

                bool filtered;
                if (a.filter.groupIndex != 0 && a.filter.groupIndex == b.filter.groupIndex)
                {
                    filtered = a.filter.groupIndex < 0;
                }
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
                    frameTriggerPairs.insert(key);
                    continue;
                }

                frameContactPairs.insert(key);
                contacts.push_back({ a, b, m });
            }
        }

        for (int it = 0; it < s_velocityIterations; it++)
        {
            for (auto& c : contacts)
            {
                ResolveContact(c.a.body, c.b.body, c.a.center, c.b.center, c.manifold,
                    c.a.material.friction, c.b.material.friction,
                    c.a.material.restitution, c.b.material.restitution,
                    std::max(c.a.material.restitutionThreshold, c.b.material.restitutionThreshold));
            }
        }

        registry.View<RigidbodyData>().each([&](entt::entity e, RigidbodyData& body)
            {
                if (body.bodyType == BodyType::Static || !body.isAwake) return;
                Entity entity(&registry, e);
                Transform t(entity);
                t.SetPosition(t.Position() + body.velocity * dt);
                if (!body.fixedRotation)
                    t.SetRotation(t.Rotation() + body.angularVelocity * dt);
            });

        for (int it = 0; it < s_positionIterations; it++)
        {
            for (auto& c : contacts)
            {
                float invMassA = c.a.body ? c.a.body->inverseMass : 0.0f;
                float invMassB = c.b.body ? c.b.body->inverseMass : 0.0f;
                if (invMassA == 0.0f && invMassB == 0.0f) continue;
                ApplyPositionalCorrection(Transform(c.a.entity), Transform(c.b.entity),
                    invMassA, invMassB, c.manifold.normal, c.manifold.penetration);
            }
        }

        s_enters.clear(); s_stays.clear(); s_exits.clear();
        s_triggerEnters.clear(); s_triggerStays.clear(); s_triggerExits.clear();

        for (auto& c : contacts)
        {
            PairKey key = MakeKey(c.a.entity.GetValue(), c.b.entity.GetValue());
            Contact contact{ c.a.entity, c.b.entity, c.manifold.point, c.manifold.normal, -c.manifold.penetration };
            (s_activeContactPairs.count(key) ? s_stays : s_enters).push_back(contact);
        }
        for (PairKey key : s_activeContactPairs)
            if (!frameContactPairs.count(key))
            {
                // Entity identity for exit events isn't reconstructible from
                // the key alone; a production version should keep a
                // key->EntityPair map alongside the set. Left as a TODO.
            }

        s_activeContactPairs = std::move(frameContactPairs);
        s_activeTriggerPairs = std::move(frameTriggerPairs);
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