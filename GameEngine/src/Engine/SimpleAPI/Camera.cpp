#include "SimpleAPI/Camera.h"

#include "Components/ComponentCamera.h"
#include "Renderer/GameViewport.h"
#include "Scene.h"

#include "glm/gtc/matrix_transform.hpp"

#include <iostream>

namespace Vortx
{
	Camera Camera::main = Camera();

	void Camera::_InternalCreate()
	{
		entity.AddComponent<CameraData>();

		Camera cam = Camera(entity);
		cam.RecalculateProjectionMatrix();

		if (Camera::main == Camera())
		{
			ChangeMain(cam);
		}
	}

	Camera::Camera(Entity entity, bool create) : BaseComponent(entity)
	{
		_InternalCreate();
	}

	Camera Camera::Main()
	{
		if (Camera::main == Camera() || !Camera::main.GetEntity().HasComponent<CameraData>())
		{
			auto& view = Scene::Current()->GetRegistry().View<CameraData>();
			if (view.empty()) {
				Camera::main = Camera();
			}
			else {
				ChangeMain(Camera(Entity(&Scene::Current()->GetRegistry(), view.front())));
			}
		}
		return Camera::main;
	}

	void Camera::ChangeMain(Camera camera)
	{
		Camera::main = camera;
		Camera::main.RecalculateProjectionMatrix();
	}

	glm::mat4 Camera::GetProjectionMatrix()
	{
		return Data().projMatrix;
	}

	glm::mat4 Camera::GetProjectionViewMatrix()
	{
		return Data().projMatrix * GetComponent<Transform>().GetInverseWorldMatrix();
	}

	float Camera::GetSize()
	{
		return Data().size;
	}

	void Camera::SetSize(float size)
	{
		Data().size = size;
		RecalculateProjectionMatrix();
	}

	void Camera::RecalculateProjectionMatrix()
	{
		float sizeY = Data().size;
		
		float sizeX = sizeY * GameViewport::GetAspect();

		Data().projMatrix = glm::ortho(-sizeX, sizeX, -sizeY, sizeY, -1.0f, 1.0f);
	}

	CameraData& Camera::Data()
	{
		return entity.GetComponent<CameraData>();
	}

	GameObject Camera::GetGameObject()
	{
		return GameObject(entity);
	}
}
