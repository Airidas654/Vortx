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

#include "GameTest.h" 

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

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

std::string Editor::FindCMake()
{
#ifdef _WIN32
    // check common install locations
    std::vector<std::string> candidates = {
        "C:/Program Files/CMake/bin/cmake.exe",
        "C:/Program Files (x86)/CMake/bin/cmake.exe",
        "C:/Program Files/Microsoft Visual Studio/2022/Community/Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/cmake.exe"
    };

    for (const auto& path : candidates)
        if (std::filesystem::exists(path))
            return "\"" + path + "\"";

    // fallback — maybe it's in PATH after all
    return "cmake";
#else
    return "cmake";
#endif
}

void Editor::Compile()
{
    if (isBuilding) return;
    isBuilding = true;
    //buildLog = "";

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

                    char buffer[256];
                    //while (fgets(buffer, sizeof(buffer), pipe))
                        //buildLog += buffer;   // captured for display

#ifdef _WIN32
                    _pclose(pipe);
#else
                    pclose(pipe);
#endif
                };

            std::filesystem::path path = Vortx::Project::current.path;
            path = path.parent_path();

            if (cmakePath == "") {
                cmakePath = FindCMake();
            }

            // configure — this is what regenerates RegisterScripts.cpp
            run(cmakePath+" -S " + path.string() + " -B " + path.string() + "/build");

            // build
            run(cmakePath+" --build " + path.string() + "/build --config Release");

            // reload on the main thread next frame
            // use a flag so OpenGL context isn't touched from this thread
            needsReload = true;
            isBuilding = false;

        }).detach();
}

void Test(Engine& engine)
{
    PropertiesLoader::Reset();

    Scene::Current()->CreateGameObject();
    Scene::Current()->CreateGameObject();
    Transform par = Scene::Current()->CreateGameObject().GetComponent<Transform>();

    Transform chil = Scene::Current()->CreateGameObject().GetComponent<Transform>();

    chil.AddComponent<GameTest>();

    chil.SetParent(par);
    chil.GetGameObject().NameSet("Child");
}

void Editor::Start(Engine& engine)
{
    engine.Init();

    GLFWwindow* window = CreateWindow();

    if (!window) return;

    ProjectHub projectHub;

    EditorInputManager input(window);

    EditorUI editorUI(window, engine);

    Test(engine);

    while (!glfwWindowShouldClose(window)) 
    {
        Vortx::Time::Update();

        glfwPollEvents();

        ImGui_ImplGlfw_NewFrame();

        ScriptSystem::instance().Update();

        editorUI.NewFrame();

        glClearColor(0.07f, 0.07f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        editorUI.Render();

        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
}