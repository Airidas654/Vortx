#include "Editor.h"
#include "EditorUI.h"
#include "EditorInputManager.h"
#include "Scene.h"
#include "PropertiesLoader.h"
#include "Scripting/ScriptSystem.h"
#include "SimpleAPI/Time.h"
#include "SimpleAPI/SceneManager.h"
#include "ProjectHub.h"
#include "Project.h"
#include "PlayModeManager.h"
#include "SerializerLoader.h"

#include "GameTest.h" 

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "json.hpp"

#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <thread>

bool Editor::isBuilding = false;
bool Editor::needsReload = false;
std::string Editor::cmakePath = "";

GLFWwindow* CreateWindow()
{
    GLFWwindow* window;

    if (!glfwInit()) 
    {
        return nullptr;
    }

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    window = glfwCreateWindow(mode->width, mode->height, "Vortx", NULL, NULL);

    if (!window)
    {
        glfwTerminate();
        return nullptr;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        return nullptr;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    //io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    return window;
}

void Editor::LoadSettings()
{
    std::filesystem::path editorPath = std::filesystem::current_path();

    if (!std::filesystem::exists(editorPath / "config.json"))
    {
        GenerateSettings();
    }

    std::ifstream in(editorPath / "config.json");

    nlohmann::json j;
    in >> j;

    in.close();

    std::string cmake = j["CMakePath"];

    if (cmake == "") {
        cmakePath = (editorPath / "CMake" / "bin" / "cmake.exe").string();
    }
    else {
        cmakePath = cmake;
    }
    
    for (uint32_t i = 0; i < cmakePath.length(); i++) 
        if (cmakePath[i] == '\\') 
            cmakePath[i] = '/';
}

void Editor::GenerateSettings()
{
    std::filesystem::path editorPath = std::filesystem::current_path();

    nlohmann::json j;
    j["CMakePath"] = "";

    std::ofstream out(editorPath / "config.json");
    out << j.dump(4);
    out.close();
}

void Editor::Compile()
{
    if (isBuilding) return;
    isBuilding = true;

    std::thread([]()
        {
            auto run = [](const std::string& cmd)
                {
#ifdef _WIN32
                    FILE* pipe = _popen(cmd.c_str(), "r");
#else
                    FILE* pipe = popen(cmd.c_str(), "r");
#endif
                    if (!pipe) return;

#ifdef _WIN32
                    _pclose(pipe);
#else
                    pclose(pipe);
#endif
                };

            std::filesystem::path path = Vortx::Project::current.path;
            path = path.parent_path();

            std::filesystem::path editorPath = std::filesystem::current_path();

            run("\"" + cmakePath + " -S \"" + path.string() + "\" -B \"" + path.string() + "/build\" -DENGINE_PATH=\""+ editorPath.string() + "\"\"");

            run("\"" + cmakePath+" --build \"" + path.string() + "/build\" --config Release\"");

            needsReload = true;
            isBuilding = false;

        }).detach();
}

void Test(Engine& engine)
{
    Scene::Current()->CreateGameObject();
    Scene::Current()->CreateGameObject();
    Transform par = Scene::Current()->CreateGameObject().GetComponent<Transform>();

    Transform chil = Scene::Current()->CreateGameObject().GetComponent<Transform>();

    chil.AddComponent<GameTest>();

    chil.SetParent(par);
    chil.GetGameObject().NameSet("Child");
}

void Setup(EditorUI& editorUI)
{
    PropertiesLoader::Reset();
    SerializerLoader::Reset();

    editorUI.SetNewSceneCallback([]() {
        SceneManager::NewScene();
    });

    editorUI.SetSaveSceneCallback([](const std::string& path) {
        SceneManager::SaveScene(path);
    });

    editorUI.SetLoadSceneCallback([](const std::string& path) {
        SceneManager::LoadScene(path);
    });
}

void Editor::Start(Engine& engine)
{
    engine.Init();

    LoadSettings();

    GLFWwindow* window = CreateWindow();

    if (!window) return;

    ProjectHub projectHub;

    EditorInputManager input(window);

    EditorUI editorUI(window, engine);

    Setup(editorUI);

    Test(engine);

    while (!glfwWindowShouldClose(window)) 
    {
        Vortx::Time::_Update();

        glfwPollEvents();

        ImGui_ImplGlfw_NewFrame();

        if (PlayModeManager::state == PlayState::Playing)
        {
            ScriptSystem::instance().Update();
        }

        if (PlayModeManager::state == PlayState::Paused && PlayModeManager::stepRequested)
        {
            PlayModeManager::stepRequested = false;
            Vortx::Time::_SetDeltaTime(1.0f/60.0f);
            ScriptSystem::instance().Update();
        }

        editorUI.NewFrame();

        glClearColor(0.07f, 0.07f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        editorUI.Render();

        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
}