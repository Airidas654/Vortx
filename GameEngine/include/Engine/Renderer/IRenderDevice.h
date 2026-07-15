#pragma once

#include "EngineAPI.h"
#include "Renderer/RenderTypes.h"

#include <glm/glm.hpp>
#include <string>
#include <cstdint>

struct GLFWwindow;

namespace Vortx
{
    class ENGINE_API IRenderDevice
    {
    public:
        virtual ~IRenderDevice() = default;

        virtual bool Init(GLFWwindow* window) = 0;
        virtual void Shutdown() = 0;

        virtual void OnResize(uint32_t width, uint32_t height) = 0;

        virtual void BeginFrame() = 0;
        virtual void Clear(const glm::vec4& color) = 0;
        virtual void SetViewProjection(const glm::mat4& viewProjection) = 0;
        virtual void EndFrame() = 0;

        virtual RenderTargetHandle CreateRenderTarget(uint32_t width, uint32_t height) = 0;
        virtual void ResizeRenderTarget(RenderTargetHandle target, uint32_t width, uint32_t height) = 0;
        virtual void DestroyRenderTarget(RenderTargetHandle target) = 0;
        virtual TextureHandle GetRenderTargetTexture(RenderTargetHandle target) = 0;

        virtual void BindRenderTarget(RenderTargetHandle target) = 0;

        virtual TextureHandle CreateTexture(const TextureDesc& desc) = 0;
        virtual void DestroyTexture(TextureHandle handle) = 0;

        virtual void* GetNativeTextureHandle(TextureHandle texture) = 0;

        virtual ShaderHandle CreateShader(const std::string& vertexSrc, const std::string& fragmentSrc) = 0;
        virtual void DestroyShader(ShaderHandle handle) = 0;

        virtual void SubmitBatch(const QuadVertex* vertices, uint32_t quadCount, const TextureHandle* textures, uint32_t textureCount, ShaderHandle shader) = 0;
    };
}