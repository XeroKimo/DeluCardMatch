module;


export module DeluEngine:Physics;
export import <box2d/box2d.h>;
import :ECS;
import xk.Math.Matrix;
import xk.Math.Angles;

namespace DeluEngine
{
	export struct BodyDef
	{
		b2BodyType type = b2_staticBody;
		b2Vec2 linearVelocity = { 0, 0 };
		float angularVelocity = 0;
		float linearDamping = 0;
		float angularDamping = 0;
		bool allowSleep = true;
		bool awake = true;
		bool fixedRotation = false;
		bool bullet = false;
		bool enabled = true;
		float gravityScale = 1;

		operator b2BodyDef() const 
		{
			b2BodyDef def;
			def.type = type;
			def.linearVelocity = linearVelocity;
			def.angularVelocity = angularVelocity;
			def.linearDamping = linearDamping;
			def.angularDamping = angularDamping;
			def.allowSleep = allowSleep;
			def.awake = awake;
			def.fixedRotation = fixedRotation;
			def.bullet = bullet;
			def.enabled = enabled;
			def.gravityScale = gravityScale;

			return def;
		}
	};

	export struct Engine;
	//Assumes to be derived from DeluEngine::GameObject
	export template<class DerivedSelf>
	class PhysicsContainer
	{
	private:
		b2Body* m_body = nullptr;

	public:
		PhysicsContainer() = default;
		PhysicsContainer(BodyDef def)
		{
			b2BodyDef internalDef = def;
			internalDef.position = { Self().Transform().Position().GetValue().X(), Self().Transform().Position().GetValue().Y() };
			internalDef.angle = Self().Transform().Rotation().GetValue()._value;
			b2BodyUserData data;
			data.pointer = reinterpret_cast<uintptr_t>(static_cast<GameObject*>(&Self()));
			internalDef.userData = data;

			m_body = GetPhysicsWorld().CreateBody(&internalDef);
		}

		~PhysicsContainer()
		{
			if (m_body)
			{
				GetPhysicsWorld().DestroyBody(m_body);
			}
		}

	public:
		b2Fixture* AddFixture(const b2Shape& shape, float density = 0)
		{
			b2FixtureDef def; 
			def.shape = &shape;
			def.density = density;
			return AddFixture(def);
		}

		b2Fixture* AddFixture(const b2FixtureDef& fixture)
		{
			CreateDefaultBodyIfNull();
			return m_body->CreateFixture(&fixture);
		}

		void RemoveFixture(const b2FixtureDef* fixture)
		{
			if (!m_body)
				return;
			return m_body->DestroyFixture(fixture);
		}

		void SyncTransformGameObjectToBody()
		{
			if (!m_body)
				return;
			m_body->SetTransform({ Self().Transform().Position().GetValue().X(), Self().Transform().Position().GetValue().Y() }, Self().Transform().Rotation().GetValue()._value);
		}

		void SyncTransformBodyToGameObject()
		{
			if (!m_body)
				return;
			b2Transform t = m_body->GetTransform();
			Self().Transform().Position() = xk::Math::Vector<float, 2>{ t.p.x, t.p.y };
			Self().Transform().Rotation() = xk::Math::Degree<float>{ t.q.GetAngle() };
		}

	private:
		void CreateDefaultBodyIfNull()
		{
			if (!m_body)
			{
				b2BodyDef def;
				b2BodyUserData data;

				def.position = { Self().Transform().Position().GetValue().X(), Self().Transform().Position().GetValue().Y() };
				def.angle = Self().Transform().Rotation().GetValue()._value;
				data.pointer = reinterpret_cast<uintptr_t>(static_cast<GameObject*>(&Self()));
				def.userData = data;
				m_body = GetPhysicsWorld().CreateBody(&def);
			}
		}
		DerivedSelf& Self() { return static_cast<DerivedSelf&>(*this); }
		const DerivedSelf& Self() const { return static_cast<DerivedSelf&>(*this); }
		b2World& GetPhysicsWorld();
	};

	template<class DerivedSelf>
	b2World& PhysicsContainer<DerivedSelf>::GetPhysicsWorld() { return Self().GetEngine().physicsWorld; }

	export class PhysicsGameObject : public GameObject, public PhysicsContainer<PhysicsGameObject>
	{
	private:
		bool m_userTransformChanged = true;

	public:
		using ObjectBaseClasses = ECS::ObjectBaseClassesHelper<GameObject>;

	public:
		PhysicsGameObject(const ECS::ObjectInitializer& objInitializer, const ECS::UserGameObjectInitializer& initializer, BodyDef bodyDef = {}) :
			GameObject(objInitializer, initializer),
			PhysicsContainer<PhysicsGameObject>(bodyDef)
		{

		}

		void Update(float deltaTime) override
		{
			if (m_userTransformChanged)
			{
				SyncTransformGameObjectToBody();
				m_userTransformChanged = false;
			}
			else
			{
				SyncTransformBodyToGameObject();
				m_userTransformChanged = false;
			}
		}

		ECS::LocalTransformProxy LocalTransform() override
		{
			m_userTransformChanged = true;
			return ECS::GameObject::LocalTransform();
		}

		ECS::WorldTransformProxy Transform() override
		{
			m_userTransformChanged = true;
			return ECS::GameObject::Transform();
		}

		void SetParent(ECS::GameObject* parent) override
		{
			m_userTransformChanged = true;
			ECS::GameObject::SetParent(parent);
		}
	};

	template<>
	b2World& PhysicsContainer<PhysicsGameObject>::GetPhysicsWorld();
};