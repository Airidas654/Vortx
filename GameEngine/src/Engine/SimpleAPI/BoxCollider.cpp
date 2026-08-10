#include "SimpleAPI\BoxCollider.h"

#include "Components\ComponentBoxCollider.h"
#include "SimpleAPI\Rigidbody.h"
#include "SimpleAPI\Transform.h"
#include "SimpleAPI\Math.h"

namespace Vortx
{
	bool BoxCollider::Exists()
	{
		return entity.HasComponent<BoxColliderData>();
	}

	void BoxCollider::_InternalCreate()
	{
		entity.AddComponent<BoxColliderData>();
	}

	BoxCollider::BoxCollider(Entity entity, bool create) : BaseComponent(entity)
	{
		_InternalCreate();
	}

	BoxColliderData& BoxCollider::Data()
	{
		return entity.GetComponent<BoxColliderData>();
	}

	GameObject BoxCollider::GetGameObject()
	{
		return GameObject(entity);
	}

	glm::vec2 BoxCollider::GetOffset()
	{
		return Data().offset;
	}

	void BoxCollider::SetOffset(const glm::vec2& offset)
	{
		Data().offset = offset;
	}

	glm::vec2 BoxCollider::GetSize()
	{
		return Data().size;
	}

	void BoxCollider::SetSize(const glm::vec2& size)
	{
		Data().size = size;
	}

	bool BoxCollider::IsTrigger()
	{
		return Data().isTrigger;
	}

	void BoxCollider::SetTrigger(bool isTrigger)
	{
		Data().isTrigger = isTrigger;
	}

	float BoxCollider::GetFriction()
	{
		return Data().material.friction;
	}

	void BoxCollider::SetFriction(float friction)
	{
		Data().material.friction = friction;
	}

	float BoxCollider::GetRestitution()
	{
		return Data().material.restitution;
	}

	void BoxCollider::SetRestitution(float restitution)
	{
		Data().material.restitution = restitution;
	}

	CollisionFilter BoxCollider::GetFilter()
	{
		return Data().filter;
	}

	void BoxCollider::SetFilter(const CollisionFilter& filter)
	{
		Data().filter = filter;
	}

	float BoxCollider::GetDensity()
	{
		return Data().density;
	}

	void BoxCollider::SetDensity(float density)
	{
		Data().density = density;

		if (HasComponent<Rigidbody>()) {
			Rigidbody rg = GetComponent<Rigidbody>();

			if (rg.GetAutoMass()) {
				rg.MarkMassDirty();
			}
		}
	}

	void BoxCollider::GetWorldBounds(glm::vec2& outMin, glm::vec2& outMax)
	{
		BoxColliderData& data = Data();
		Transform t(entity);

		float rotation = t.Rotation();
		glm::vec2 scale = t.Scale();
		glm::vec2 center = t.Position() + Math::Rotate(data.offset, rotation) * scale;
		glm::vec2 halfExtents = data.size * 0.5f * scale;

		float c = fabsf(cosf(rotation));
		float s = fabsf(sinf(rotation));

		glm::vec2 aabbExtents
		{
			c * halfExtents.x + s * halfExtents.y,
			s * halfExtents.x + c * halfExtents.y
		};

		outMin = center - aabbExtents;
		outMax = center + aabbExtents;
	}
}