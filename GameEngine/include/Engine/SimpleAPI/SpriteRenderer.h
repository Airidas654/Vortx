#pragma once

#include "SimpleAPI/BaseComponent.h"
#include "SimpleAPI/GameObject.h"

#include "EngineAPI.h"

#include <glm/glm.hpp>

#include <optional>

namespace Vortx
{
	struct ENGINE_API SpriteRendererData;

	class ENGINE_API SpriteRenderer : public BaseComponent
	{
	protected:
		void _InternalCreate() override;

	public:

		SpriteRenderer() = default;

		bool Exists() override;

		SpriteRenderer(const SpriteRenderer&) = default;
		SpriteRenderer& operator=(const SpriteRenderer&) = default;

		SpriteRenderer(SpriteRenderer&&) = default;
		SpriteRenderer& operator=(SpriteRenderer&&) = default;

		SpriteRenderer(Entity entity) : BaseComponent(entity) {}

		SpriteRenderer(Entity entity, bool create);

		SpriteRendererData& Data();

		Vortx::AssetID GetTexture();
		void SetTexture(const Vortx::AssetID& textureID);

		GameObject GetGameObject();

		bool operator== (SpriteRenderer& other) { return entity == other.entity; }
		bool operator== (const SpriteRenderer& other) const { return entity == other.entity; }

		bool operator!= (SpriteRenderer& other) { return entity != other.entity; }
		bool operator!= (const SpriteRenderer& other) const { return entity != other.entity; }
	};
}