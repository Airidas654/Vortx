#pragma once

#include <glm/glm.hpp>
#include <cstdint>
#include <vector>

namespace Vortx
{
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
        uint32_t channels = 4;
        const uint8_t* pixels = nullptr;
        TextureFilter filter = TextureFilter::Linear;
        TextureWrap wrap = TextureWrap::ClampToEdge;
    };

    struct QuadVertex
    {
        glm::vec3 position;
        glm::vec4 color;
        glm::vec2 uv;
        float texIndex;
    };

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