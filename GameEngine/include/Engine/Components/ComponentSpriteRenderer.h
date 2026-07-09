#pragma once

#include "AssetID.h"

#include <glm/glm.hpp>

namespace Vortx
{
    struct SpriteRendererData
    {
        AssetID textureID = AssetID(0);
        glm::vec4 color = glm::vec4(1.0f);
        bool flipX = false;
        bool flipY = false;
    };
}
