#pragma once

#include "EngineAPI.h"
#include "Renderer/RenderTypes.h"

#include <glm/glm.hpp>
#include <cstdint>

namespace Vortx
{
    enum class GameViewportMode { Uninitialized, Standalone, Embedded };

    class ENGINE_API GameViewport
    {
    public:
        static void InitStandalone();

        static void InitEmbedded(uint32_t initialWidth = 1280, uint32_t initialHeight = 720);

        static void Shutdown();

        static GameViewportMode GetMode();

        static uint32_t GetWidth();
        static uint32_t GetHeight();
        static float GetAspect();

        static void SetEmbeddedSize(uint32_t width, uint32_t height);

        static bool IsReady();

        static RenderTargetHandle GetRenderTarget();
        static TextureHandle GetColorTexture();

        static void BeginFrame(const glm::mat4& viewProjection,
            const glm::vec4& clearColor = glm::vec4(0.07f, 0.07f, 0.08f, 1.0f));
        static void EndFrame();

        static void Present();

    private:
        static GameViewportMode mode;
        static RenderTargetHandle target;
        static uint32_t width;
        static uint32_t height;
    };
}