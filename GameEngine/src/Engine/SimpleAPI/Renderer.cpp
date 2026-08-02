#include "SimpleAPI\Renderer.h"

#include "Core/AssetDatabase.h"
#include "Assets/TextureAsset.h"

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

namespace Vortx
{
    namespace
    {
        constexpr uint32_t kMaxQuadsPerBatch = 10000;
        constexpr uint32_t kMaxTextureSlots = 16;

        struct DrawCommand
        {
            glm::mat4 transform;
            TextureHandle texture;
            glm::vec4 tint;
            int32_t layer;
            int32_t orderInLayer;
        };

        constexpr glm::vec2 kUnitCorners[4] = 
        {
            { -0.5f, -0.5f },
            {  0.5f, -0.5f },
            {  0.5f,  0.5f },
            { -0.5f,  0.5f }
        };
        constexpr glm::vec2 kUnitUVs[4] = { {0,0}, {1,0}, {1,1}, {0,1} };

        std::unique_ptr<IRenderDevice> s_device;
        ShaderHandle s_shader;
        TextureHandle s_whiteTexture;

        std::vector<DrawCommand> s_commands;
        std::vector<QuadVertex> s_vertices;
        std::array<TextureHandle, kMaxTextureSlots> s_textureSlots;
        uint32_t s_textureSlotCount = 0;
        uint32_t s_quadsInBatch = 0;

        const char* kVertexShaderSrc = R"(
            #version 330 core
            layout(location = 0) in vec3 a_Position;
            layout(location = 1) in vec4 a_Color;
            layout(location = 2) in vec2 a_UV;
            layout(location = 3) in float a_TexIndex;

            uniform mat4 u_ViewProjection;

            out vec4 v_Color;
            out vec2 v_UV;
            out float v_TexIndex;

            void main()
            {
                v_Color = a_Color;
                v_UV = a_UV;
                v_TexIndex = a_TexIndex;
                gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
            }
        )";

        const char* kFragmentShaderSrc = R"(
            #version 330 core
            in vec4 v_Color;
            in vec2 v_UV;
            in float v_TexIndex;

            uniform sampler2D u_Textures[16];

            out vec4 FragColor;

            void main()
            {
                int index = int(v_TexIndex);
                FragColor = texture(u_Textures[index], v_UV) * v_Color;
            }
        )";

        void StartBatch()
        {
            s_vertices.clear();
            s_quadsInBatch = 0;
            s_textureSlotCount = 1;
            s_textureSlots[0] = s_whiteTexture;
        }

        float GetOrAddTextureSlot(TextureHandle texture)
        {
            for (uint32_t i = 0; i < s_textureSlotCount; i++)
                if (s_textureSlots[i].id == texture.id)
                    return (float)i;

            float slot = (float)s_textureSlotCount;
            s_textureSlots[s_textureSlotCount++] = texture;
            return slot;
        }

        void Flush()
        {
            if (s_quadsInBatch == 0) return;

            s_device->SubmitBatch(s_vertices.data(), s_quadsInBatch,
                s_textureSlots.data(), s_textureSlotCount,
                s_shader);
        }

        void EmitQuad(const DrawCommand& cmd)
        {
            bool alreadyBound = false;
            for (uint32_t i = 0; i < s_textureSlotCount; i++)
                if (s_textureSlots[i].id == cmd.texture.id) { alreadyBound = true; break; }

            uint32_t slotsNeeded = alreadyBound ? 0 : 1;
            bool vertexOverflow = s_quadsInBatch + 1 > kMaxQuadsPerBatch;
            bool slotOverflow = s_textureSlotCount + slotsNeeded > kMaxTextureSlots;

            if (vertexOverflow || slotOverflow)
            {
                Flush();
                StartBatch();
            }

            float texIndex = GetOrAddTextureSlot(cmd.texture);

            for (int i = 0; i < 4; i++)
            {
                glm::vec4 worldPos = cmd.transform * glm::vec4(kUnitCorners[i], 0.0f, 1.0f);

                QuadVertex v;
                v.position = glm::vec3(worldPos);
                v.color = cmd.tint;
                v.uv = kUnitUVs[i];
                v.texIndex = texIndex;
                s_vertices.push_back(v);
            }

            s_quadsInBatch++;
        }
    }

    bool Renderer::Init(std::unique_ptr<IRenderDevice> device, GLFWwindow* window)
    {
        s_device = std::move(device);

        if (!s_device->Init(window))
            return false;

        s_shader = s_device->CreateShader(kVertexShaderSrc, kFragmentShaderSrc);

        uint8_t whitePixel[4] = { 255, 255, 255, 255 };
        TextureDesc whiteDesc;
        whiteDesc.width = 1;
        whiteDesc.height = 1;
        whiteDesc.channels = 4;
        whiteDesc.pixels = whitePixel;
        whiteDesc.filter = TextureFilter::Nearest;
        s_whiteTexture = s_device->CreateTexture(whiteDesc);

        s_commands.reserve(4096);
        s_vertices.reserve(kMaxQuadsPerBatch * 4);

        return true;
    }

    void Renderer::Shutdown()
    {
        s_device->DestroyTexture(s_whiteTexture);
        s_device->DestroyShader(s_shader);
        s_device->Shutdown();
        s_device.reset();
    }

    void Renderer::OnResize(uint32_t width, uint32_t height)
    {
        s_device->OnResize(width, height);
    }

    void Renderer::NewFrame()
    {
        s_device->BeginFrame();
        s_commands.clear();
    }

    /*void Renderer::BeginScene(const glm::mat4& viewProjection, const glm::vec4& clearColor, RenderTargetHandle target)
    {
        s_device->BindRenderTarget(target);
        s_device->Clear(clearColor);
        s_device->SetViewProjection(viewProjection);

        s_commands.clear();
    }

    void Renderer::EndScene()
    {
        std::stable_sort(s_commands.begin(), s_commands.end(),
            [](const DrawCommand& a, const DrawCommand& b)
        {
            if (a.layer != b.layer) return a.layer < b.layer;
            return a.orderInLayer < b.orderInLayer;
        });

        StartBatch();

        for (const DrawCommand& cmd : s_commands)
            EmitQuad(cmd);

        Flush();

        s_commands.clear();
    }*/

    void Renderer::RenderView(const glm::mat4& viewProjection, const glm::vec4& clearColor, RenderTargetHandle target)
    {
        s_device->BindRenderTarget(target);
        s_device->Clear(clearColor);
        s_device->SetViewProjection(viewProjection);

        std::stable_sort(s_commands.begin(), s_commands.end(),
            [](const DrawCommand& a, const DrawCommand& b)
        {
            if (a.layer != b.layer) return a.layer < b.layer;
            return a.orderInLayer < b.orderInLayer;
        });

        StartBatch();
        for (const DrawCommand& cmd : s_commands)
            EmitQuad(cmd);
        Flush();
    }

    void Renderer::Clear(const glm::vec4& color, RenderTargetHandle target)
    {
        s_device->BindRenderTarget(target);
        s_device->Clear(color);
    }

    void Renderer::BindBackbuffer()
    {
        s_device->BindRenderTarget({});
    }

    void Renderer::Present()
    {
        s_device->EndFrame();
    }

    void Renderer::DrawQuad(const glm::mat4& transform, const glm::vec4& color,
        int32_t layer, int32_t orderInLayer)
    {
        DrawSprite(transform, s_whiteTexture, color, layer, orderInLayer);
    }

    void Renderer::DrawSprite(const glm::mat4& transform, TextureHandle texture,
        const glm::vec4& tint, int32_t layer, int32_t orderInLayer)
    {
        s_commands.push_back({ transform, texture, tint, layer, orderInLayer });
    }

    void Renderer::DrawSprite(const glm::mat4& transform, AssetID texture,
        const glm::vec4& tint, int32_t layer, int32_t orderInLayer)
    {
        DrawSprite(transform, AssetDatabase::GetLoaded<TextureAsset>(texture)->handle, tint, layer, orderInLayer);
    }

    void Renderer::DrawQuad(const glm::vec2& position, const glm::vec2& size, glm::vec1 rotation,
        const glm::vec4& color, int32_t layer, int32_t orderInLayer)
    {
        glm::mat4 t = glm::translate(glm::mat4(1.0f), glm::vec3(position, 0.0f));
        t = glm::rotate(t, rotation.x, glm::vec3(0, 0, 1));
        t = glm::scale(t, glm::vec3(size, 1.0f));
        DrawQuad(t, color, layer, orderInLayer);
    }

    void Renderer::DrawSprite(const glm::vec2& position, const glm::vec2& size, glm::vec1 rotation,
        TextureHandle texture, const glm::vec4& tint, int32_t layer, int32_t orderInLayer)
    {
        glm::mat4 t = glm::translate(glm::mat4(1.0f), glm::vec3(position, 0.0f));
        t = glm::rotate(t, rotation.x, glm::vec3(0, 0, 1));
        t = glm::scale(t, glm::vec3(size, 1.0f));
        DrawSprite(t, texture, tint, layer, orderInLayer);
    }

    void Renderer::DrawSprite(const glm::vec2& position, const glm::vec2& size, glm::vec1 rotation,
        AssetID texture, const glm::vec4& tint, int32_t layer, int32_t orderInLayer)
    {
        glm::mat4 t = glm::translate(glm::mat4(1.0f), glm::vec3(position, 0.0f));
        t = glm::rotate(t, rotation.x, glm::vec3(0, 0, 1));
        t = glm::scale(t, glm::vec3(size, 1.0f));
        DrawSprite(t, AssetDatabase::GetLoaded<TextureAsset>(texture)->handle, tint, layer, orderInLayer);
    }

    TextureHandle Renderer::CreateTexture(const TextureDesc& desc)
    {
        return s_device->CreateTexture(desc);
    }

    void Renderer::DestroyTexture(TextureHandle handle)
    {
        s_device->DestroyTexture(handle);
    }

    RenderTargetHandle Renderer::CreateRenderTarget(uint32_t width, uint32_t height)
    {
        return s_device->CreateRenderTarget(width, height);
    }

    void Renderer::ResizeRenderTarget(RenderTargetHandle target, uint32_t width, uint32_t height)
    {
        s_device->ResizeRenderTarget(target, width, height);
    }

    void Renderer::DestroyRenderTarget(RenderTargetHandle target)
    {
        s_device->DestroyRenderTarget(target);
    }

    TextureHandle Renderer::GetRenderTargetTexture(RenderTargetHandle target)
    {
        return s_device->GetRenderTargetTexture(target);
    }

    void* Renderer::GetNativeTextureHandle(TextureHandle texture)
    {
        return s_device->GetNativeTextureHandle(texture);
    }

    IRenderDevice* Renderer::GetDevice()
    {
        return s_device.get();
    }
}