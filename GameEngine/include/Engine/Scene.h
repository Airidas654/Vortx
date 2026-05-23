#pragma once

#include "Components/ComponentTransform.h"

#include "SimpleAPI/GameObject.h"

#include "entt.hpp"

class Scene
{
public:

	void ChangeTransformParent(TransformData &transform, const TransformData *parent);

	GameObject CreateGameObject();

private:
	entt::registry registry;

	std::vector<AssetID> children;
};