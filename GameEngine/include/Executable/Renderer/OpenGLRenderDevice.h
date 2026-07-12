#pragma once

#include "Renderer/IRenderDevice.h"

#include <unordered_map>

namespace Vortx
{
    class OpenGLRenderDevice : public IRenderDevice
    {
    public:
        // Must be called BEFORE glfwCreateWindow (GLFW requires client-API
        // hints to be set pre-creation). A future WebGPURenderDevice would
        // have its own version of this that hints GLFW_NO_API instead.
        static void PrepareGLFWHints();

        bool Init(GLFWwindow* window) override;
        void Shutdown() override;

        void OnResize(uint32_t width, uint32_t height) override;

        void BeginFrame() override;
        void Clear(const glm::vec4& color) override;
        void SetViewProjection(const glm::mat4& viewProjection) override;
        void EndFrame() override;

        RenderTargetHandle CreateRenderTarget(uint32_t width, uint32_t height) override;
        void ResizeRenderTarget(RenderTargetHandle target, uint32_t width, uint32_t height) override;
        void DestroyRenderTarget(RenderTargetHandle target) override;
        TextureHandle GetRenderTargetTexture(RenderTargetHandle target) override;
        void BindRenderTarget(RenderTargetHandle target) override;

        TextureHandle CreateTexture(const TextureDesc& desc) override;
        void DestroyTexture(TextureHandle handle) override;
        void* GetNativeTextureHandle(TextureHandle texture) override;

        ShaderHandle CreateShader(const std::string& vertexSrc, const std::string& fragmentSrc) override;
        void DestroyShader(ShaderHandle handle) override;

        void SubmitBatch(const QuadVertex* vertices, uint32_t quadCount,
            const TextureHandle* textures, uint32_t textureCount,
            ShaderHandle shader) override;

    private:
        // Cached alongside the linked program so SubmitBatch never has to
        // call glGetUniformLocation (a string hash-map lookup) per draw.
        struct GLShaderInfo
        {
            uint32_t program = 0;
            int viewProjectionLoc = -1;
            int texturesLoc = -1;
        };

        struct GLRenderTargetInfo
        {
            uint32_t fbo = 0;
            uint32_t depthRbo = 0;
            TextureHandle colorTexture;
            uint32_t width = 0;
            uint32_t height = 0;
        };

        GLFWwindow* window = nullptr;
        glm::mat4 viewProjection = glm::mat4(1.0f);
        uint32_t defaultWidth = 0;
        uint32_t defaultHeight = 0;

        uint32_t nextTextureId = 1;
        uint32_t nextShaderId = 1;
        uint32_t nextRenderTargetId = 1;
        std::unordered_map<uint32_t, uint32_t> glTextures;             // handle id -> GL texture object
        std::unordered_map<uint32_t, GLShaderInfo> glShaders;          // handle id -> program + cached uniform locations
        std::unordered_map<uint32_t, GLRenderTargetInfo> glRenderTargets; // handle id -> fbo + attachments

        uint32_t quadVAO = 0;
        uint32_t quadVBO = 0;
        uint32_t quadEBO = 0;

        static constexpr uint32_t kMaxBatchQuads = 10000;
        static constexpr uint32_t kMaxBatchVertices = kMaxBatchQuads * 4;

        uint32_t CompileShaderStage(uint32_t stageType, const std::string& src);
    };
}