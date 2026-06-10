#include "ProjectHub.h"

#include "Project.h"
#include "Core/File.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "json.hpp"

#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <unordered_set>

namespace Vortx
{
	ProjectHub* ProjectHub::instance = nullptr;

	ProjectHub::ProjectHub()
	{
        projectLoaded = false;
		ProjectHub::instance = this;

		Setup();
	}

	ProjectHub::~ProjectHub()
	{
		ProjectHub::instance = nullptr;
	}

	void ProjectHub::Setup()
	{
        if (std::filesystem::exists(std::filesystem::path(hubPath))) 
        {
            try {
                std::ifstream file(hubPath);

                nlohmann::json j;
                file >> j;

                file.close();

                projects.clear();

                std::unordered_set<std::string> been;

                for (auto i : j["Projects"]) {
                    if (std::filesystem::exists(std::filesystem::path((std::string)i["Path"])) && been.find(i["Path"]) == been.end())
                    {
                        std::string name;
                        try {
                            std::ifstream file((std::string)i["Path"]);

                            nlohmann::json j_proj;
                            file >> j_proj;

                            file.close();

                            if (!j_proj.contains("Name")) {
                                return;
                            }

                            name = j_proj["Name"];
                        }
                        catch (nlohmann::json::exception e) {
                            std::cerr << e.what() << std::endl;

                            continue;
                        }

                        ProjectShowcase obj;
                        obj.name = name;
                        obj.path = i["Path"];

                        been.emplace(i["Path"]);

                        projects.emplace_back(obj);
                    }
                }
            }
            catch (nlohmann::json::exception e) {
                std::cerr << e.what() << std::endl;

                projects.clear();
            }
        }
	}

    void ProjectHub::SaveProjects()
    {
        nlohmann::json j;

        for (auto& i : projects)
        {
            nlohmann::json obj;
            obj["Path"] = i.path;

            j["Projects"].emplace_back(obj);
        }

        std::ofstream file(hubPath);
        file << j.dump(4);
        file.close();
    }

	void ProjectHub::Render()
	{
        const ImGuiViewport* viewport =
            ImGui::GetMainViewport();

        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);

        ImGui::Begin("Projects", nullptr,
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize);

        if (ImGui::Button("Create Project"))
        {
            CreateProject();
            //OpenCreateProjectPopup();
        }

        ImGui::SameLine();

        if (ImGui::Button("Add Existing Project"))
        {
            AddProject();
        }

        ImGui::Separator();

        for (size_t i = 0; i < projects.size(); i++)
        {
            auto& project = projects[i];

            ImGui::PushID((int)i);

            ImGui::BeginChild(
                "ProjectCard",
                ImVec2(0, 70),
                true);

            ImGui::Text("%s", project.name.c_str());
            ImGui::TextDisabled("%s", project.path.c_str());

            if (ImGui::IsItemClicked() ||
                ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
            {
                LoadProject(project.path);
            }

            ImGui::EndChild();

            ImGui::PopID();
        }

        ImGui::End();
	}

    void ProjectHub::CreateProject()
    {
        std::string path = File::SaveFileDialog("");

        std::filesystem::path projectPath = std::filesystem::path(path);

        if (!projectPath.has_extension() && !projectPath.empty())
        {
            CreateProject(path);
        }
    }

    void ProjectHub::CreateProject(const std::string& path)
    {
        std::filesystem::path projectPath = std::filesystem::path(path);
        std::filesystem::path templatePath = std::filesystem::current_path() / "Assets" / "ProjectTemplate";

        if (std::filesystem::exists(projectPath))
        {
            std::cerr << "Project already exists" << std::endl;
            return;
        }

        std::string name = projectPath.filename().string();

        std::filesystem::copy(templatePath, projectPath, std::filesystem::copy_options::recursive);

        std::ifstream in(projectPath / "CMakeLists.txt");
        std::string contentCMake((std::istreambuf_iterator<char>(in)), {});
        in.close();

        in = std::ifstream(projectPath / "project.vortx");
        std::string contentJSON((std::istreambuf_iterator<char>(in)), {});
        in.close();

        auto replace = [](std::string& str, const std::string& from, const std::string& to)
            {
                size_t pos = 0;
                while ((pos = str.find(from, pos)) != std::string::npos)
                {
                    str.replace(pos, from.size(), to);
                    pos += to.size();
                }
            };

        replace(contentCMake, "{{PROJECT_NAME}}", name);
        replace(contentCMake, "{{ENGINE_PATH}}", std::filesystem::current_path().string());

        replace(contentJSON, "{{PROJECT_NAME}}", name);

        std::ofstream out(projectPath / "CMakeLists.txt");
        out << contentCMake;
        out.close();

        out = std::ofstream(projectPath / "project.vortx");
        out << contentJSON;
        out.close();

        AddProject((projectPath/"project.vortx").string());
    }

    void ProjectHub::AddProject()
    {
        std::string path = File::OpenFileDialog();

        if (path != "") {
            AddProject(path);
        }
    }

    void ProjectHub::AddProject(const std::string& path)
    {
        std::string name;

        for (auto& i : projects) {
            if (path == i.path) return;
        }

        if (std::filesystem::path(path).extension().string() != ".vortx")
        {
            return;
        }

        if (std::filesystem::exists(std::filesystem::path(path)))
        {
            try {
                std::ifstream file(path);

                nlohmann::json j;
                file >> j;

                file.close();

                if (!j.contains("Name")) {
                    return;
                }

                name = j["Name"];
            }
            catch (nlohmann::json::exception e) {
                std::cerr << e.what() << std::endl;

                return;
            }
        }

        ProjectShowcase obj;
        obj.name = name;
        obj.path = path;

        projects.emplace_back(obj);

        SaveProjects();
    }

    void ProjectHub::LoadProject(const std::string& path)
    {
        Project::LoadProject(path);

        ProjectHub::instance->projectLoaded = true;
    }
}

