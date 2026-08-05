#include "SimpleAPI/SpriteRenderer.h"

#include "Components/ComponentSpriteRenderer.h"

#include "Scene.h"

namespace Vortx
{

	void SpriteRenderer::_InternalCreate()
	{
		entity.AddComponent<SpriteRendererData>();
	}

	SpriteRenderer::SpriteRenderer(Entity entity, bool create) : BaseComponent(entity)
	{
		_InternalCreate();
	}

	SpriteRendererData& SpriteRenderer::Data()
	{
		return entity.GetComponent<SpriteRendererData>();
	}

	AssetID SpriteRenderer::GetTexture()
	{
		return Data().textureID;
	}

	void SpriteRenderer::SetTexture(const AssetID& textureID)
	{
		Data().textureID = textureID;
		if (Scene::Current())
			Scene::Current()->MarkAssetsDirty();
	}

	GameObject SpriteRenderer::GetGameObject()
	{
		return GameObject(entity);
	}
}
