#include "Scene.h"

extern "C"
{

	void Scene::ChangeTransformParent(Transform& transform, const Transform* parent)
	{
		//if ((parent == nullptr && transform.parent == AssetID(0)) || transform.id == parent->id || transform.parent == parent->id) return;

		//auto view = registry.view<Transform>();

		//if (transform.parent == AssetID(0))
		//{
		//	std::remove(children.begin(), children.end(), transform.id);
		//}
		//else 
		//{
		//	//Transform &prev_parent = registry.g

		//	//view.
		//}


	}

	GameObject Scene::CreateGameObject()
	{
		GameObject gameObject = { registry.Create() };

		children.emplace_back(gameObject.AddComponent<Transform>());

		return gameObject;
	}

}