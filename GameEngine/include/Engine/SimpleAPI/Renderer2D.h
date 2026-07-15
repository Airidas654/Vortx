#pragma once

#include "EngineAPI.h"
#include "Renderer/IRenderDevice.h"

#include <glm/glm.hpp>
#include <memory>
#include <cstdint>

namespace Vortx
{
    namespace RenderLayer
    {
        constexpr int32_t Background = 0;
        constexpr int32_t World = 1000;
        constexpr int32_t Effects = 2000;
        constexpr int32_t UI = 3000;
    }

    class ENGINE_API Renderer2D
    {
    public:
        static bool Init(std::unique_ptr<IRenderDevice> device, GLFWwindow* window);
        static void Shutdown();

        static void OnResize(uint32_t width, uint32_t height);

        static void NewFrame();

        static void BeginScene(const glm::mat4& viewProjection, const glm::vec4& clearColor = glm::vec4(0.07f, 0.07f, 0.08f, 1.0f), RenderTargetHandle target = {});

        static void EndScene();

        static void BindBackbuffer();

        static void Present();

        static void DrawQuad(const glm::vec2& position, const glm::vec2& size, glm::vec1 rotation, const glm::vec4& color,
            int32_t layer = RenderLayer::World, int32_t orderInLayer = 0);

        static void DrawSprite(const glm::vec2& position, const glm::vec2& size, glm::vec1 rotation, TextureHandle texture, 
            const glm::vec4& tint = glm::vec4(1.0f), int32_t layer = RenderLayer::World, int32_t orderInLayer = 0);

        static TextureHandle CreateTexture(const TextureDesc& desc);
        static void DestroyTexture(TextureHandle handle);

        static RenderTargetHandle CreateRenderTarget(uint32_t width, uint32_t height);
        static void ResizeRenderTarget(RenderTargetHandle target, uint32_t width, uint32_t height);
        static void DestroyRenderTarget(RenderTargetHandle target);
        static TextureHandle GetRenderTargetTexture(RenderTargetHandle target);

        static void* GetNativeTextureHandle(TextureHandle texture);

        static IRenderDevice* GetDevice();
    };
}