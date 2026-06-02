#include "SimpleAPI/GameObject.h"
#include "Components/ComponentMetadata.h"

#include "Scene.h"

void GameObject::_InternalCreate()
{}

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
	return entity.GetComponent<MetadataData>().tag;
}

void GameObject::TagSet(const std::string& tag)
{
	entity.GetComponent<MetadataData>().tag = tag;
}