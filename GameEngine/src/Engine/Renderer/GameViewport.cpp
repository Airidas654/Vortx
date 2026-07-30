#include "Renderer/GameViewport.h"
#include "SimpleAPI/Renderer2D.h"
#include "Core/WindowManager.h"

namespace Vortx
{
    GameViewportMode GameViewport::mode = GameViewportMode::Uninitialized;
    RenderTargetHandle GameViewport::target;
    uint32_t GameViewport::width = 0;
    uint32_t GameViewport::height = 0;

    void GameViewport::InitStandalone()
    {
        mode = GameViewportMode::Standalone;
        width = WindowManager::GetFramebufferWidth();
        height = WindowManager::GetFramebufferHeight();

        WindowManager::AddResizeCallback([](uint32_t w, uint32_t h) {
            width = w;
            height = h;
            Renderer2D::OnResize(w, h);
            });
    }

    void GameViewport::InitEmbedded(uint32_t initialWidth, uint32_t initialHeight)
    {
        mode = GameViewportMode::Embedded;
        width = initialWidth;
        height = initialHeight;
        target = Renderer2D::CreateRenderTarget(width, height);
    }

    void GameViewport::Shutdown()
    {
        if (mode == GameViewportMode::Embedded && target.IsValid())
            Renderer2D::DestroyRenderTarget(target);

        target = {};
        width = 0;
        height = 0;
        mode = GameViewportMode::Uninitialized;
    }

    GameViewportMode GameViewport::GetMode() { return mode; }
    uint32_t GameViewport::GetWidth() { return width; }
    uint32_t GameViewport::GetHeight() { return height; }

    float GameViewport::GetAspect()
    {
        return height > 0 ? (float)width / (float)height : 1.0f;
    }

    void GameViewport::SetEmbeddedSize(uint32_t w, uint32_t h)
    {
        if (mode != GameViewportMode::Embedded) return;
        if (w == width && h == height) return;
        if (w == 0 || h == 0) { width = w; height = h; return; }

        width = w;
        height = h;
        Renderer2D::ResizeRenderTarget(target, w, h);
    }

    bool GameViewport::IsReady()
    {
        return width > 0 && height > 0;
    }

    RenderTargetHandle GameViewport::GetRenderTarget()
    {
        return mode == GameViewportMode::Embedded ? target : RenderTargetHandle{};
    }

    TextureHandle GameViewport::GetColorTexture()
    {
        if (mode != GameViewportMode::Embedded) return {};
        return Renderer2D::GetRenderTargetTexture(target);
    }

    void GameViewport::BeginFrame(const glm::mat4& viewProjection, const glm::vec4& clearColor)
    {
        if (!IsReady()) return;
        Renderer2D::BeginScene(viewProjection, clearColor, GetRenderTarget());
    }

    void GameViewport::EndFrame()
    {
        if (!IsReady()) return;
        Renderer2D::EndScene();
    }

    void GameViewport::Present()
    {
        if (mode != GameViewportMode::Standalone) return;
        Renderer2D::BindBackbuffer();
        Renderer2D::Present();
    }
}