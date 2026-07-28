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

		const glm::mat4& GetWorldMatrix();

		glm::vec2 Right();
		glm::vec2 Up();

		glm::vec2 TransformPoint(const glm::vec2& localPoint);      // local -> world    TODO
		glm::vec2 InverseTransformPoint(const glm::vec2& worldPoint); // world -> local   TODO
		glm::vec2 TransformDirection(const glm::vec2& localDir);     // ignores translation   TODO

		void Translate(const glm::vec2& delta);          // local-space move  TODO
		void Rotate(float degreesOrRadians);             //   TODO
		void LookAt(const glm::vec2& worldTarget);        // sets rotation to face a point   TODO

		void MarkWorldDirty();
		void EnsureWorldUpdated();
		void UpdateSubtree();

		int GetChildCount();            // TODO
		Transform GetChild(int index);  // TODO
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