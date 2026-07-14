#pragma once

#include "EngineAPI.h"
#include "Renderer/IRenderDevice.h"

#include <glm/glm.hpp>
#include <memory>
#include <cstdint>

namespace Vortx
{
    // Coarse draw-order buckets, spaced out so each has room for a
    // per-sprite "order in layer" without colliding with its neighbor.
    // Add game-specific layers between these, or ignore this and pass
    // plain integers to DrawQuad/DrawSprite - it's just an int underneath.
    namespace RenderLayer
    {
        constexpr int32_t Background = 0;
        constexpr int32_t World = 1000;
        constexpr int32_t Effects = 2000;
        constexpr int32_t UI = 3000;
    }

    // The API the rest of the engine/editor actually calls. Everything
    // here is 2D-specific (quads, sprites, layered batching - no meshes,
    // no 3D lighting) and it only ever talks to IRenderDevice. Swapping GL
    // for WebGPU later means writing a new IRenderDevice and changing the
    // one call to Init() below - nothing in here changes.
    class ENGINE_API Renderer2D
    {
    public:
        static bool Init(std::unique_ptr<IRenderDevice> device, GLFWwindow* window);
        static void Shutdown();

        static void OnResize(uint32_t width, uint32_t height);

        // Call ONCE per real frame, before any BeginScene calls - this is
        // the frame-level acquire step (matters more once a backend needs
        // one, e.g. WebGPU's per-frame surface texture).
        static void NewFrame();

        // viewProjection = camera projection * view, already combined.
        // target defaults to the window's own backbuffer; pass a render
        // target (see CreateRenderTarget) to draw into an off-screen
        // texture instead - e.g. one BeginScene/EndScene pair per editor
        // viewport panel, each with its own target and camera.
        static void BeginScene(const glm::mat4& viewProjection,
            const glm::vec4& clearColor = glm::vec4(0.07f, 0.07f, 0.08f, 1.0f),
            RenderTargetHandle target = {});

        // Sorts everything drawn this scene by (layer, orderInLayer) and
        // flushes it in that order, THEN builds/sends batches. Does NOT
        // present.
        static void EndScene();

        // Rebinds the default backbuffer. Call this before letting ImGui
        // render (editorUI.Render()) if your last BeginScene targeted an
        // off-screen render target - otherwise ImGui ends up drawing into
        // that texture instead of the window, and the screen stays blank.
        static void BindBackbuffer();

        // Call ONCE per real frame, after everything (including ImGui) has
        // been drawn to the backbuffer.
        static void Present();

        // Solid-color quad, centered at position, rotation in radians.
        static void DrawQuad(const glm::vec2& position, const glm::vec2& size, glm::vec1 rotation, const glm::vec4& color,
            int32_t layer = RenderLayer::World, int32_t orderInLayer = 0);

        // Textured quad ("sprite"). tint multiplies the texture (white = untouched).
        static void DrawSprite(const glm::vec2& position, const glm::vec2& size, glm::vec1 rotation,
            TextureHandle texture, const glm::vec4& tint = glm::vec4(1.0f),
            int32_t layer = RenderLayer::World, int32_t orderInLayer = 0);

        static TextureHandle CreateTexture(const TextureDesc& desc);
        static void DestroyTexture(TextureHandle handle);

        // Passthroughs for editor viewport panels ("Scene", "Game View", ...).
        static RenderTargetHandle CreateRenderTarget(uint32_t width, uint32_t height);
        static void ResizeRenderTarget(RenderTargetHandle target, uint32_t width, uint32_t height);
        static void DestroyRenderTarget(RenderTargetHandle target);
        static TextureHandle GetRenderTargetTexture(RenderTargetHandle target);

        // Native handle for ImGui::Image - see IRenderDevice::GetNativeTextureHandle.
        static void* GetNativeTextureHandle(TextureHandle texture);

        // Escape hatch for editor-only, backend-specific setup (e.g. handing
        // the raw device to ImGui's opengl3/wgpu backend init call).
        static IRenderDevice* GetDevice();
    };
}