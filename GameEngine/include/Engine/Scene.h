#pragma once

#include "EngineAPI.h"

#include "SimpleAPI/GameObject.h"
#include "SimpleAPI/Transform.h"

#include "ECS/Registry.h"

#include <optional>

class ENGINE_API Scene
{
public:

	void ChangeTransformParent(Transform& transform, std::optional<Transform>& parent);
	void ChangeTransformParent(Transform& transform, std::optional<Transform>& parent, uint32_t index);

	GameObject CreateGameObject();

	static std::unique_ptr<Scene>& Current();

	Registry& GetRegistry() { return registry; };

	std::vector<Transform>& GetChildren() { return children; }

private:
	Registry registry;

	std::vector<Transform> children;

	void DetachFromParent(Transform& transform);
};