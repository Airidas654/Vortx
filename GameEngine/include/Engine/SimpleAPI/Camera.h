#pragma once

#include "SimpleAPI/BaseComponent.h"
#include "SimpleAPI/GameObject.h"

#include "EngineAPI.h"

#include <glm/glm.hpp>

#include <optional>

namespace Vortx
{
	struct ENGINE_API CameraData;

	class ENGINE_API Camera : public BaseComponent
	{
	protected:
		void _InternalCreate() override;

	public:

		static Camera* main;

		Camera() = default;

		Camera(const Camera&) = default;
		Camera& operator=(const Camera&) = default;

		Camera(Camera&&) = default;
		Camera& operator=(Camera&&) = default;

		Camera(Entity entity) : BaseComponent(entity) {}

		Camera(Entity entity, bool create);

		~Camera();

		void ChangeMain(Camera camera);

		glm::mat4 GetProjectionMatrix();

		CameraData& Data();

		GameObject GetGameObject();

		bool operator== (Camera& other) { return entity == other.entity; }
		bool operator== (const Camera& other) const { return entity == other.entity; }

		bool operator!= (Camera& other) { return entity != other.entity; }
		bool operator!= (const Camera& other) const { return entity != other.entity; }
	};
}