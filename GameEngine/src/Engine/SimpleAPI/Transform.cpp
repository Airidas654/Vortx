#include "SimpleAPI/Transform.h"

#include "Components/ComponentTransform.h"

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

glm::vec2& Transform::Scale()
{
	return entity.GetComponent<TransformData>().scale;
}

GameObject Transform::GameObject()
{
	return GameObject::GameObject(entity);
}