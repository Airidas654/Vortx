#include "Editor.h"
#include "EditorUI.h"
#include "EditorInputManager.h"
#include "Scene.h"
#include "PropertiesLoader.h"
#include "Scripting/ScriptSystem.h"
#include "SimpleAPI/Time.h"
#include "ProjectHub.h"

#include "GameTest.h" 

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <iostream>

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

void LoadScene()
{
    Scene::current = std::unique_ptr<Scene>(new Scene());

    PropertiesLoader::Reset();
}

void Test(Engine& engine)
{
    LoadScene();

    Scene::current->CreateGameObject();
    Scene::current->CreateGameObject();
    Transform par = Scene::current->CreateGameObject().GetComponent<Transform>();

    Transform chil = Scene::current->CreateGameObject().GetComponent<Transform>();

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