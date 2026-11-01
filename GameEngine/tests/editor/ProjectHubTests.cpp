#include "TestUtils.h"
#include "ProjectHub.h"
#include "json.hpp"

using namespace TestUtils;

// Runs inside a temp working directory so HubProjects.json and the
// Assets/ProjectTemplate lookup never touch real data.
class ProjectHubTest : public ::testing::Test
{
protected:
    TempDir tmp;
    fs::path oldCwd;

    void SetUp() override { oldCwd = fs::current_path(); fs::current_path(tmp.path()); }
    void TearDown() override { fs::current_path(oldCwd); }

    fs::path MakeProject(const std::string& folder, const std::string& name)
    {
        fs::path file = tmp / folder / "project.vortx";
        WriteFile(file, nlohmann::json{ {"Name", name} }.dump());
        return file;
    }
};

TEST_F(ProjectHubTest, ConstructorSetsInstanceAndDestructorClearsIt)
{
    {
        ProjectHub hub;
        EXPECT_EQ(ProjectHub::instance, &hub);
        EXPECT_FALSE(hub.projectLoaded);
        EXPECT_TRUE(hub.projects.empty());
    }
    EXPECT_EQ(ProjectHub::instance, nullptr);
}

TEST_F(ProjectHubTest, LoadsKnownProjectsSkippingMissingAndDuplicates)
{
    fs::path p = MakeProject("game", "My Game");
    nlohmann::json hub;
    hub["Projects"] = nlohmann::json::array({
        { {"Path", p.string()} },
        { {"Path", p.string()} },
        { {"Path", (tmp / "ghost" / "project.vortx").string()} } });
    WriteFile("HubProjects.json", hub.dump());

    ProjectHub h;
    ASSERT_EQ(h.projects.size(), 1u);
    EXPECT_EQ(h.projects[0].name, "My Game");
    EXPECT_EQ(h.projects[0].path, p.string());
}

TEST_F(ProjectHubTest, CorruptHubFileYieldsEmptyList)
{
    WriteFile("HubProjects.json", "{ broken");
    ProjectHub h;
    EXPECT_TRUE(h.projects.empty());
}

TEST_F(ProjectHubTest, AddProjectRegistersAndPersists)
{
    fs::path p = MakeProject("a", "Alpha");
    {
        ProjectHub h;
        h.AddProject(p.string());
        ASSERT_EQ(h.projects.size(), 1u);
        EXPECT_EQ(h.projects[0].name, "Alpha");
    }
    ProjectHub reloaded;
    ASSERT_EQ(reloaded.projects.size(), 1u);
    EXPECT_EQ(reloaded.projects[0].path, p.string());
}

TEST_F(ProjectHubTest, AddProjectIgnoresDuplicates)
{
    fs::path p = MakeProject("a", "Alpha");
    ProjectHub h;
    h.AddProject(p.string());
    h.AddProject(p.string());
    EXPECT_EQ(h.projects.size(), 1u);
}

TEST_F(ProjectHubTest, AddProjectRejectsWrongExtension)
{
    WriteFile(tmp / "other.json", R"({"Name":"x"})");
    ProjectHub h;
    h.AddProject((tmp / "other.json").string());
    EXPECT_TRUE(h.projects.empty());
}

TEST_F(ProjectHubTest, AddProjectRejectsFileWithoutName)
{
    WriteFile(tmp / "noname" / "project.vortx", R"({"Scenes":[]})");
    ProjectHub h;
    h.AddProject((tmp / "noname" / "project.vortx").string());
    EXPECT_TRUE(h.projects.empty());
}

TEST_F(ProjectHubTest, AddProjectRejectsInvalidJson)
{
    WriteFile(tmp / "bad" / "project.vortx", "{ nope");
    ProjectHub h;
    h.AddProject((tmp / "bad" / "project.vortx").string());
    EXPECT_TRUE(h.projects.empty());
}

TEST_F(ProjectHubTest, AddProjectRejectsMissingFile)
{
    ProjectHub h;
    h.AddProject((tmp / "ghost" / "project.vortx").string());
    EXPECT_TRUE(h.projects.empty());
}

TEST_F(ProjectHubTest, SaveProjectsWritesPaths)
{
    ProjectHub h;
    h.projects.push_back({ "Name", "some/path/project.vortx" });
    h.SaveProjects();

    nlohmann::json j = nlohmann::json::parse(ReadFile("HubProjects.json"));
    ASSERT_EQ(j["Projects"].size(), 1u);
    EXPECT_EQ(j["Projects"][0]["Path"], std::string("some/path/project.vortx"));
}

TEST_F(ProjectHubTest, CreateProjectCopiesTemplateAndSetsName)
{
    WriteFile(tmp / "Assets" / "ProjectTemplate" / "project.vortx", R"({"Name":"{{PROJECT_NAME}}"})");
    WriteFile(tmp / "Assets" / "ProjectTemplate" / "CMakeLists.txt", "# template");

    fs::path target = tmp / "MyNewGame";
    ProjectHub h;
    h.CreateProject(target.string());

    ASSERT_TRUE(fs::exists(target / "CMakeLists.txt"));
    nlohmann::json j = nlohmann::json::parse(ReadFile(target / "project.vortx"));
    EXPECT_EQ(j["Name"], std::string("MyNewGame"));

    ASSERT_EQ(h.projects.size(), 1u);
    EXPECT_EQ(h.projects[0].name, "MyNewGame");
}

TEST_F(ProjectHubTest, CreateProjectDoesNotOverwriteExistingFolder)
{
    WriteFile(tmp / "Assets" / "ProjectTemplate" / "project.vortx", R"({"Name":"t"})");
    fs::path target = tmp / "Existing";
    WriteFile(target / "marker.txt", "keep me");

    ProjectHub h;
    h.CreateProject(target.string());

    EXPECT_TRUE(h.projects.empty());
    EXPECT_EQ(ReadFile(target / "marker.txt"), "keep me");
}
