#include "SimpleAPI/Rigidbody.h"

#include "Components/ComponentRigidbody.h"
#include "SimpleAPI/Transform.h"
#include "SimpleAPI/Math.h"

#include <iostream>

namespace Vortx
{

	bool Rigidbody::Exists()
	{
		return entity.HasComponent<RigidbodyData>();
	}

	void Rigidbody::_InternalCreate()
	{
		entity.AddComponent<RigidbodyData>();
	}

	Rigidbody::Rigidbody(Entity entity, bool create) : BaseComponent(entity)
	{
		_InternalCreate();
	}

	RigidbodyData& Rigidbody::Data()
	{
		return entity.GetComponent<RigidbodyData>();
	}

	GameObject Rigidbody::GetGameObject()
	{
		return GameObject(entity);
	}

	BodyType Rigidbody::GetBodyType()
	{
		return Data().bodyType;
	}

	void Rigidbody::SetBodyType(BodyType type)
	{
		Data().bodyType = type;
		MarkMassDirty();
	}

	void Rigidbody::MarkMassDirty()
	{
		Data().massDirty = 1;
	}

	float Rigidbody::GetMass()
	{
		return Data().mass;
	}

	void Rigidbody::SetMass(float mass)
	{
		RigidbodyData& data = Data();
		data.mass = mass;
		data.massDirty = 0;
		SetAutoMass(0);
	}

	void Rigidbody::SetAutoMass(bool enabled)
	{
		Data().useAutoMass = enabled;
	}

	bool Rigidbody::GetAutoMass()
	{
		return Data().useAutoMass;
	}

	float Rigidbody::GetInverseMass()
	{
		return Data().inverseMass;
	}

	float Rigidbody::GetInertia()
	{
		return Data().inertia;
	}

	glm::vec2 Rigidbody::GetVelocity()
	{
		return Data().velocity;
	}

	void Rigidbody::SetVelocity(const glm::vec2& velocity)
	{
		Data().velocity = velocity;
	}

	float Rigidbody::GetAngularVelocity()
	{
		return Data().angularVelocity;
	}

	void Rigidbody::SetAngularVelocity(float angularVelocity)
	{
		Data().angularVelocity = angularVelocity;
	}


	float Rigidbody::GetGravityScale()
	{
		return Data().gravityScale;
	}

	void Rigidbody::SetGravityScale(float scale)
	{
		Data().gravityScale = scale;
	}


	float Rigidbody::GetLinearDamping()
	{
		return Data().linearDamping;
	}

	void Rigidbody::SetLinearDamping(float damping)
	{
		Data().linearDamping = damping;
	}

	float Rigidbody::GetAngularDamping()
	{
		return Data().angularDamping;
	}

	void Rigidbody::SetAngularDamping(float damping)
	{
		Data().angularDamping = damping;
	}


	bool Rigidbody::IsFixedRotation()
	{
		return Data().fixedRotation;
	}

	void Rigidbody::SetFixedRotation(bool fixedRotation)
	{
		Data().fixedRotation = fixedRotation;
	}

	glm::vec2 Rigidbody::GetLocalCenterOfMass()
	{
		return Data().centerOfMass;
	}

	glm::vec2 Rigidbody::GetWorldCenterOfMass()
	{
		Transform t(entity);
		
		return t.Position() + Math::Rotate(Data().centerOfMass, t.Rotation()) * t.Scale();
	}

	bool Rigidbody::IsSleepingAllowed()
	{
		return Data().allowSleep;
	}

	void Rigidbody::SetSleepingAllowed(bool allowed)
	{
		Data().allowSleep = allowed;
		if (!allowed) WakeUp();
	}

	bool Rigidbody::IsAwake()
	{
		return Data().isAwake;
	}

	void Rigidbody::WakeUp()
	{
		RigidbodyData& data = Data();
		data.isAwake = true;
		data.sleepTime = 0.0f;
	}

	void Rigidbody::Sleep()
	{
		RigidbodyData& data = Data();
		if (!data.allowSleep || data.bodyType != BodyType::Dynamic) return;

		data.isAwake = false;
		data.sleepTime = 0.0f;
		data.velocity = glm::vec2(0.0f);
		data.angularVelocity = 0.0f;
	}

	void Rigidbody::ApplyForce(const glm::vec2& force)
	{
		RigidbodyData& data = Data();
		if (data.bodyType != BodyType::Dynamic) return;

		data.forceAccum += force;
		WakeUp();
	}

	void Rigidbody::ApplyForceAtPoint(const glm::vec2& force, const glm::vec2& worldPoint)
	{
		RigidbodyData& data = Data();
		if (data.bodyType != BodyType::Dynamic) return;

		data.forceAccum += force;

		glm::vec2 r = worldPoint - GetWorldCenterOfMass();
		data.torqueAccum += Math::Cross(r, force);

		WakeUp();
	}

	void Rigidbody::ApplyTorque(float torque)
	{
		RigidbodyData& data = Data();
		if (data.bodyType != BodyType::Dynamic || data.fixedRotation) return;

		data.torqueAccum += torque;
		WakeUp();
	}

	
	void Rigidbody::ApplyImpulse(const glm::vec2& impulse)
	{
		RigidbodyData& data = Data();
		if (data.bodyType != BodyType::Dynamic) return;

		data.velocity += impulse * data.inverseMass;
		WakeUp();
	}

	void Rigidbody::ApplyImpulseAtPoint(const glm::vec2& impulse, const glm::vec2& worldPoint)
	{
		RigidbodyData& data = Data();
		if (data.bodyType != BodyType::Dynamic) return;

		data.velocity += impulse * data.inverseMass;

		glm::vec2 r = worldPoint - GetWorldCenterOfMass();
		data.angularVelocity += data.inverseInertia * Math::Cross(r, impulse);

		WakeUp();
	}

	void Rigidbody::ApplyAngularImpulse(float impulse)
	{
		RigidbodyData& data = Data();
		if (data.bodyType != BodyType::Dynamic || data.fixedRotation) return;

		data.angularVelocity += data.inverseInertia * impulse;
		WakeUp();
	}

	glm::vec2 Rigidbody::GetPointVelocity(const glm::vec2& worldPoint)
	{
		RigidbodyData& data = Data();
		glm::vec2 r = worldPoint - GetWorldCenterOfMass();
		return data.velocity + Math::Cross(data.angularVelocity, r);
	}

}
