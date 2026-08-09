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
	private:
		static Camera main;

	public:

		Camera() = default;

		bool Exists() override;

		Camera(const Camera&) = default;
		Camera& operator=(const Camera&) = default;

		Camera(Camera&&) = default;
		Camera& operator=(Camera&&) = default;

		Camera(Entity entity) : BaseComponent(entity) {}

		Camera(Entity entity, bool create);

		~Camera() = default;

		static Camera Main();

		static void ChangeMain(Camera camera);

		float GetSize();

		void SetSize(float size);

		void RecalculateProjectionMatrix();

		glm::mat4 GetProjectionMatrix();

		glm::mat4 GetProjectionViewMatrix();

		CameraData& Data();

		GameObject GetGameObject();

		bool operator== (Camera& other) { return entity == other.entity; }
		bool operator== (const Camera& other) const { return entity == other.entity; }

		bool operator!= (Camera& other) { return entity != other.entity; }
		bool operator!= (const Camera& other) const { return entity != other.entity; }
	};
}