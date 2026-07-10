#pragma once

#include "AssetID.h"
#include "Engine.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <string>
#include <memory>
#include <functional>
#include <unordered_set>

using namespace Vortx;

//using ImportModelCallback = std::function<void(const std::string& path)>;
//using ImportTextureCallback = std::function<AssetID(const std::string& path, bool sRGB)>;
using SaveSceneCallback = std::function<void(const std::string& path)>;
using LoadSceneCallback = std::function<void(const std::string& path)>;
using NewSceneCallback = std::function<void()>;

class EditorUI {
public:
    EditorUI(GLFWwindow* window, Engine& engine);
    ~EditorUI();

    void NewFrame();
    void Render();
    void Shutdown();

    /* void SetImportModelCallback(ImportModelCallback cb) { importModel = cb; }
        void SetImportTextureCallback(ImportTextureCallback cb) { importTexture = cb; }*/
        void SetSaveSceneCallback(SaveSceneCallback cb) { onSaveScene = cb; }
        void SetLoadSceneCallback(LoadSceneCallback cb) { onLoadScene = cb; }
        void SetNewSceneCallback(NewSceneCallback cb) { onNewScene = cb; }

    AssetID GetSelected() const { return selectedNode; }

    void ResetSelected() { ChangeSelected(AssetID(0)); };

private:

    bool sceneWindowOpen = true;
    bool propertiesWindowOpen = true;
    bool consoleWindowOpen = true;
    bool assetsWindowOpen = true;

    bool consoleSelectMode = false;
    std::string consoleCachedText;
    size_t consoleCachedLineCount = 0;

    std::string currentAssetFolder;
    std::unordered_set<Vortx::AssetID> previewAcquired;

    float cellSize = 100;

    void RenderEditorView();
    void DrawMenuBar();
    void RenderDockspace();
    void DrawScenePanel();
    void RenderSceneNode(Vortx::Transform& transform, uint32_t index);
    void DrawPropertiesPanel();
    void DrawConsolePanel();
    void DrawAssetsPanel();

    void DrawToolbar();
    /*void DrawViewportStatusBar();
    void DrawRenderSettingsSection();

    void RaytraceImageSave();

    void DrawCameraTransformSection();
    void DrawCameraSceneSection();
    void DrawCameraSection();
    void DrawTransformSection(Model* model);
    void DrawMaterialSection(Model* model);
    void DrawTextureSlot(const char* label, std::function<void(AssetID)> texture_replace, AssetID texID, bool needSRGB, const char* hint);

    std::string OpenFileDialog(const char* filterDesc, const char* filterPattern);
    std::string SaveFileDialog(const char* filterDesc, const char* filterPattern, const char* defaultExt);
    */
    void ChangeSelected(const AssetID& entity);

    GLFWwindow* window;
    Engine& engine;

    //std::unique_ptr<Scene>& scene;
    //RayTracing* raytracing;
    //EditorController* editor;

    AssetID selectedNode;

    /*
    glm::vec3 cachedTranslation{ 0.0f };
    glm::vec3 cachedRotation{ 0.0f };
    glm::vec3 cachedScale{ 1.0f };

    int uiRayBounces = 3;
    int uiShadowBounces = 1;
    int uiBatchSize = 1;
    int uiTargetSamples = 512;
    float uiExposure = 1.0f;
    bool uiApplyGamma = false;
    glm::ivec2 uiresolution = { 1280, 720 };
    float uifov = 45;

    bool sectionTransformOpen = true;
    bool sectionMaterialOpen = true;
    bool sectionRenderOpen = true;

    ImportModelCallback importModel;
    ImportTextureCallback importTexture;*/
    SaveSceneCallback onSaveScene;
    LoadSceneCallback onLoadScene;
    NewSceneCallback onNewScene;
};
