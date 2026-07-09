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

	GameObject SpriteRenderer::GetGameObject()
	{
		return GameObject(entity);
	}
}
