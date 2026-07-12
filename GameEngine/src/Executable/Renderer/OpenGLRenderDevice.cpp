#include "Renderer/OpenGLRenderDevice.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <vector>

namespace Vortx
{
    void OpenGLRenderDevice::PrepareGLFWHints()
    {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    }

    bool OpenGLRenderDevice::Init(GLFWwindow* win)
    {
        window = win;

        glfwMakeContextCurrent(window);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            std::cerr << "[OpenGLRenderDevice] Failed to load GL functions\n";
            return false;
        }

        int fbWidth = 0, fbHeight = 0;
        glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
        defaultWidth = (uint32_t)fbWidth;
        defaultHeight = (uint32_t)fbHeight;

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glViewport(0, 0, fbWidth, fbHeight);

        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glGenBuffers(1, &quadEBO);

        glBindVertexArray(quadVAO);

        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, kMaxBatchVertices * sizeof(QuadVertex), nullptr, GL_DYNAMIC_DRAW);

        std::vector<uint32_t> indices = GenerateQuadIndices(kMaxBatchQuads);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(QuadVertex), (void*)offsetof(QuadVertex, position));

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(QuadVertex), (void*)offsetof(QuadVertex, color));

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(QuadVertex), (void*)offsetof(QuadVertex, uv));

        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(QuadVertex), (void*)offsetof(QuadVertex, texIndex));

        glBindVertexArray(0);

        return true;
    }

    void OpenGLRenderDevice::Shutdown()
    {
        for (auto& [id, rt] : glRenderTargets)
        {
            glDeleteRenderbuffers(1, &rt.depthRbo);
            glDeleteFramebuffers(1, &rt.fbo);
        }
        glRenderTargets.clear();

        glDeleteVertexArrays(1, &quadVAO);
        glDeleteBuffers(1, &quadVBO);
        glDeleteBuffers(1, &quadEBO);

        for (auto& [id, tex] : glTextures) glDeleteTextures(1, &tex);
        for (auto& [id, info] : glShaders) glDeleteProgram(info.program);

        glTextures.clear();
        glShaders.clear();
    }

    void OpenGLRenderDevice::OnResize(uint32_t width, uint32_t height)
    {
        defaultWidth = width;
        defaultHeight = height;
        glViewport(0, 0, (GLsizei)width, (GLsizei)height);
    }

    void OpenGLRenderDevice::BeginFrame()
    {
        // Nothing extra needed for GL. Kept for symmetry with backends that
        // need an explicit per-frame acquire step (e.g. WebGPU's
        // surface.GetCurrentTexture()).
    }

    void OpenGLRenderDevice::Clear(const glm::vec4& color)
    {
        glClearColor(color.r, color.g, color.b, color.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void OpenGLRenderDevice::SetViewProjection(const glm::mat4& vp)
    {
        viewProjection = vp;
    }

    void OpenGLRenderDevice::EndFrame()
    {
        glfwSwapBuffers(window);
    }

    RenderTargetHandle OpenGLRenderDevice::CreateRenderTarget(uint32_t width, uint32_t height)
    {
        // The color attachment is a perfectly normal texture, registered
        // in the same glTextures map as any image-loaded texture - so it
        // can be handed to ImGui, or even sampled by DrawSprite later,
        // with no special-casing anywhere else.
        uint32_t colorTex;
        glGenTextures(1, &colorTex);
        glBindTexture(GL_TEXTURE_2D, colorTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, (GLsizei)width, (GLsizei)height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);

        TextureHandle colorHandle{ nextTextureId++ };
        glTextures[colorHandle.id] = colorTex;

        uint32_t fbo, depthRbo;
        glGenFramebuffers(1, &fbo);
        glGenRenderbuffers(1, &depthRbo);

        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex, 0);

        glBindRenderbuffer(GL_RENDERBUFFER, depthRbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, (GLsizei)width, (GLsizei)height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthRbo);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cerr << "[OpenGLRenderDevice] Render target framebuffer incomplete\n";

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        RenderTargetHandle handle{ nextRenderTargetId++ };
        glRenderTargets[handle.id] = { fbo, depthRbo, colorHandle, width, height };
        return handle;
    }

    void OpenGLRenderDevice::ResizeRenderTarget(RenderTargetHandle target, uint32_t width, uint32_t height)
    {
        if (width == 0 || height == 0) return;

        auto it = glRenderTargets.find(target.id);
        if (it == glRenderTargets.end()) return;

        GLRenderTargetInfo& rt = it->second;
        if (rt.width == width && rt.height == height) return; // already the right size

        rt.width = width;
        rt.height = height;

        // GL textures/renderbuffers can't be resized in place, but
        // reallocating storage under the SAME object id means the FBO's
        // existing attachment stays valid - no need to re-attach anything.
        auto texIt = glTextures.find(rt.colorTexture.id);
        if (texIt != glTextures.end())
        {
            glBindTexture(GL_TEXTURE_2D, texIt->second);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, (GLsizei)width, (GLsizei)height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        glBindRenderbuffer(GL_RENDERBUFFER, rt.depthRbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, (GLsizei)width, (GLsizei)height);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
    }

    void OpenGLRenderDevice::DestroyRenderTarget(RenderTargetHandle target)
    {
        auto it = glRenderTargets.find(target.id);
        if (it == glRenderTargets.end()) return;

        DestroyTexture(it->second.colorTexture);
        glDeleteRenderbuffers(1, &it->second.depthRbo);
        glDeleteFramebuffers(1, &it->second.fbo);

        glRenderTargets.erase(it);
    }

    TextureHandle OpenGLRenderDevice::GetRenderTargetTexture(RenderTargetHandle target)
    {
        auto it = glRenderTargets.find(target.id);
        return it != glRenderTargets.end() ? it->second.colorTexture : TextureHandle{};
    }

    void OpenGLRenderDevice::BindRenderTarget(RenderTargetHandle target)
    {
        if (!target.IsValid())
        {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glViewport(0, 0, (GLsizei)defaultWidth, (GLsizei)defaultHeight);
            return;
        }

        auto it = glRenderTargets.find(target.id);
        if (it == glRenderTargets.end()) return;

        glBindFramebuffer(GL_FRAMEBUFFER, it->second.fbo);
        glViewport(0, 0, (GLsizei)it->second.width, (GLsizei)it->second.height);
    }

    uint32_t OpenGLRenderDevice::CompileShaderStage(uint32_t stageType, const std::string& src)
    {
        uint32_t shader = glCreateShader(stageType);
        const char* csrc = src.c_str();
        glShaderSource(shader, 1, &csrc, nullptr);
        glCompileShader(shader);

        int success = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            char log[512];
            glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
            std::cerr << "[OpenGLRenderDevice] Shader compile error: " << log << "\n";
        }
        return shader;
    }

    ShaderHandle OpenGLRenderDevice::CreateShader(const std::string& vertexSrc, const std::string& fragmentSrc)
    {
        uint32_t vert = CompileShaderStage(GL_VERTEX_SHADER, vertexSrc);
        uint32_t frag = CompileShaderStage(GL_FRAGMENT_SHADER, fragmentSrc);

        uint32_t program = glCreateProgram();
        glAttachShader(program, vert);
        glAttachShader(program, frag);
        glLinkProgram(program);

        int success = 0;
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success)
        {
            char log[512];
            glGetProgramInfoLog(program, sizeof(log), nullptr, log);
            std::cerr << "[OpenGLRenderDevice] Shader link error: " << log << "\n";
        }

        glDeleteShader(vert);
        glDeleteShader(frag);

        GLShaderInfo info;
        info.program = program;
        info.viewProjectionLoc = glGetUniformLocation(program, "u_ViewProjection");
        info.texturesLoc = glGetUniformLocation(program, "u_Textures");

        ShaderHandle handle{ nextShaderId++ };
        glShaders[handle.id] = info;
        return handle;
    }

    void OpenGLRenderDevice::DestroyShader(ShaderHandle handle)
    {
        auto it = glShaders.find(handle.id);
        if (it == glShaders.end()) return;
        glDeleteProgram(it->second.program);
        glShaders.erase(it);
    }

    TextureHandle OpenGLRenderDevice::CreateTexture(const TextureDesc& desc)
    {
        uint32_t tex;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);

        GLint filter = (desc.filter == TextureFilter::Nearest) ? GL_NEAREST : GL_LINEAR;
        GLint wrap = (desc.wrap == TextureWrap::Repeat) ? GL_REPEAT : GL_CLAMP_TO_EDGE;

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);

        GLenum format = (desc.channels == 4) ? GL_RGBA : (desc.channels == 3) ? GL_RGB : GL_RED;

        glTexImage2D(GL_TEXTURE_2D, 0, format, (GLsizei)desc.width, (GLsizei)desc.height, 0,
            format, GL_UNSIGNED_BYTE, desc.pixels);

        glBindTexture(GL_TEXTURE_2D, 0);

        TextureHandle handle{ nextTextureId++ };
        glTextures[handle.id] = tex;
        return handle;
    }

    void OpenGLRenderDevice::DestroyTexture(TextureHandle handle)
    {
        auto it = glTextures.find(handle.id);
        if (it == glTextures.end()) return;
        glDeleteTextures(1, &it->second);
        glTextures.erase(it);
    }

    void* OpenGLRenderDevice::GetNativeTextureHandle(TextureHandle texture)
    {
        auto it = glTextures.find(texture.id);
        if (it == glTextures.end()) return nullptr;
        return (void*)(intptr_t)it->second;
    }

    void OpenGLRenderDevice::SubmitBatch(const QuadVertex* vertices, uint32_t quadCount,
        const TextureHandle* textures, uint32_t textureCount,
        ShaderHandle shader)
    {
        auto progIt = glShaders.find(shader.id);
        if (progIt == glShaders.end()) return;
        const GLShaderInfo& info = progIt->second;

        glUseProgram(info.program);

        if (info.viewProjectionLoc >= 0)
            glUniformMatrix4fv(info.viewProjectionLoc, 1, GL_FALSE, &viewProjection[0][0]);

        std::vector<int> samplerSlots(textureCount);
        for (uint32_t i = 0; i < textureCount; i++)
        {
            glActiveTexture(GL_TEXTURE0 + i);
            auto texIt = glTextures.find(textures[i].id);
            glBindTexture(GL_TEXTURE_2D, texIt != glTextures.end() ? texIt->second : 0);
            samplerSlots[i] = (int)i;
        }
        if (info.texturesLoc >= 0 && textureCount > 0)
            glUniform1iv(info.texturesLoc, (GLsizei)textureCount, samplerSlots.data());

        glBindVertexArray(quadVAO);

        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, kMaxBatchVertices * sizeof(QuadVertex), nullptr, GL_DYNAMIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, quadCount * 4 * sizeof(QuadVertex), vertices);

        glDrawElements(GL_TRIANGLES, (GLsizei)(quadCount * 6), GL_UNSIGNED_INT, nullptr);

        glBindVertexArray(0);
    }
}