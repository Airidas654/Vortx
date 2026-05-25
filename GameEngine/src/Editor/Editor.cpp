#include "Editor.h"
#include "EditorUI.h"
#include "EditorInputManager.h"
#include "Scene.h"

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
        return nullptr;

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    window = glfwCreateWindow(mode->width, mode->height, "GameEngine", NULL, NULL);

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

void Test(Engine& engine)
{
    engine.scene->CreateGameObject();
    engine.scene->CreateGameObject();
    engine.scene->CreateGameObject();
}

void Editor::Start(Engine& engine)
{
    engine.Init();

    GLFWwindow* window = CreateWindow();

    EditorInputManager input(window);

    EditorUI editorUI(window, engine);

    Test(engine);

    if (!window) return;

    double deltaTime = 0.0f;
    double lastFrame = 0.0f;

    while (!glfwWindowShouldClose(window)) 
    {
        double temp = glfwGetTime();
        deltaTime = temp - lastFrame;
        lastFrame = temp;

        glfwPollEvents();

        ImGui_ImplGlfw_NewFrame();

        editorUI.NewFrame();

        glClearColor(0.07f, 0.07f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        editorUI.Render();

        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
}