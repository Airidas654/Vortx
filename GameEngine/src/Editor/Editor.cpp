#include "Editor.h"
#include "EditorUI.h"
#include "EditorInputManager.h"
#include "Scene.h"
#include "PropertiesLoader.h"
#include "Scripting/ScriptSystem.h"
#include "SimpleAPI/Time.h"
#include "SimpleAPI/SceneManager.h"
#include "SimpleAPI/SpriteRenderer.h"
#include "SimpleAPI/Camera.h"
#include "ProjectHub.h"
#include "Project.h"
#include "PlayModeManager.h"
#include "SerializerLoader.h"
#include "Scripting\ScriptRegistry.h"
#include "Assets/AudioAsset.h"
#include "Assets/FontAsset.h"
#include "Assets/TextureAsset.h"
#include "Renderer/OpenGLRenderDevice.h"
#include "SimpleAPI\Renderer2D.h"

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

using namespace Vortx;

bool Editor::isBuilding = false;
bool Editor::needsReload = false;
bool Editor::autoCompile = false;
std::string Editor::cmakePath = "";
DynamicLibrary Editor::gameLib;

GLFWwindow* CreateWindow()
{
    GLFWwindow* window;

    if (!glfwInit())
    {
        return nullptr;
    }

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    OpenGLRenderDevice::PrepareGLFWHints();

    window = glfwCreateWindow(mode->width, mode->height, "Vortx", NULL, NULL);

    if (!window)
    {
        glfwTerminate();
        return nullptr;
    }

    auto device = std::make_unique<OpenGLRenderDevice>();
    if (!Renderer2D::Init(std::move(device), window)) return nullptr;

    /*glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        return nullptr;
    }*/

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

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
        auto run = [](const std::string& cmd) -> bool
        {
            
#ifdef _WIN32
            std::string fullCmd = "cmd /C \"" + cmd + "\"";
            FILE* pipe = _popen(fullCmd.c_str(), "r");
#else
            FILE* pipe = popen(cmd.c_str(), "r");
#endif
            if (!pipe)
            {
                std::cerr << "Failed to open pipe for: " + cmd << std::endl;
                
                return false;
            }

            char buffer[256];
            while (fgets(buffer, sizeof(buffer), pipe))
            {
                std::cout << buffer << std::endl;
            }

#ifdef _WIN32
            int exitCode = _pclose(pipe);
#else
            int status = pclose(pipe);
            int exitCode = WEXITSTATUS(status);
#endif
            return exitCode == 0;
        };

        std::filesystem::path path = std::filesystem::path(Vortx::Project::current.path).parent_path();
        std::filesystem::path editorPath = std::filesystem::current_path();

        std::string configCmd =
            "\"" + cmakePath + "\" -S \"" + path.string() +
            "\" -B \"" + path.string() + "/build\" "+
            "-DENGINE_BIN_PATH=\"" + editorPath.string() + "\" " +
            "-DENGINE_PATH=\"" + "C:/Users/Airidas/Desktop/GameEngine/GameEngine" + '\"';

        std::string buildCmd =
            "\"" + cmakePath + "\" --build \"" + path.string() +
            "/build\" --config "+
#ifdef _DEBUG
            "Debug";
#else
            "Release";
#endif

        bool configOk = run(configCmd);
        bool buildOk = configOk && run(buildCmd);

        needsReload = buildOk;
        isBuilding = false;
    }).detach();
}

void Test(Engine& engine)
{
    GameObject fir = Scene::Current()->CreateGameObject();
    fir.AddComponent<Camera>();
    Scene::Current()->CreateGameObject();
    Transform par = Scene::Current()->CreateGameObject().GetComponent<Transform>();

    Transform chil = Scene::Current()->CreateGameObject().GetComponent<Transform>();

    chil.AddComponent<SpriteRenderer>();

    //chil.AddComponent<GameTest>();

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

        if (!isBuilding && needsReload)
        {
            needsReload = false;

            ScriptSystem::Instance().DestroyAll();

            ScriptRegistry::Instance().ResetGame();

            if (gameLib.IsLoaded())
                gameLib.Unload();

            std::filesystem::path gameLibPathDLL = Project::current.path;
            std::filesystem::path gameLibPathPDB;

#ifdef _DEBUG
            gameLibPathPDB = gameLibPathDLL.parent_path() / "build" / "Debug" / "Game.pdb";
            gameLibPathDLL = gameLibPathDLL.parent_path() / "build" / "Debug" / "Game.dll";
#else
            gameLibPathPDB = gameLibPathDLL.parent_path() / "build" / "Release" / "Game.pdb";
            gameLibPathDLL = gameLibPathDLL.parent_path() / "build" / "Release" / "Game.dll";
#endif

            std::filesystem::path gameLibPathDLLTemp = gameLibPathDLL.parent_path() / "GameLoaded.dll";
            std::filesystem::path gameLibPathPDBTemp = gameLibPathDLL.parent_path() / "GameLoaded.pdb";

            std::filesystem::copy_file(
                gameLibPathDLL,
                gameLibPathDLLTemp,
                std::filesystem::copy_options::overwrite_existing);

            std::filesystem::copy_file(
                gameLibPathPDB,
                gameLibPathPDBTemp,
                std::filesystem::copy_options::overwrite_existing);

            if (!gameLib.Load(gameLibPathDLLTemp.string()))
                std::cerr << "Failed to reload Game.dll\n";
        }

        if (PlayModeManager::state == PlayState::Playing)
        {
            ScriptSystem::Instance().Update();
        }

        if (PlayModeManager::state == PlayState::Paused && PlayModeManager::stepRequested)
        {
            PlayModeManager::stepRequested = false;
            Vortx::Time::_SetDeltaTime(1.0f / 60.0f);
            ScriptSystem::Instance().Update();
        }

        Renderer2D::NewFrame();

        glm::mat4 viewProjMatrix(0.25f, 0,0,0,0,0.333f,0,0,0,0, -0.02f, 0,0,0, -1.202f, 1);

        Renderer2D::BeginScene(viewProjMatrix);
        Renderer2D::EndScene();

        editorUI.TryDrawScene();

        //Renderer2D::BeginScene(viewProjMatrix, glm::vec4(0.07f, 0.07f, 0.08f, 1.0f), editorUI.GetSceneRenderTarget());
        ////DrawSceneSprites();   // whatever iterates your ECS and calls Renderer2D::DrawSprite
        //Renderer2D::EndScene();

        Renderer2D::BeginScene(viewProjMatrix, glm::vec4(0.07f, 0.07f, 0.08f, 1.0f), editorUI.GetGameRenderTarget());
        //DrawSceneSprites();
        Renderer2D::EndScene();

        Renderer2D::BindBackbuffer();

        editorUI.NewFrame();

        ScriptSystem::Instance().SceneUpdate(editorUI.GetSelected());

        editorUI.Render();

        Renderer2D::Present();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
}