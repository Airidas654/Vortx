#include "SimpleAPI/Transform.h"

#include "Components/ComponentTransform.h"

#include "Scene.h"

#include <iostream>

namespace Vortx
{

	void Transform::_InternalCreate()
	{
		entity.AddComponent<TransformData>();
	}

	Transform::Transform(Entity entity, bool create) : BaseComponent(entity)
	{
		_InternalCreate();
	}

	TransformData& Transform::Data()
	{
		return entity.GetComponent<TransformData>();
	}

	static glm::mat4 ComputeLocalMatrix(const TransformData& t)
	{
		glm::mat4 m = glm::translate(glm::mat4(1.0f), glm::vec3(t.localPosition, 0.0f));
		m = glm::rotate(m, t.localRotation, glm::vec3(0, 0, 1));
		m = glm::scale(m, glm::vec3(t.localScale, 1.0f));
		return m;
	}

	glm::vec2 Transform::LocalPosition() 
	{ 
		return Data().localPosition; 
	}

	float Transform::LocalRotation() 
	{ 
		return Data().localRotation; 
	}

	glm::vec2 Transform::LocalScale() 
	{ 
		return Data().localScale; 
	}

	void Transform::SetLocalPosition(const glm::vec2& localPos)
	{
		Data().localPosition = localPos;
		MarkWorldDirty();
	}

	void Transform::SetLocalRotation(float localAngle)
	{
		Data().localRotation = localAngle;
		MarkWorldDirty();
	}

	void Transform::SetLocalScale(const glm::vec2& localScale)
	{
		Data().localScale = localScale;
		MarkWorldDirty();
	}

	glm::vec2 Transform::Position() 
	{ 
		EnsureWorldUpdated(); 
		return Data().worldPositionCache; 
	}

	float Transform::Rotation() 
	{ 
		EnsureWorldUpdated(); 
		return Data().worldRotationCache; 
	}

	glm::vec2 Transform::Scale() 
	{ 
		EnsureWorldUpdated(); 
		return Data().worldScaleCache; 
	}

	void Transform::SetPosition(const glm::vec2& worldPos)
	{
		TransformData& data = Data();
		glm::vec2 local = worldPos;

		if (data.parent.has_value())
		{
			glm::mat4 parentInv = glm::inverse(data.parent->GetWorldMatrix());
			local = glm::vec2(parentInv * glm::vec4(worldPos, 0.0f, 1.0f));
		}

		data.localPosition = local;
		MarkWorldDirty();
	}

	void Transform::SetRotation(float worldAngle)
	{
		TransformData& data = Data();
		float parentAngle = data.parent.has_value() ? data.parent->Rotation() : 0.0f;
		data.localRotation = worldAngle - parentAngle;
		MarkWorldDirty();
	}

	void Transform::SetScale(const glm::vec2& worldScale)
	{
		TransformData& data = Data();
		glm::vec2 parentScale = data.parent.has_value() ? data.parent->Scale() : glm::vec2(1.0f);
		data.localScale = worldScale / parentScale;
		MarkWorldDirty();
	}

	glm::vec2 Transform::Right()
	{
		float r = Rotation();
		return { cosf(r), sinf(r) };
	}

	glm::vec2 Transform::Up()
	{
		float r = Rotation();
		return { -sinf(r), cosf(r) };
	}

	void Transform::Translate(const glm::vec2& delta)
	{
		glm::vec2 pos = Position();
		SetPosition(pos + delta);
	}

	void Transform::LocalTranslate(const glm::vec2& delta)
	{
		glm::vec2 pos = LocalPosition();
		SetLocalPosition(pos + delta);
	}

	void Transform::Rotate(float radians)
	{
		float rot = LocalRotation();
		SetLocalRotation(rot + radians);
	}

	void Transform::LookAt(const glm::vec2& worldTarget)
	{
		glm::vec2 worldPos = Position();
		glm::vec2 dir = glm::normalize(worldTarget - worldPos);

		float rot = atan2f(dir.y, dir.x);

		SetRotation(rot);
	}

	glm::vec2 Transform::LocalToWorldPoint(const glm::vec2& localPoint)
	{
		glm::vec4 result = GetWorldMatrix() * glm::vec4(localPoint, 0.0f, 1.0f);
		return glm::vec2(result.x, result.y);
	}

	glm::vec2 Transform::WorldToLocalPoint(const glm::vec2& worldPoint)
	{
		glm::mat4 inv = glm::inverse(GetWorldMatrix());
		glm::vec4 result = inv * glm::vec4(worldPoint, 0.0f, 1.0f);
		return glm::vec2(result.x, result.y);
	}

	glm::vec2 Transform::LocalToWorldDirection(const glm::vec2& localDir)
	{
		glm::vec4 result = GetWorldMatrix() * glm::vec4(localDir, 0.0f, 0.0f);
		return glm::vec2(result.x, result.y);
	}

	void Transform::MarkWorldDirty()
	{
		TransformData& data = Data();
		if (data.worldDirty) return;

		data.worldDirty = true;
		for (Transform& child : data.children)
			child.MarkWorldDirty();
	}

	void Transform::EnsureWorldUpdated()
	{
		TransformData& data = Data();
		if (!data.worldDirty) return;

		glm::mat4 parentWorld(1.0f);
		float parentRot = 0.0f;
		glm::vec2 parentScale(1.0f);

		if (data.parent.has_value())
		{
			data.parent->EnsureWorldUpdated();
			TransformData& pData = data.parent->Data();
			parentWorld = pData.worldMatrix;
			parentRot = pData.worldRotationCache;
			parentScale = pData.worldScaleCache;
		}

		data.localMatrix = ComputeLocalMatrix(data);
		data.worldMatrix = parentWorld * data.localMatrix;
		data.worldRotationCache = parentRot + data.localRotation;
		data.worldScaleCache = parentScale * data.localScale;
		data.worldPositionCache = glm::vec2(data.worldMatrix[3][0], data.worldMatrix[3][1]);

		data.worldDirty = false;
	}

	void Transform::UpdateSubtree()
	{
		EnsureWorldUpdated();
		for (Transform& child : Data().children)
			child.UpdateSubtree();
	}

	glm::mat4 Transform::GetWorldMatrix()
	{
		EnsureWorldUpdated();
		return Data().worldMatrix;
	}

	int Transform::GetChildCount()
	{
		return Data().children.size();
	}

	Transform Transform::GetChild(int index)
	{
		auto& children = Data().children;

		if (index < 0 || index >= children.size())
		{
			std::cerr << "Transform::GetChild invalid index" << std::endl;
			return Transform();
		}

		return children[index];
	}

	std::vector<Transform>& Transform::GetChildren()
	{
		return Data().children;
	}

	std::optional<Transform> Transform::GetParent()
	{
		return Data().parent;
	}

	void Transform::SetParent(std::optional<Transform> parent)
	{
		glm::vec2 pos = Position();
		float rot = Rotation();
		glm::vec2 sca = Scale();

		Scene::Current()->ChangeTransformParent(*this, parent);
		
		SetPosition(pos);
		SetRotation(rot);
		SetScale(sca);
	}

	void Transform::SetParent(std::optional<Transform> parent, uint32_t index)
	{
		glm::vec2 pos = Position();
		float rot = Rotation();
		glm::vec2 sca = Scale();

		Scene::Current()->ChangeTransformParent(*this, parent, index);
		
		SetPosition(pos);
		SetRotation(rot);
		SetScale(sca);
	}

	GameObject Transform::GetGameObject()
	{
		return GameObject(entity);
	}

	/*void Transform::UpdateWorldMatrix(const glm::mat4& parentWorld)
	{
		TransformData& data = Data();
		data.localMatrix = TransformData::ComputeLocalMatrix(data);
		data.worldMatrix = parentWorld * data.localMatrix;

		for (Transform& child : data.children)
			child.UpdateWorldMatrix(data.worldMatrix);
	}*/
}
