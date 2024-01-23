module;

#include <gsl/pointers>

export module Delu.Bullet;
import DeluEngine;
import xk.Math.Matrix;

namespace Delu
{
	export struct BulletData
	{
		float speed;
		std::shared_ptr<DeluEngine::SpriteData> sprite;
	};

	export class Bullet : public DeluEngine::GameObject
	{
	public:
		struct ConstructorParams
		{
			const BulletData& data;
			xk::Math::Vector<float, 2> direction;
		};

	private:
		BulletData m_data;
		ECS::UniqueObject<DeluEngine::SpriteObject> m_sprite;
		xk::Math::Vector<float, 2> m_direction;

	public:
		using SceneDestructorMemberList = ECS::SceneDestructorMemberListHelper<&Bullet::m_sprite>;
		using ObjectBaseClasses = ECS::ObjectBaseClassesHelper<DeluEngine::GameObject>;

	public:
		Bullet(const ECS::ObjectInitializer& initializer, const ECS::UserGameObjectInitializer& goInitializer, ConstructorParams params) :
			//SceneAware(initializer.scene),
			DeluEngine::GameObject(initializer, goInitializer),
			m_data(params.data),
			m_sprite(NewObject<DeluEngine::SpriteObject>(ECS::UserGameObjectInitializer{}, DeluEngine::SpriteObject::ConstructorParams{ params.data.sprite })),
			m_direction(params.direction)
		{
			m_sprite->SetParent(this);
		}

		void Update(float deltaTime) override
		{
			Transform().Position() += m_direction * m_data.speed * deltaTime;
		}
	};
}