#pragma once

#include "EngineAPI.h"
#include "Renderer/RenderTypes.h"

#include <glm/glm.hpp>
#include <string>
#include <cstdint>

struct GLFWwindow;

namespace Vortx
{
    // The ONLY class allowed to know about the real graphics API.
    // Renderer2D - and everything above it - talks exclusively to this
    // interface. To move from OpenGL to WebGPU (or Vulkan, or anything
    // else), you write one new class implementing IRenderDevice and change
    // one line where the device gets constructed. Renderer2D, Scene,
    // EditorUI etc. don't change at all.
    class ENGINE_API IRenderDevice
    {
    public:
        virtual ~IRenderDevice() = default;

        // Window already exists (glfwCreateWindow already ran); this is
        // where a backend sets up its context/surface/swapchain equivalent.
        virtual bool Init(GLFWwindow* window) = 0;
        virtual void Shutdown() = 0;

        virtual void OnResize(uint32_t width, uint32_t height) = 0;

        virtual void BeginFrame() = 0;
        virtual void Clear(const glm::vec4& color) = 0;
        virtual void SetViewProjection(const glm::mat4& viewProjection) = 0;
        virtual void EndFrame() = 0; // present / swap buffers - call ONCE per frame, after ImGui too

        // Render targets - off-screen textures you can draw into instead
        // of the window's backbuffer. This is what makes "render only
        // inside this ImGui panel" possible: create one per panel, resize
        // it to match that panel's content region, bind it before drawing.
        virtual RenderTargetHandle CreateRenderTarget(uint32_t width, uint32_t height) = 0;
        virtual void ResizeRenderTarget(RenderTargetHandle target, uint32_t width, uint32_t height) = 0;
        virtual void DestroyRenderTarget(RenderTargetHandle target) = 0;
        virtual TextureHandle GetRenderTargetTexture(RenderTargetHandle target) = 0;

        // target.IsValid() == false binds the default backbuffer/window.
        virtual void BindRenderTarget(RenderTargetHandle target) = 0;

        virtual TextureHandle CreateTexture(const TextureDesc& desc) = 0;
        virtual void DestroyTexture(TextureHandle handle) = 0;

        // Backend-native texture handle, for feeding into ImGui::Image.
        // This is the one place a bit of backend-specific meaning leaks
        // through by necessity - ImGui's own renderer backend
        // (imgui_impl_opengl3 today, imgui_impl_wgpu later) expects a
        // native handle, not an opaque engine one. On GL this returns the
        // GLuint texture name cast to a pointer; a WebGPU backend would
        // return a WGPUTextureView instead. EditorUI code calling
        // ImGui::Image() doesn't change either way.
        virtual void* GetNativeTextureHandle(TextureHandle texture) = 0;

        // vertexSrc/fragmentSrc are GLSL for the OpenGL backend today. A
        // WebGPU backend would expect WGSL instead - Renderer2D never sees
        // the source itself (it lives in Renderer2D.cpp as a constant), so
        // swapping shader language later means editing those two strings
        // in one place, not touching this interface.
        virtual ShaderHandle CreateShader(const std::string& vertexSrc, const std::string& fragmentSrc) = 0;
        virtual void DestroyShader(ShaderHandle handle) = 0;

        // Submits exactly one draw batch built by Renderer2D. quadCount
        // implies both vertexCount (quadCount*4) and indexCount
        // (quadCount*6) - the index buffer itself is uploaded once at
        // Init() since its contents never change, see GenerateQuadIndices.
        virtual void SubmitBatch(const QuadVertex* vertices, uint32_t quadCount,
            const TextureHandle* textures, uint32_t textureCount,
            ShaderHandle shader) = 0;
    };
}