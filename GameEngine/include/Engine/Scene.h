#pragma once

#include "EngineAPI.h"

#include "SimpleAPI/GameObject.h"
#include "SimpleAPI/Transform.h"

#include "ECS/Registry.h"

#include <optional>
#include <string>

namespace Vortx
{

	class ENGINE_API Scene
	{
	public:

		std::string path;

		std::unordered_map<AssetID, GameObject> gameObjectsRegister;

		void ChangeTransformParent(Transform& transform, std::optional<Transform>& parent);
		void ChangeTransformParent(Transform& transform, std::optional<Transform>& parent, uint32_t index);

		GameObject CreateGameObject();

		void UpdateTransforms();

		static std::unique_ptr<Scene>& Current();

		Registry& GetRegistry() { return registry; };

		std::vector<Transform>& GetChildren() { return children; }

		std::vector<uint8_t> Serialize();

		void Deserialize(const std::vector<uint8_t>& data);

		void SyncAssets();

	private:
		Registry registry;

		std::vector<Transform> children;

		void DetachFromParent(Transform& transform);
	};
}