#pragma once
#include "imgui.h"

// Minimal ImGui frame with no backend, so draw code can run in a test.
// If you are on ImGui >= 1.92 and NewFrame asserts about textures, add
//   io.BackendFlags |= ImGuiBackendFlags_RendererHasTextures;
struct HeadlessImGui
{
    ImGuiContext* ctx = nullptr;

    HeadlessImGui()
    {
        IMGUI_CHECKVERSION();
        ctx = ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2(800, 600);
        io.DeltaTime = 1.0f / 60.0f;
        io.IniFilename = nullptr;
        unsigned char* px = nullptr; int w = 0, h = 0;
        io.Fonts->GetTexDataAsRGBA32(&px, &w, &h);
        ImGui::NewFrame();
        ImGui::Begin("test");
    }

    ~HeadlessImGui()
    {
        ImGui::End();
        ImGui::Render();
        ImGui::DestroyContext(ctx);
    }
};
