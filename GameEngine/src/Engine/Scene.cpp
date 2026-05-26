#include "Scene.h"

#include "SimpleAPI/Transform.h" 
#include "Components/ComponentTransform.h" 

#include <iostream>

std::unique_ptr<Scene> Scene::current = nullptr;

void Scene::DetachFromParent(Transform& transform)
{
	TransformData& data = transform.Data();

	std::optional<Transform> parent = data.parent;

	 std::cout << "this " << (uint32_t)transform.GetEntity().GetValue() << std::endl;
		
	if (!parent.has_value())
	{
		std::cout << "empty" << std::endl;

		const auto it = std::remove(children.begin(), children.end(), transform);

		children.erase(it, children.end());
	}
	else
	{
		std::cout << "parent " << (uint32_t)parent.value().GetEntity().GetValue() << std::endl;

		std::cout << "not empty" << std::endl;

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

		//children.emplace(children.begin() + index, transform);
		children.emplace_back(transform);
	}
	else
	{
		TransformData& data = parent.value().Data();

		index = std::max(0u, std::min((uint32_t)data.children.size(), index));

		//data.children.emplace(data.children.begin() + index, transform);

		data.children.emplace_back(transform);

		data.parent = parent;
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

	return gameObject;
}
