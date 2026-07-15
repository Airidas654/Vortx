#include "SimpleAPI\Renderer2D.h"

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
            glm::vec2 position;
            glm::vec2 size;
            glm::vec1 rotation;
            TextureHandle texture;
            glm::vec4 tint;
            int32_t layer;
            int32_t orderInLayer;
        };

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

            glm::vec2 half = cmd.size * 0.5f;
            float c = cosf(cmd.rotation.x);
            float s = sinf(cmd.rotation.x);

            glm::vec2 corners[4] = {
                { -half.x, -half.y },
                {  half.x, -half.y },
                {  half.x,  half.y },
                { -half.x,  half.y }
            };
            glm::vec2 uvs[4] = { {0,0}, {1,0}, {1,1}, {0,1} };

            for (int i = 0; i < 4; i++)
            {
                glm::vec2 rotated = {
                    corners[i].x * c - corners[i].y * s,
                    corners[i].x * s + corners[i].y * c
                };

                QuadVertex v;
                v.position = glm::vec3(cmd.position + rotated, 0.0f);
                v.color = cmd.tint;
                v.uv = uvs[i];
                v.texIndex = texIndex;
                s_vertices.push_back(v);
            }

            s_quadsInBatch++;
        }
    }

    bool Renderer2D::Init(std::unique_ptr<IRenderDevice> device, GLFWwindow* window)
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

    void Renderer2D::Shutdown()
    {
        s_device->DestroyTexture(s_whiteTexture);
        s_device->DestroyShader(s_shader);
        s_device->Shutdown();
        s_device.reset();
    }

    void Renderer2D::OnResize(uint32_t width, uint32_t height)
    {
        s_device->OnResize(width, height);
    }

    void Renderer2D::NewFrame()
    {
        s_device->BeginFrame();
    }

    void Renderer2D::BeginScene(const glm::mat4& viewProjection, const glm::vec4& clearColor, RenderTargetHandle target)
    {
        s_device->BindRenderTarget(target);
        s_device->Clear(clearColor);
        s_device->SetViewProjection(viewProjection);

        s_commands.clear();
    }

    void Renderer2D::EndScene()
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
    }

    void Renderer2D::BindBackbuffer()
    {
        s_device->BindRenderTarget({});
    }

    void Renderer2D::Present()
    {
        s_device->EndFrame();
    }

    void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, glm::vec1 rotation, const glm::vec4& color,
        int32_t layer, int32_t orderInLayer)
    {
        DrawSprite(position, size, rotation, s_whiteTexture, color, layer, orderInLayer);
    }

    void Renderer2D::DrawSprite(const glm::vec2& position, const glm::vec2& size, glm::vec1 rotation,
        TextureHandle texture, const glm::vec4& tint,
        int32_t layer, int32_t orderInLayer)
    {
        s_commands.push_back({ position, size, rotation, texture, tint, layer, orderInLayer });
    }

    TextureHandle Renderer2D::CreateTexture(const TextureDesc& desc)
    {
        return s_device->CreateTexture(desc);
    }

    void Renderer2D::DestroyTexture(TextureHandle handle)
    {
        s_device->DestroyTexture(handle);
    }

    RenderTargetHandle Renderer2D::CreateRenderTarget(uint32_t width, uint32_t height)
    {
        return s_device->CreateRenderTarget(width, height);
    }

    void Renderer2D::ResizeRenderTarget(RenderTargetHandle target, uint32_t width, uint32_t height)
    {
        s_device->ResizeRenderTarget(target, width, height);
    }

    void Renderer2D::DestroyRenderTarget(RenderTargetHandle target)
    {
        s_device->DestroyRenderTarget(target);
    }

    TextureHandle Renderer2D::GetRenderTargetTexture(RenderTargetHandle target)
    {
        return s_device->GetRenderTargetTexture(target);
    }

    void* Renderer2D::GetNativeTextureHandle(TextureHandle texture)
    {
        return s_device->GetNativeTextureHandle(texture);
    }

    IRenderDevice* Renderer2D::GetDevice()
    {
        return s_device.get();
    }
}