#include "Scene.h"

#include "SimpleAPI/Transform.h" 
#include "Components/ComponentTransform.h" 
#include "Components/ComponentMetadata.h" 

#include <iostream>

std::unique_ptr<Scene> Scene::current = nullptr;

void Scene::DetachFromParent(Transform& transform)
{
	TransformData& data = transform.Data();

	std::optional<Transform> parent = data.parent;

	if (!parent.has_value())
	{
		const auto it = std::remove(children.begin(), children.end(), transform);

		children.erase(it, children.end());
	}
	else
	{
		std::vector<Transform>& child = parent.value().GetChildren();

		const auto it = std::remove(child.begin(), child.end(), transform);

		child.erase(it, child.end());
	}

	data.parent = std::nullopt;
}

void Scene::ChangeTransformParent(Transform& transform, std::optional<Transform> &parent, uint32_t index)
{
	DetachFromParent(transform);

	if (!parent.has_value())
	{
		index = std::max(0u, std::min((uint32_t)children.size(), index));

		children.emplace(children.begin() + index, transform);
	}
	else
	{
		TransformData& data_parent = parent.value().Data();

		index = std::max(0u, std::min((uint32_t)data_parent.children.size(), index));

		data_parent.children.emplace(data_parent.children.begin() + index, transform);

		transform.Data().parent = parent;
	}
}

void Scene::ChangeTransformParent(Transform& transform, std::optional<Transform> &parent)
{
	ChangeTransformParent(transform, parent, UINT32_MAX);
}

GameObject Scene::CreateGameObject()
{
	GameObject gameObject = { registry.Create() };

	children.emplace_back(gameObject.AddComponent<Transform>());
	gameObject.GetEntity().AddComponent<MetadataData>();

	return gameObject;
}
