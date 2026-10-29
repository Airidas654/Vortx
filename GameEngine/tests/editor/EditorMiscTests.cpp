#include "TestUtils.h"
#include "Editor.h"
#include "PlayModeManager.h"
#include "Components/ComponentScript.h"
#include "Project.h"
#include "Scripting/ScriptBase.h"
#include "Scripting/ScriptRegistry.h"
#include "Scripting/ScriptSystem.h"

#include <GLFW/glfw3.h>
#include "SimpleAPI/KeyCode.h"

using namespace Vortx;
using namespace TestUtils;

// KeyCode must stay in lock-step with GLFW, since scripts pass it straight to GLFW.
TEST(KeyCode, MatchesGlfwConstants)
{
    EXPECT_EQ(KeyCode::KEY_SPACE, GLFW_KEY_SPACE);
    EXPECT_EQ(KeyCode::KEY_A, GLFW_KEY_A);
    EXPECT_EQ(KeyCode::KEY_Z, GLFW_KEY_Z);
    EXPECT_EQ(KeyCode::KEY_0, GLFW_KEY_0);
    EXPECT_EQ(KeyCode::KEY_ESCAPE, GLFW_KEY_ESCAPE);
    EXPECT_EQ(KeyCode::KEY_ENTER, GLFW_KEY_ENTER);
    EXPECT_EQ(KeyCode::KEY_F1, GLFW_KEY_F1);
    EXPECT_EQ(KeyCode::KEY_F25, GLFW_KEY_F25);
    EXPECT_EQ(KeyCode::KEY_KP_0, GLFW_KEY_KP_0);
    EXPECT_EQ(KeyCode::KEY_LEFT_SHIFT, GLFW_KEY_LEFT_SHIFT);
    EXPECT_EQ(KeyCode::KEY_RIGHT, GLFW_KEY_RIGHT);
    EXPECT_EQ(KeyCode::KEY_MENU, GLFW_KEY_MENU);
    EXPECT_EQ(KeyCode::KEY_LAST, GLFW_KEY_LAST);
}

TEST(Editor, WriteEnginePathsEmitsCMakeFile)
{
    TempDir tmp;
    auto old = fs::current_path();
    fs::current_path(tmp.path());

    Project::current.path = (tmp / "proj" / "project.vortx").string();
    fs::create_directories(tmp / "proj");

    Editor::WriteEnginePaths();

    std::string text = ReadFile(tmp / "proj" / "EnginePaths.cmake");
    EXPECT_NE(text.find("ENGINE_PATH"), std::string::npos);
    EXPECT_NE(text.find("ENGINE_BIN_PATH"), std::string::npos);
    EXPECT_NE(text.find((fs::current_path() / "EngineSDK").generic_string()), std::string::npos);

    fs::current_path(old);
}

namespace
{
    int g_destroyed = 0;
    class PlayScript : public ScriptBase<PlayScript>
    {
    public:
        void Destroy() override { g_destroyed++; }
    };
}

class PlayScriptsTest : public SceneTest
{
protected:
    void SetUp() override
    {
        SceneTest::SetUp();
        g_destroyed = 0;
        Editor::autoCompile = false;
        PlayModeManager::state = PlayState::Editing;
        ScriptRegistry::Instance().ResetGame();
        ScriptBase<PlayScript>::Register();
    }
    void TearDown() override
    {
        if (PlayModeManager::state != PlayState::Editing) PlayModeManager::ExitPlay();
        ScriptRegistry::Instance().ResetGame();
        SceneTest::TearDown();
    }
};

TEST_F(PlayScriptsTest, EnterPlayInstantiatesScriptsAndExitPlayDestroysThem)
{
    const std::string name = typeid(PlayScript).name();
    GameObject go = Make("Scripted");
    go.GetEntity().AddComponent<ScriptComponent>().scripts.push_back({ name, nullptr });

    PlayModeManager::EnterPlay();
    {
        auto found = Find("Scripted");
        ASSERT_TRUE(found.has_value());
        EXPECT_NE(found->GetEntity().GetComponent<ScriptComponent>().scripts[0].instance, nullptr);
    }

    PlayModeManager::ExitPlay();

    EXPECT_EQ(g_destroyed, 1);
    auto restored = Find("Scripted");
    ASSERT_TRUE(restored.has_value());
    auto& sc = restored->GetEntity().GetComponent<ScriptComponent>();
    ASSERT_EQ(sc.scripts.size(), 1u);
    EXPECT_EQ(sc.scripts[0].name, name);
    EXPECT_EQ(sc.scripts[0].instance, nullptr);
}
