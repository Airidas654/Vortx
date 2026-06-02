#pragma once

#include "SimpleAPI/BaseComponent.h"
#include "SimpleAPI/GameObject.h"

#include "EngineAPI.h"

#include <glm/glm.hpp>

#include <optional>

struct ENGINE_API TransformData;

class ENGINE_API Transform : public BaseComponent
{
protected:
	void _InternalCreate() override;

public:

	Transform() = default;

	Transform(const Transform&) = default;
	Transform& operator=(const Transform&) = default;

	Transform(Transform&&) = default;
	Transform& operator=(Transform&&) = default;

	Transform(Entity entity) : BaseComponent(entity) {}

	Transform(Entity entity, bool create);

	glm::vec2& Position();
	glm::vec1& Rotation();
	glm::vec2& Scale();

	std::vector<Transform>& GetChildren();
	std::optional<Transform> GetParent();
	void SetParent(std::optional<Transform> parent);
	void SetParent(std::optional<Transform> parent, uint32_t index);

	TransformData& Data();

	GameObject GetGameObject();

	bool operator== (Transform& other) { return entity == other.entity; }
	bool operator== (const Transform& other) const { return entity == other.entity; }
		
	bool operator!= (Transform& other) { return entity != other.entity; }
	bool operator!= (const Transform& other) const { return entity != other.entity; }
};