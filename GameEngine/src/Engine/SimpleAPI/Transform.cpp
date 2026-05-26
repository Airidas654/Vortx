#include "SimpleAPI/Transform.h"

#include "Components/ComponentTransform.h"

#include "Scene.h"

void Transform::_InternalCreate()
{
	entity.AddComponent<TransformData>();
}

Transform::Transform(Entity entity, bool create) : BaseComponent(entity)
{
	_InternalCreate();
}

TransformData& Transform::Data()
{
	return entity.GetComponent<TransformData>();
}

glm::vec2& Transform::Position()
{
	return entity.GetComponent<TransformData>().position;
}

glm::vec1& Transform::Rotation()
{
	return entity.GetComponent<TransformData>().rotation;
}

std::vector<Transform>& Transform::GetChildren()
{
	return entity.GetComponent<TransformData>().children;
}

std::optional<Transform> Transform::GetParent()
{
	return entity.GetComponent<TransformData>().parent;
}

void Transform::SetParent(std::optional<Transform> parent)
{
	Scene::current->ChangeTransformParent(*this, parent);
}

void Transform::SetParent(std::optional<Transform> parent, uint32_t index)
{
	Scene::current->ChangeTransformParent(*this, parent, index);
}

glm::vec2& Transform::Scale()
{
	return entity.GetComponent<TransformData>().scale;
}

GameObject Transform::GameObject()
{
	return GameObject::GameObject(entity);
}
