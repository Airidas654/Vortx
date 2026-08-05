#include "Scene.h"

#include "SimpleAPI/Transform.h" 
#include "SimpleAPI/SceneManager.h"
#include "Components/ComponentTransform.h" 
#include "Components/ComponentMetadata.h" 
#include "SerializerLoader.h"
#include "Core/File.h"
#include "Core/AssetDatabase.h"

#include "json.hpp"

#include <iostream>
#include <fstream>
#include <filesystem>

namespace Vortx
{

	std::unique_ptr<Scene>& Scene::Current()
	{
		return Vortx::SceneManager::current;
	}

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

	void Scene::UpdateTransforms()
	{
		for (Transform& root : children)
			root.UpdateSubtree();
	}

	void Scene::ChangeTransformParent(Transform& transform, std::optional<Transform>& parent, uint32_t index)
	{
		DetachFromParent(transform);

		if (!parent.has_value())
		{
			index = max(0u, min((uint32_t)children.size(), index));

			children.emplace(children.begin() + index, transform);
		}
		else
		{
			TransformData& data_parent = parent.value().Data();

			index = max(0u, min((uint32_t)data_parent.children.size(), index));

			data_parent.children.emplace(data_parent.children.begin() + index, transform);

			transform.Data().parent = parent;
		}
	}

	void Scene::ChangeTransformParent(Transform& transform, std::optional<Transform>& parent)
	{
		ChangeTransformParent(transform, parent, UINT32_MAX);
	}

	GameObject Scene::CreateGameObject()
	{
		GameObject gameObject = { registry.Create() };

		children.emplace_back(gameObject.AddComponent<Transform>());
		MetadataData meta = gameObject.GetEntity().AddComponent<MetadataData>();

		gameObjectsRegister.emplace(meta.id, gameObject);

		return gameObject;
	}

	void Scene::DestroyGameObject(GameObject gameObject)
	{
		Entity entity = gameObject.GetEntity();
		Transform transform = gameObject.GetComponent<Transform>();

		std::vector<Transform> &childrenCopy = transform.GetChildren();
		for (Transform& child : childrenCopy)
			DestroyGameObject(child.GetGameObject());

		DetachFromParent(transform);

		MetadataData& meta = entity.GetComponent<MetadataData>();
		gameObjectsRegister.erase(meta.id);

		registry.Destroy(entity);

		MarkAssetsDirty();
	}

	std::vector<uint8_t> Scene::Serialize()
	{
		return SerializerLoader::SerializeScene(*this);
	}

	void Scene::Deserialize(const std::vector<uint8_t>& data)
	{
		registry.Clear();
		children.clear();
		gameObjectsRegister.clear();
		SerializerLoader::DeserializeScene(*this, data);
		SyncAssets();
	}

	void Scene::SyncAssets()
	{
		std::vector<AssetID> refs = SerializerLoader::CollectSceneAssets(*this);
		AssetDatabase::SyncActiveSet(refs);
		assetsDirty = false;
	}

	void Scene::SyncAssetsIfDirty()
	{
		if (!assetsDirty) return;
		SyncAssets();
	}
}
