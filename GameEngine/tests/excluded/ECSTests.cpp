#include <gtest/gtest.h>
#include "ECS/Entity.h"
#include "ECS/Registry.h"

using namespace Vortx;

namespace
{
    struct Health { int hp = 10; };
    struct Tag { int v = 0; };
}

TEST(Entity, DefaultEntitiesAreEqual)
{
    Entity a, b;
    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a != b);
}

TEST(Registry, CreatedEntitiesAreDistinct)
{
    Registry r;
    Entity a = r.Create();
    Entity b = r.Create();
    EXPECT_TRUE(a != b);
    EXPECT_TRUE(a == a);
}

TEST(Registry, AddGetHasRemoveComponent)
{
    Registry r;
    Entity e = r.Create();

    EXPECT_FALSE(e.HasComponent<Health>());

    Health& h = e.AddComponent<Health>();
    EXPECT_EQ(h.hp, 10);
    h.hp = 3;

    EXPECT_TRUE(e.HasComponent<Health>());
    EXPECT_EQ(e.GetComponent<Health>().hp, 3);

    e.RemoveComponent<Health>();
    EXPECT_FALSE(e.HasComponent<Health>());
}

TEST(Registry, HasComponentById)
{
    Registry r;
    Entity e = r.Create();
    e.AddComponent<Health>();

    EXPECT_TRUE(e.HasComponent(entt::type_hash<Health>::value()));
    EXPECT_FALSE(e.HasComponent(entt::type_hash<Tag>::value()));
}

TEST(Registry, DestroyRemovesEntity)
{
    Registry r;
    Entity e = r.Create();
    e.AddComponent<Health>();
    r.Destroy(e);

    EXPECT_FALSE(e.HasComponent<Health>());
    EXPECT_FALSE(e.HasComponent(entt::type_hash<Health>::value()));
}

TEST(Registry, ViewIteratesMatchingEntities)
{
    Registry r;
    for (int i = 0; i < 5; i++)
    {
        Entity e = r.Create();
        if (i % 2 == 0) e.AddComponent<Health>();
    }

    int count = 0;
    r.View<Health>().each([&](Health&) { count++; });
    EXPECT_EQ(count, 3);
}

TEST(Registry, ClearRemovesEverything)
{
    Registry r;
    Entity e = r.Create();
    e.AddComponent<Health>();
    r.Clear();

    int count = 0;
    r.View<Health>().each([&](Health&) { count++; });
    EXPECT_EQ(count, 0);
}

TEST(Registry, TwoComponentView)
{
    Registry r;
    Entity a = r.Create(); a.AddComponent<Health>(); a.AddComponent<Tag>();
    Entity b = r.Create(); b.AddComponent<Health>();

    int count = 0;
    r.View<Health, Tag>().each([&](Health&, Tag&) { count++; });
    EXPECT_EQ(count, 1);
}
