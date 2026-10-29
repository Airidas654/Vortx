#include "TestUtils.h"
#include "HeadlessImGui.h"
#include "AssetPropertiesLoader.h"
#include "GizmosManager.h"
#include "EditorController.h"
#include "ProjectHub.h"
#include "PropertiesLoader.h"
#include "Core/AssetDatabase.h"
#include "SimpleAPI/BoxCollider.h"
#include "Components/ComponentScript.h"
#include "SimpleAPI/Camera.h"
#include "SimpleAPI/CircleCollider.h"
#include "SimpleAPI/Gizmos.h"
#include "SimpleAPI/Rigidbody.h"
#include "SimpleAPI/SpriteRenderer.h"

using namespace Vortx;
using namespace TestUtils;

TEST(GizmosManagerDraw, ConsumesQueuedCommandsAndEmitsGeometry)
{
    HeadlessImGui imgui;
    ImDrawList* dl = ImGui::GetWindowDrawList();

    EditorController controller;
    GizmosManager manager;                       // enables Gizmos
    manager.UpdateParams(controller, { 0, 0 }, { 800, 600 });

    Gizmos::Clear();
    Gizmos::color = glm::u8vec4(255, 0, 0, 255);
    Gizmos::DrawCircle({ 0, 0 }, 1.0f, 2.0f);
    Gizmos::DrawLine({ -1, -1 }, { 1, 1 }, 2.0f);
    Gizmos::DrawBox({ 0, 0 }, { 2, 2 }, 0.3f, 2.0f);
    ASSERT_EQ(Gizmos::commands.size(), 3u);

    int before = dl->VtxBuffer.Size;
    manager.Draw(dl);

    EXPECT_GT(dl->VtxBuffer.Size, before);
    EXPECT_TRUE(Gizmos::commands.empty());       // Draw clears the queue
    Gizmos::enabled = false;
}

TEST(GizmosManagerDraw, EmptyQueueDrawsNothing)
{
    HeadlessImGui imgui;
    ImDrawList* dl = ImGui::GetWindowDrawList();
    GizmosManager manager;
    Gizmos::Clear();
    int before = dl->VtxBuffer.Size;
    manager.Draw(dl);
    EXPECT_EQ(dl->VtxBuffer.Size, before);
    Gizmos::enabled = false;
}

class PropertiesDrawTest : public SceneTest
{
protected:
    void SetUp() override { SceneTest::SetUp(); PropertiesLoader::Reset(); AssetPropertiesLoader::Reset(); }
};

TEST_F(PropertiesDrawTest, DrawingEveryComponentEditorDoesNotCrash)
{
    GameObject go = Make("All");
    go.AddComponent<SpriteRenderer>();
    go.AddComponent<Camera>();
    go.AddComponent<BoxCollider>();
    go.AddComponent<CircleCollider>();
    go.AddComponent<Rigidbody>();
    go.GetEntity().AddComponent<ScriptComponent>();

    HeadlessImGui imgui;
    PropertiesLoader::DrawPropertiesPanel(go.IDGet());
    SUCCEED();
}

TEST_F(PropertiesDrawTest, UnknownSelectionOrNoSceneReturnsEarly)
{
    PropertiesLoader::DrawPropertiesPanel(AssetID(123, 456));   // no ImGui frame needed
    SceneManager::current.reset();
    PropertiesLoader::DrawPropertiesPanel(AssetID(0));
    SUCCEED();
}

TEST_F(PropertiesDrawTest, DrawingMissingAssetShowsPlaceholder)
{
    HeadlessImGui imgui;
    AssetPropertiesLoader::DrawPropertiesPanel(AssetID(9, 9), "gone.png");
    SUCCEED();
}

TEST_F(PropertiesDrawTest, DrawingKnownAssetsDoesNotCrash)
{
    TempDir tmp;
    WriteFile(tmp / "Assets" / "f.ttf", "x");
    WriteFile(tmp / "Assets" / "t.png", "x");
    AssetDatabase::Refresh((tmp / "Assets").string());

    AssetID font(0), tex(0);
    ASSERT_TRUE(AssetDatabase::TryGetGuid((tmp / "Assets" / "f.ttf").string(), font));
    ASSERT_TRUE(AssetDatabase::TryGetGuid((tmp / "Assets" / "t.png").string(), tex));

    HeadlessImGui imgui;
    AssetPropertiesLoader::DrawPropertiesPanel(font, "f.ttf");   // no custom editor registered
    AssetPropertiesLoader::DrawPropertiesPanel(tex, "t.png");    // texture import settings + Save
    SUCCEED();
}

TEST(ProjectHubRender, RendersProjectCardsWithoutCrashing)
{
    TempDir tmp;
    auto old = fs::current_path();
    fs::current_path(tmp.path());
    {
        ProjectHub hub;
        hub.projects.push_back({ "One", "a/project.vortx" });
        hub.projects.push_back({ "Two", "b/project.vortx" });
        HeadlessImGui imgui;
        hub.Render();
        EXPECT_FALSE(hub.projectLoaded);   // nothing was clicked
    }
    fs::current_path(old);
}
