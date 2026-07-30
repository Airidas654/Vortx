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

		std::cout << (int)entity.GetValue() << std::endl;

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

	Camera::~Camera()
	{
		/*if (Camera::main != Camera() && Camera::main == *this)
		{*/
			//Camera::main = nullptr;

			//auto& view = Scene::Current()->GetRegistry().View<Camera>();

			/*for (auto entity : view)
			{
				if (entity != this->entity.GetValue())
				{
					ChangeMain(Camera(Entity(&Scene::Current()->GetRegistry(), entity)));
					return;
				}
			}*/
		//}
	}

	void Camera::ChangeMain(Camera camera)
	{
		Camera::main = camera;
		std::cout << "changed" << std::endl;
		if (Camera::main == Camera()) {
			std::cout << "wtf " << (int)entity.GetValue() << std::endl;
		}
	}

	glm::mat4 Camera::GetProjectionMatrix()
	{
		return Data().projMatrix;
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
