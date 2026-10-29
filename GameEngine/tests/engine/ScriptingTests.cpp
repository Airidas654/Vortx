#include "TestUtils.h"
#include "Components/ComponentMetadata.h"
#include "Components/ComponentScript.h"
#include "Scripting/ScriptBase.h"
#include "Scripting/ScriptRegistry.h"
#include "Scripting/ScriptSystem.h"

#include <typeinfo>

using namespace Vortx;

namespace
{
    struct Counts { int start = 0, update = 0, fixed = 0, destroy = 0, gizmos = 0, gizmosSel = 0; };
    Counts g;

    class CounterScript : public ScriptBase<CounterScript>
    {
    public:
        void Start() override { g.start++; }
        void Update() override { g.update++; }
        void FixedUpdate() override { g.fixed++; }
        void Destroy() override { g.destroy++; }
        void OnDrawGizmos() override { g.gizmos++; }
        void OnDrawGizmosSelected() override { g.gizmosSel++; }
    };

    const std::string kName = typeid(CounterScript).name();
}

class ScriptingTest : public SceneTest
{
protected:
    void SetUp() override
    {
        SceneTest::SetUp();
        g = Counts{};
        ScriptRegistry::Instance().ResetGame();
        ASSERT_TRUE(ScriptBase<CounterScript>::Register());
    }

    void TearDown() override
    {
        ScriptSystem::Instance().DestroyAll();
        ScriptRegistry::Instance().ResetGame();
        SceneTest::TearDown();
    }

    ScriptComponent& Scripts(GameObject go) { return go.GetEntity().GetComponent<ScriptComponent>(); }
};

TEST_F(ScriptingTest, RegisterMakesScriptDiscoverable)
{
    auto& all = ScriptRegistry::Instance().GetAll();
    ASSERT_EQ(all.count(kName), 1u);
    EXPECT_NE(all.at(kName).create, nullptr);
    EXPECT_NE(all.at(kName).destroy, nullptr);
}

TEST_F(ScriptingTest, CreateReturnsFreshInstances)
{
    ScriptInstance* a = ScriptRegistry::Instance().Create(kName);
    ScriptInstance* b = ScriptRegistry::Instance().Create(kName);
    ASSERT_NE(a, nullptr);
    ASSERT_NE(b, nullptr);
    EXPECT_NE(a, b);
    EXPECT_NE(dynamic_cast<CounterScript*>(a), nullptr);
    ScriptRegistry::Instance().GetAll().at(kName).destroy(a);
    ScriptRegistry::Instance().GetAll().at(kName).destroy(b);
}

TEST_F(ScriptingTest, CreateUnknownReturnsNull)
{
    EXPECT_EQ(ScriptRegistry::Instance().Create("nope"), nullptr);
}

TEST_F(ScriptingTest, ResetGameClearsRegistry)
{
    ScriptRegistry::Instance().ResetGame();
    EXPECT_TRUE(ScriptRegistry::Instance().GetAll().empty());
}

TEST_F(ScriptingTest, ScriptBaseDefaultHooksAreNoOps)
{
    struct Plain : ScriptBase<Plain> {};
    Plain p;
    p.Start(); p.Update(); p.FixedUpdate(); p.Destroy(); p.OnDrawGizmos(); p.OnDrawGizmosSelected();
    SUCCEED();
}

TEST_F(ScriptingTest, AddScriptCreatesComponentAndInstance)
{
    GameObject go = Make();
    Entity e = go.GetEntity();
    ScriptSystem::Instance().AddScript(e, kName);

    ASSERT_TRUE(e.HasComponent<ScriptComponent>());
    auto& sc = Scripts(go);
    ASSERT_EQ(sc.scripts.size(), 1u);
    EXPECT_EQ(sc.scripts[0].name, kName);
    ASSERT_NE(sc.scripts[0].instance, nullptr);
    EXPECT_TRUE(sc.scripts[0].instance->gameObject.GetEntity() == e);
    EXPECT_FALSE(sc.scripts[0].instance->_started);
}

TEST_F(ScriptingTest, UpdateCallsStartFirstThenUpdate)
{
    GameObject go = Make();
    Entity e = go.GetEntity();
    ScriptSystem::Instance().AddScript(e, kName);

    ScriptSystem::Instance().Update();
    EXPECT_EQ(g.start, 1);
    EXPECT_EQ(g.update, 0);

    ScriptSystem::Instance().Update();
    ScriptSystem::Instance().Update();
    EXPECT_EQ(g.start, 1);
    EXPECT_EQ(g.update, 2);
}

TEST_F(ScriptingTest, FixedUpdateCallsHook)
{
    GameObject go = Make();
    Entity e = go.GetEntity();
    ScriptSystem::Instance().AddScript(e, kName);

    ScriptSystem::Instance().FixedUpdate();
    ScriptSystem::Instance().FixedUpdate();
    EXPECT_EQ(g.fixed, 2);
}

TEST_F(ScriptingTest, SceneUpdateCallsSelectedHookOnlyForSelection)
{
    GameObject a = Make();
    GameObject b = Make();
    Entity ea = a.GetEntity(), eb = b.GetEntity();
    ScriptSystem::Instance().AddScript(ea, kName);
    ScriptSystem::Instance().AddScript(eb, kName);

    ScriptSystem::Instance().SceneUpdate(a.IDGet());

    EXPECT_EQ(g.gizmos, 2);
    EXPECT_EQ(g.gizmosSel, 1);
}

TEST_F(ScriptingTest, DestroyAllCallsDestroyAndNullsInstances)
{
    GameObject go = Make();
    Entity e = go.GetEntity();
    ScriptSystem::Instance().AddScript(e, kName);

    ScriptSystem::Instance().DestroyAll();

    EXPECT_EQ(g.destroy, 1);
    EXPECT_EQ(Scripts(go).scripts[0].instance, nullptr);
    ScriptSystem::Instance().DestroyAll();   // second call is a no-op
    EXPECT_EQ(g.destroy, 1);
}

TEST_F(ScriptingTest, CreateAllInstantiatesMissingInstancesOnly)
{
    GameObject go = Make();
    go.GetEntity().AddComponent<ScriptComponent>().scripts.push_back({ kName, nullptr });

    ScriptSystem::Instance().CreateAll();
    ScriptInstance* first = Scripts(go).scripts[0].instance;
    ASSERT_NE(first, nullptr);

    ScriptSystem::Instance().CreateAll();
    EXPECT_EQ(Scripts(go).scripts[0].instance, first);
}

TEST_F(ScriptingTest, CreateAllSkipsUnknownScriptNames)
{
    GameObject go = Make();
    go.GetEntity().AddComponent<ScriptComponent>().scripts.push_back({ "ghost", nullptr });
    ScriptSystem::Instance().CreateAll();
    EXPECT_EQ(Scripts(go).scripts[0].instance, nullptr);
}

TEST_F(ScriptingTest, RemoveScriptDestroysInstanceAndDropsEmptyComponent)
{
    GameObject go = Make();
    Entity e = go.GetEntity();
    ScriptSystem::Instance().AddScript(e, kName);

    ScriptSystem::Instance().RemoveScript(e, kName);

    EXPECT_EQ(g.destroy, 1);
    EXPECT_FALSE(e.HasComponent<ScriptComponent>());
}

TEST_F(ScriptingTest, RemoveScriptKeepsComponentWhenOthersRemain)
{
    GameObject go = Make();
    Entity e = go.GetEntity();
    ScriptSystem::Instance().AddScript(e, kName);
    ScriptSystem::Instance().AddScript(e, kName);

    ScriptSystem::Instance().RemoveScript(e, kName);

    ASSERT_TRUE(e.HasComponent<ScriptComponent>());
    EXPECT_EQ(Scripts(go).scripts.size(), 1u);
}

TEST_F(ScriptingTest, RemoveUnknownScriptOrMissingComponentIsNoOp)
{
    GameObject go = Make();
    Entity e = go.GetEntity();
    ScriptSystem::Instance().RemoveScript(e, kName);   // no ScriptComponent at all
    ScriptSystem::Instance().AddScript(e, kName);
    ScriptSystem::Instance().RemoveScript(e, "other");
    EXPECT_EQ(Scripts(go).scripts.size(), 1u);
}
