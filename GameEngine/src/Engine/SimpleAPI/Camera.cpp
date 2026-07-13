#include "SimpleAPI/Camera.h"

#include "Components/ComponentCamera.h"
#include "Scene.h"

namespace Vortx
{
	Camera* Camera::main = nullptr;

	void Camera::_InternalCreate()
	{
		entity.AddComponent<CameraData>();

		if (Camera::main == nullptr)
		{
			ChangeMain(Camera(entity));
		}
	}

	Camera::Camera(Entity entity, bool create) : BaseComponent(entity)
	{
		_InternalCreate();
	}

	Camera::~Camera()
	{
		if (Camera::main != nullptr && *(Camera::main) == *this)
		{
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
		}
	}

	void Camera::ChangeMain(Camera camera)
	{
		if (Camera::main != nullptr)
		{
			delete Camera::main;
		}

		Camera::main = new Camera(camera);
	}

	glm::mat4 Camera::GetProjectionMatrix()
	{
		return entity.GetComponent<CameraData>().projMatrix;
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
