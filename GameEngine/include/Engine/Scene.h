#pragma once

#include "EngineAPI.h"
#include "Components/ComponentTransform.h"
#include "SimpleAPI/Transform.h"

#include "SimpleAPI/GameObject.h"

#include "ECS/Registry.h"

extern "C"
{

	class ENGINE_API Scene
	{
	public:

		void ChangeTransformParent(Transform& transform, const Transform* parent);

		GameObject CreateGameObject();

		std::vector<Transform>& GetChildren() { return children; }

	private:
		Registry registry;

		std::vector<Transform> children;
	};

}