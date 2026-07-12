#pragma once

#include <glm/glm.hpp>
#include <cstdint>
#include <vector>

namespace Vortx
{
    // Opaque handles. Nothing outside a backend's own .cpp should ever see
    // the real resource behind these (GLuint, WGPUTexture, VkImage...) -
    // engine/editor code only ever holds an ID.
    struct TextureHandle
    {
        uint32_t id = 0;
        bool IsValid() const { return id != 0; }
        bool operator==(const TextureHandle& other) const { return id == other.id; }
    };

    struct ShaderHandle
    {
        uint32_t id = 0;
        bool IsValid() const { return id != 0; }
    };

    // A texture you can render INTO instead of just sample from - what a
    // "Scene" or "Game View" panel actually draws into before ImGui
    // displays it as an image. id 0 means "the default backbuffer/window",
    // used as BeginScene's default target.
    struct RenderTargetHandle
    {
        uint32_t id = 0;
        bool IsValid() const { return id != 0; }
    };

    enum class TextureFilter { Nearest, Linear };
    enum class TextureWrap { Repeat, ClampToEdge };

    struct TextureDesc
    {
        uint32_t width = 0;
        uint32_t height = 0;
        uint32_t channels = 4;              // 4 = RGBA8
        const uint8_t* pixels = nullptr;    // tightly packed, row-major
        TextureFilter filter = TextureFilter::Linear;
        TextureWrap wrap = TextureWrap::ClampToEdge;
    };

    // One vertex of a 2D quad. Renderer2D builds arrays of these on the CPU
    // every frame; every backend just needs to be able to consume this
    // layout, which is why it lives here instead of inside the GL backend.
    struct QuadVertex
    {
        glm::vec3 position;   // z is only for layering/sort, not real 3D depth
        glm::vec4 color;
        glm::vec2 uv;
        float texIndex;       // index into the batch's bound texture slots
    };

    // The index pattern for a quad batch is always 0,1,2,2,3,0 repeated
    // with a +4 offset per quad - it never changes at runtime. Every
    // backend can generate this once at init and upload it once, instead
    // of rebuilding/re-uploading indices every frame.
    inline std::vector<uint32_t> GenerateQuadIndices(uint32_t maxQuads)
    {
        std::vector<uint32_t> indices(maxQuads * 6);
        for (uint32_t i = 0; i < maxQuads; i++)
        {
            uint32_t base = i * 4;
            uint32_t offset = i * 6;
            indices[offset + 0] = base + 0;
            indices[offset + 1] = base + 1;
            indices[offset + 2] = base + 2;
            indices[offset + 3] = base + 2;
            indices[offset + 4] = base + 3;
            indices[offset + 5] = base + 0;
        }
        return indices;
    }
}