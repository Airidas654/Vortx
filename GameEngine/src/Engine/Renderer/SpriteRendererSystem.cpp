#include "Renderer/SpriteRendererSystem.h"
#include "Scene.h"
#include "Components/ComponentTransform.h"
#include "Components/ComponentSpriteRenderer.h"
#include "Core/AssetDatabase.h"
#include "Assets/TextureAsset.h"
#include "SimpleAPI/Renderer.h"

namespace Vortx
{
    void SpriteRenderSystem::Submit()
    {
        if (!Scene::Current()) return;

        Scene::Current()->GetRegistry().View<SpriteRendererData, TransformData>().each(
            [](entt::entity, SpriteRendererData& renderer, TransformData& transform)
        {
            if (renderer.textureID != AssetID(0))
            {
                if (TextureAsset* tex = AssetDatabase::GetLoaded<TextureAsset>(renderer.textureID))
                {
                    Renderer::DrawSprite(transform.worldMatrix, tex->handle, renderer.color, renderer.layer, renderer.orderInLayer);
                    return;
                }
            }
            Renderer::DrawQuad(transform.worldMatrix, renderer.color, renderer.layer, renderer.orderInLayer);
        });
    }
}