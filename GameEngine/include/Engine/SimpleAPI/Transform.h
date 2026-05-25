#pragma once

#include "SimpleAPI/BaseComponent.h"
#include "SimpleAPI/GameObject.h"

#include <glm/glm.hpp>

struct TransformData;

class Transform : public BaseComponent
{
protected:
	void _InternalCreate() override;

public:

	Transform(Entity entity) : BaseComponent(entity) {}

	Transform(Entity entity, bool create);

	glm::vec2& Position();
	glm::vec1& Rotation();
	glm::vec2& Scale();

	TransformData& Data();

	GameObject GameObject();
};