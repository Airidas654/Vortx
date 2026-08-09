#pragma once

#include "SimpleAPI/BaseComponent.h"
#include "SimpleAPI/GameObject.h"

#include "EngineAPI.h"

#include <glm/glm.hpp>

#include <optional>

namespace Vortx
{
	struct ENGINE_API TransformData;

	class ENGINE_API Transform : public BaseComponent
	{
	protected:
		void _InternalCreate() override;

	public:

		Transform() = default;

		bool Exists() override;

		Transform(const Transform&) = default;
		Transform& operator=(const Transform&) = default;

		Transform(Transform&&) = default;
		Transform& operator=(Transform&&) = default;

		Transform(Entity entity) : BaseComponent(entity) {}

		Transform(Entity entity, bool create);

		glm::vec2 LocalPosition();
		float LocalRotation();
		glm::vec2 LocalScale();

		glm::vec2 Position();
		float Rotation();
		glm::vec2 Scale();

		void SetLocalPosition(const glm::vec2& localPos);
		void SetLocalRotation(float localAngle);
		void SetLocalScale(const glm::vec2& localScale);

		void SetPosition(const glm::vec2& worldPos);
		void SetRotation(float worldAngle);
		void SetScale(const glm::vec2& worldScale);

		glm::mat4 GetWorldMatrix();
		glm::mat4 GetInverseWorldMatrix();

		glm::vec2 Right();
		glm::vec2 Up();
		
		void Translate(const glm::vec2& delta);
		void LocalTranslate(const glm::vec2& delta);
		void Rotate(float radians);
		void LookAt(const glm::vec2& worldTarget);

		glm::vec2 LocalToWorldPoint(const glm::vec2& localPoint);
		glm::vec2 WorldToLocalPoint(const glm::vec2& worldPoint);
		glm::vec2 LocalToWorldDirection(const glm::vec2& localDir);

		void MarkWorldDirty();
		void EnsureWorldUpdated();
		void UpdateSubtree();

		int GetChildCount();
		Transform GetChild(int index);
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
}