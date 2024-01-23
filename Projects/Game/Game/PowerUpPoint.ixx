module;

#include <gsl/pointers>

export module DeluGame.PowerUpPoint;
import DeluEngine;
import xk.Math.Matrix;

namespace Delu
{
	export class PowerUpPoint : public DeluEngine::PhysicsGameObject
	{
	private:
		ECS::UniqueObject<DeluEngine::SpriteObject> m_sprite;

	public:
		using SceneDestructorMemberList = ECS::SceneDestructorMemberListHelper<&PowerUpPoint::m_sprite>;
		using ObjectBaseClasses = ECS::ObjectBaseClassesHelper<DeluEngine::PhysicsGameObject>;

	public:
		PowerUpPoint(const ECS::ObjectInitializer& initializer, const ECS::UserGameObjectInitializer& goInitializer/*, std::shared_ptr<DeluEngine::SpriteData> sprite*/) :
			//SceneAware(initializer.scene),
			DeluEngine::PhysicsGameObject(initializer, goInitializer,
				{
					.type = b2_dynamicBody,
					.linearVelocity = { 0, 35.f },
					.gravityScale = 1
				}),
			m_sprite(NewObject<DeluEngine::SpriteObject>(ECS::UserGameObjectInitializer{ .optionalParent = this }, DeluEngine::SpriteObject::ConstructorParams{ nullptr }))
		{
			//m_sprite->SetParent(this);
			AddFixture([] { b2CircleShape circle;
			circle.m_radius = 10.f;
			return circle;
				}(), 1.f);

		}

		void Update(float deltaTime) override
		{
			DeluEngine::PhysicsGameObject::Update(deltaTime);
		}
	};
}