module;

#include <memory>
#include <vector>
#include <string_view>
#include <gsl/pointers>
#include <functional>
#include <variant>
#include <iostream>
#include <map>
#include <string>

export module DeluGame.Player;
import ECS;
import DeluEngine;
import SDL2pp;
import xk.Math.Matrix;
import DeluGame.ControllerContextConstants;
import Delu.Bullet;
import Delu.Weapon;

using namespace xk::Math::Aliases;
namespace Delu
{
	export struct PlayerData
	{
		using ArmoryProgression = std::map<int, WeaponData>;
		float speed;
		std::shared_ptr<DeluEngine::SpriteData> sprite;
		std::vector<Vector2> weaponSockets;
		std::vector<ArmoryProgression> armoryProgression;
		int startingArmoryLevel = 0;
	};

	export class Player : public DeluEngine::PhysicsGameObject/*, public DeluEngine::PhysicsContainer<Player>*/
	{
	private:
		PlayerData m_data;
		Vector2 m_direction;
		bool m_firing;
		ECS::UniqueObject<DeluEngine::SpriteObject> m_sprite;
		std::vector<ECS::UniqueObject<Weapon>> m_weapons;
		std::vector<Weapon*> m_activatedWeapons;
		int m_armoryLevel;

	public:
		using SceneDestructorMemberList = ECS::SceneDestructorMemberListHelper<&Player::m_sprite, &Player::m_weapons>;
		using ObjectBaseClasses = ECS::ObjectBaseClassesHelper<DeluEngine::PhysicsGameObject>;

	public:
		Player(const ECS::ObjectInitializer& initializer, const ECS::UserGameObjectInitializer& goInitializer, PlayerData data) :
			//SceneAware(initializer.scene),
			DeluEngine::PhysicsGameObject(initializer, goInitializer,
				{
					.type = b2_kinematicBody,
					.allowSleep = false,
				}),
			m_data(data),
			m_sprite(NewObject<DeluEngine::SpriteObject>(ECS::UserGameObjectInitializer{}, DeluEngine::SpriteObject::ConstructorParams{ m_data.sprite }))
		{
			for (int i = 0; auto position : data.weaponSockets)
			{
				m_weapons.emplace_back(NewObject<Weapon>(ECS::UserGameObjectInitializer{}, Weapon::ConstructorParams{ {} }));
				m_weapons.back()->name = "Player Weapon " + std::to_string(i++);
				m_weapons.back()->SetParent(this);
				m_weapons.back()->LocalTransform().Position() = position;
			}
			m_activatedWeapons.reserve(m_weapons.size());
			SetArmoryLevel(data.startingArmoryLevel);
			name = "Player";
			m_sprite->name = "Player Sprite";

			m_sprite->SetParent(this);
			m_sprite->Transform().Rotation() = xk::Math::Degree{ 180.f };

			AddFixture([] { b2CircleShape circle;
				circle.m_radius = 10.f;
				return circle;
				}(), 1.f);
		}

	public:
		void SceneConstructor()
		{
			auto& playerContext = GetEngine().controllerContext.FindContext(Delu::ControllerContext::playerControlContext);

			playerContext.FindAction(Delu::ControllerContext::Actions::playerMovement).BindAxis2D(
				[this](Vector2 value)
				{
					ChangeDirection(value);
				});

			playerContext.FindAction(Delu::ControllerContext::Actions::playerFire).BindButton(
				[this](bool keyDown)
				{
					Fire(keyDown);
				});

			playerContext.FindAction(Delu::ControllerContext::Actions::playerStopFire).BindButton(
				[this](bool keyDown)
				{
					Fire(keyDown);
				});
		}

		void SceneDestructor()
		{
			auto& playerContext = GetEngine().controllerContext.FindContext(Delu::ControllerContext::playerControlContext);
			playerContext.FindAction(Delu::ControllerContext::Actions::playerMovement).BindAxis2D(std::function<void(Vector2)>{});
			playerContext.FindAction(Delu::ControllerContext::Actions::playerFire).BindButton(std::function<void(bool)>{});
			playerContext.FindAction(Delu::ControllerContext::Actions::playerStopFire).BindButton(std::function<void(bool)>{});
		}

		void Update(float deltaTime)
		{
			DeluEngine::PhysicsGameObject::Update(deltaTime);
			Transform().Position() += m_direction * m_data.speed * deltaTime;
			if (DeluEngine::Input::Pressed(DeluEngine::Key::I))
				SetArmoryLevel(m_armoryLevel + 1);
			if (DeluEngine::Input::Pressed(DeluEngine::Key::O))
				SetArmoryLevel(m_armoryLevel - 1);

		}

		void SetArmoryLevel(int level)
		{
			try
			{
				const PlayerData::ArmoryProgression& armory = m_data.armoryProgression.at(level);

				bool isCurrentlyFiring = m_activatedWeapons.empty() ? false : m_activatedWeapons.front()->firing;
				for (auto weapon : m_activatedWeapons)
				{
					weapon->firing = false;
				}
				m_activatedWeapons.clear();

				for (auto [id, weapon] : armory)
				{
					try
					{
						m_activatedWeapons.push_back(m_weapons.at(id).Get());
						m_activatedWeapons.back()->firing = isCurrentlyFiring;
						m_activatedWeapons.back()->weaponData = weapon;
					}
					catch (...)
					{
						std::cout << id << " is not a valid weapon socket\n";
					}
				}

				m_armoryLevel = level;
			}
			catch (...)
			{

			}
		}

	private:
		void Fire(bool keyDown)
		{
			for (auto weapon : m_activatedWeapons)
			{
				weapon->firing = keyDown;
			}

		}

		void ChangeDirection(Vector2 input)
		{
			m_direction = input;
		}
	};
};