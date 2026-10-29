#include "TestUtils.h"
#include "Project.h"
#include "json.hpp"

using namespace Vortx;
using namespace TestUtils;

class ProjectTest : public SceneTest
{
protected:
    TempDir tmp;
    fs::path projFile;

    void WriteProject(const nlohmann::json& j)
    {
        projFile = tmp / "project.vortx";
        WriteFile(projFile, j.dump(4));
        fs::create_directories(tmp / "Assets");
    }
};

TEST_F(ProjectTest, LoadReadsNameAndScenes)
{
    WriteProject({ {"Name", "Demo"}, {"Scenes", {"Assets/Level1.scene", "Assets/Menu.scene"}} });

    Project::LoadProject(projFile.string());

    EXPECT_EQ(Project::current.name, "Demo");
    EXPECT_EQ(Project::current.path, projFile.string());
    ASSERT_EQ(Project::current.scenes.size(), 2u);
    EXPECT_EQ(Project::current.scene_name_to_index.at("Level1"), 0u);
    EXPECT_EQ(Project::current.scene_name_to_index.at("Menu"), 1u);
}

TEST_F(ProjectTest, LoadWithoutScenesLeavesListEmpty)
{
    WriteProject({ {"Name", "Bare"} });
    Project::LoadProject(projFile.string());
    EXPECT_TRUE(Project::current.scenes.empty());
    EXPECT_TRUE(Project::current.scene_name_to_index.empty());
}

TEST_F(ProjectTest, LoadingAgainResetsSceneList)
{
    WriteProject({ {"Name", "A"}, {"Scenes", {"x/One.scene"}} });
    Project::LoadProject(projFile.string());
    ASSERT_EQ(Project::current.scenes.size(), 1u);

    WriteProject({ {"Name", "B"} });
    Project::LoadProject(projFile.string());
    EXPECT_TRUE(Project::current.scenes.empty());
    EXPECT_EQ(Project::current.name, "B");
}

TEST_F(ProjectTest, LoadOpensFirstSceneWhenItExists)
{
    // Build a scene file on disk first.
    Make("FromDisk");
    auto scenePath = (tmp / "Assets" / "Main.scene").string();
    fs::create_directories(tmp / "Assets");
    SceneManager::SaveScene(scenePath);
    SceneManager::NewScene();

    WriteProject({ {"Name", "WithScene"}, {"Scenes", {scenePath}} });
    Project::LoadProject(projFile.string());

    ASSERT_NE(SceneManager::current, nullptr);
    EXPECT_TRUE(Find("FromDisk").has_value());
}

TEST_F(ProjectTest, SettingValuesRoundTrip)
{
    WriteProject({ {"Name", "S"} });
    Project::LoadProject(projFile.string());

    EXPECT_EQ(Project::current.LoadSettingValue("missing", 42), 42);
    EXPECT_EQ(Project::current.LoadEditorSettingValue("missing", "dflt"), std::string("dflt"));

    Project::current.SaveSettingValue("Volume", 0.5);
    Project::current.SaveEditorSettingValue("AutoCompile", true);

    EXPECT_DOUBLE_EQ(Project::current.LoadSettingValue("Volume").get<double>(), 0.5);
    EXPECT_TRUE(Project::current.LoadEditorSettingValue("AutoCompile").get<bool>());
}

TEST_F(ProjectTest, SaveJSONPersistsSettings)
{
    WriteProject({ {"Name", "P"} });
    Project::LoadProject(projFile.string());
    Project::current.SaveSettingValue("Quality", "high");
    Project::current.SaveJSON();

    nlohmann::json onDisk = nlohmann::json::parse(ReadFile(projFile));
    EXPECT_EQ(onDisk["Name"], std::string("P"));
    EXPECT_EQ(onDisk["Settings"]["Quality"], std::string("high"));
}
