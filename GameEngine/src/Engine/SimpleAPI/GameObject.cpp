#include "SimpleAPI/GameObject.h"
#include "Components/ComponentMetadata.h"
#include "Scene.h"

#include <iostream>

namespace Vortx
{

	void GameObject::_InternalCreate()
	{
	}

	GameObject::GameObject(Entity entity, bool create) : BaseComponent(entity)
	{
	}

	const std::string& GameObject::NameGet()
	{
		return entity.GetComponent<MetadataData>().name;
	}

	void GameObject::NameSet(const std::string& name)
	{
		entity.GetComponent<MetadataData>().name = name;
	}

	const std::string& GameObject::TagGet()
	{
		std::cout << "tag get" << std::endl;
		return entity.GetComponent<MetadataData>().tag;
	}

	void GameObject::TagSet(const std::string& tag)
	{
		entity.GetComponent<MetadataData>().tag = tag;
	}

	const AssetID& GameObject::IDGet()
	{
		return entity.GetComponent<MetadataData>().id;
	}

	void GameObject::IDSet(const AssetID& id)
	{
		if (Scene::Current() == nullptr) return;

		MetadataData& meta = entity.GetComponent<MetadataData>();

		auto it = Scene::Current()->gameObjectsRegister.find(meta.id);

		if (it != Scene::Current()->gameObjectsRegister.end())
		{
			GameObject obj = it->second;

			Scene::Current()->gameObjectsRegister.erase(it);

			meta.id = id;
			Scene::Current()->gameObjectsRegister.emplace(id, obj);
		}
	}
}