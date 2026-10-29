#include "TestUtils.h"
#include "Editor.h"
#include "PlayModeManager.h"

using namespace Vortx;

class PlayModeTest : public SceneTest
{
protected:
    void SetUp() override
    {
        SceneTest::SetUp();
        PlayModeManager::state = PlayState::Editing;
        PlayModeManager::stepRequested = false;
        PlayModeManager::snapshot.clear();
        Editor::autoCompile = false;   // EnterPlay would otherwise spawn cmake
    }

    void TearDown() override
    {
        if (PlayModeManager::state != PlayState::Editing) PlayModeManager::ExitPlay();
        SceneTest::TearDown();
    }
};

TEST_F(PlayModeTest, StartsInEditingState)
{
    EXPECT_EQ(PlayModeManager::state, PlayState::Editing);
}

TEST_F(PlayModeTest, EnterPlayTakesSnapshotAndStartsPlaying)
{
    Make("A");
    PlayModeManager::EnterPlay();
    EXPECT_EQ(PlayModeManager::state, PlayState::Playing);
    EXPECT_FALSE(PlayModeManager::snapshot.empty());
}

TEST_F(PlayModeTest, ExitPlayRestoresSceneFromSnapshot)
{
    GameObject a = Make("Original");
    Tr(a).SetPosition({ 1, 2 });

    PlayModeManager::EnterPlay();
    Make("Spawned during play");
    a.NameSet("Renamed");
    Tr(a).SetPosition({ 50, 50 });

    PlayModeManager::ExitPlay();

    EXPECT_EQ(PlayModeManager::state, PlayState::Editing);
    EXPECT_FALSE(Find("Spawned during play").has_value());
    EXPECT_FALSE(Find("Renamed").has_value());
    auto restored = Find("Original");
    ASSERT_TRUE(restored.has_value());
    EXPECT_VEC2_NEAR(Tr(*restored).Position(), glm::vec2(1, 2), 1e-4f);
    EXPECT_EQ(scene().gameObjectsRegister.size(), 1u);
}

TEST_F(PlayModeTest, PauseAndResumeOnlyApplyInTheRightStates)
{
    PlayModeManager::Pause();
    EXPECT_EQ(PlayModeManager::state, PlayState::Editing);
    PlayModeManager::Resume();
    EXPECT_EQ(PlayModeManager::state, PlayState::Editing);

    PlayModeManager::EnterPlay();
    PlayModeManager::Pause();
    EXPECT_EQ(PlayModeManager::state, PlayState::Paused);
    PlayModeManager::Pause();
    EXPECT_EQ(PlayModeManager::state, PlayState::Paused);

    PlayModeManager::Resume();
    EXPECT_EQ(PlayModeManager::state, PlayState::Playing);
}

TEST_F(PlayModeTest, StepOnlyRequestedWhilePaused)
{
    PlayModeManager::Step();
    EXPECT_FALSE(PlayModeManager::stepRequested);

    PlayModeManager::EnterPlay();
    PlayModeManager::Step();
    EXPECT_FALSE(PlayModeManager::stepRequested);

    PlayModeManager::Pause();
    PlayModeManager::Step();
    EXPECT_TRUE(PlayModeManager::stepRequested);
}

TEST_F(PlayModeTest, ExitFromPausedAlsoRestores)
{
    Make("Keep");
    PlayModeManager::EnterPlay();
    Make("Extra");
    PlayModeManager::Pause();
    PlayModeManager::ExitPlay();

    EXPECT_EQ(PlayModeManager::state, PlayState::Editing);
    EXPECT_TRUE(Find("Keep").has_value());
    EXPECT_FALSE(Find("Extra").has_value());
}
