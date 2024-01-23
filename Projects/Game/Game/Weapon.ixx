module;

#include <gsl/pointers>
#include <optional>
export module Delu.Weapon;
import DeluEngine;
export import Delu.Bullet;
import xk.Math.Matrix;

//Template for making new gameObject classes

namespace Delu
{
	export class Weapon;
	
	export struct WeaponData
	{
		struct BulletConfiguration
		{
			xk::Math::Degree<float> initialBulletRotation;
			BulletData data;
		};
		int bulletsPerSecond = 0;
		xk::Math::Vector<float, 2> fireDirection;

		BulletConfiguration bulletConfig;
	};
}

namespace Delu
{
	class Weapon : public DeluEngine::GameObject
	{
	public:
		struct ConstructorParams
		{
			WeaponData weaponData;
		};

	private:
		float m_timeAccumulator = 0;

	public:
		bool firing = false;
		WeaponData weaponData;

	public:
		using ObjectBaseClasses = ECS::ObjectBaseClassesHelper<DeluEngine::GameObject>;
 
	public:
		//Uncomment either the following
		//using ECS::GameObjectClass::GameObjectClass;
		//or
		Weapon(const ECS::ObjectInitializer& initializer, const ECS::UserGameObjectInitializer& goInitializer, ConstructorParams params) :
			//SceneAware(initializer.scene),
			DeluEngine::GameObject(initializer, goInitializer),
			weaponData(params.weaponData)
		{
		}

		void Update(float deltaTime) override
		{
			m_timeAccumulator -= deltaTime;
			if (m_timeAccumulator < 0)
			{
				m_timeAccumulator = 0;
			}
			if (firing)
			{
				if (m_timeAccumulator <= 0)
				{
					m_timeAccumulator = FireTime();
					std::shared_ptr<Bullet> b = NewObject<Bullet>(ECS::UserGameObjectInitializer{}, Bullet::ConstructorParams{ weaponData.bulletConfig.data, weaponData.fireDirection }).lock();
					b->Transform().Position() = Transform().Position();
					b->Transform().Rotation() = weaponData.bulletConfig.initialBulletRotation;
				}
			}
		}

	private:
		float FireTime()
		{
			return 1.f / static_cast<float>(weaponData.bulletsPerSecond);
		}
	}; 
}