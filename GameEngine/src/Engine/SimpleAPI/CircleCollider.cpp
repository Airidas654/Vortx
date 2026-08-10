#include "SimpleAPI\CircleCollider.h"

#include "Components\ComponentCircleCollider.h"
#include "SimpleAPI\Rigidbody.h"
#include "SimpleAPI\Transform.h"
#include "SimpleAPI\Math.h"

namespace Vortx
{
	bool CircleCollider::Exists()
	{
		return entity.HasComponent<CircleColliderData>();
	}

	void CircleCollider::_InternalCreate()
	{
		entity.AddComponent<CircleColliderData>();
	}

	CircleCollider::CircleCollider(Entity entity, bool create) : BaseComponent(entity)
	{
		_InternalCreate();
	}

	CircleColliderData& CircleCollider::Data()
	{
		return entity.GetComponent<CircleColliderData>();
	}

	GameObject CircleCollider::GetGameObject()
	{
		return GameObject(entity);
	}

	glm::vec2 CircleCollider::GetOffset()
	{
		return Data().offset;
	}

	void CircleCollider::SetOffset(const glm::vec2& offset)
	{
		Data().offset = offset;
	}

	float CircleCollider::GetRadius()
	{
		return Data().radius;
	}

	void CircleCollider::SetRadius(float radius)
	{
		Data().radius = radius;
	}

	bool CircleCollider::IsTrigger()
	{
		return Data().isTrigger;
	}

	void CircleCollider::SetTrigger(bool isTrigger)
	{
		Data().isTrigger = isTrigger;
	}

	float CircleCollider::GetFriction()
	{
		return Data().material.friction;
	}

	void CircleCollider::SetFriction(float friction)
	{
		Data().material.friction = friction;
	}

	float CircleCollider::GetRestitution()
	{
		return Data().material.restitution;
	}

	void CircleCollider::SetRestitution(float restitution)
	{
		Data().material.restitution = restitution;
	}

	CollisionFilter CircleCollider::GetFilter()
	{
		return Data().filter;
	}

	void CircleCollider::SetFilter(const CollisionFilter& filter)
	{
		Data().filter = filter;
	}

	float CircleCollider::GetDensity()
	{
		return Data().density;
	}

	void CircleCollider::SetDensity(float density)
	{
		Data().density = density;

		if (HasComponent<Rigidbody>()) {
			Rigidbody rg = GetComponent<Rigidbody>();

			if (rg.GetAutoMass()) {
				rg.MarkMassDirty();
			}
		}
	}

	void CircleCollider::GetWorldBounds(glm::vec2& outMin, glm::vec2& outMax)
	{
		CircleColliderData& data = Data();
		Transform t(entity);

		glm::vec2 center = t.Position();

		glm::vec2 aabbExtents
		{
			data.radius,
			data.radius
		};

		outMin = center - aabbExtents;
		outMax = center + aabbExtents;
	}
}