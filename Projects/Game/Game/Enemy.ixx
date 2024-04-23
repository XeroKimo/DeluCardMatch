module;

#include <iostream>
#include <vector>
#include <SDL2/SDL.h>
#include <optional>

export module Enemy;
import ECS;
import DeluEngine;
import SDL2pp;
import LevelStrip.Event.EnemySpawn;
import xk.Math.Matrix;
import xk.Math.CatmullRomSpline;

using namespace xk::Math::Aliases;
using xk::Math::CatmullRomSpline;
using xk::Math::LinearSpline;
using xk::Math::LinearSegment;

void DrawSpline(DeluEngine::DebugRenderer& renderer, const CatmullRomSpline<float>& spline, size_t sampleCounts)
{
	for (size_t i = 0; i < sampleCounts - 1; i++)
	{
		auto p1 = spline.Interpolate(static_cast<float>(i) / sampleCounts);
		auto p2 = spline.Interpolate(static_cast<float>(i + 1) / sampleCounts);
		renderer.DrawLine(p1, p2);
	}
}

namespace Delu
{
	export class Enemy : public DeluEngine::GameObject
	{
		Data<EnemySpawnEvent> m_data;
		ECS::UniqueObject<DeluEngine::SpriteObject> m_sprite;
		CatmullRomSpline<float> spline;
		float distanceAlongSpline = 0;
	public:
		using SceneDestructorMemberList = ECS::SceneDestructorMemberListHelper<&Enemy::m_sprite>;
		using ObjectBaseClasses = ECS::ObjectBaseClassesHelper<DeluEngine::GameObject>;

	public:
		Enemy(const ECS::ObjectInitializer& initializer, const ECS::UserGameObjectInitializer& goInitializer, Data<EnemySpawnEvent> data) :
			//SceneAware(initializer.scene),
			DeluEngine::GameObject(initializer, goInitializer),
			m_sprite(NewObject<DeluEngine::SpriteObject>(ECS::UserGameObjectInitializer{}, DeluEngine::SpriteObject::ConstructorParams{ m_data.sprite })),
			m_data(data)
		{
			m_sprite->SetParent(this);
			std::cout << "Enemy Spawned\n";

			spline.points.push_back({ 134, 54 });
			spline.points.push_back({ 300, 300 });
			spline.points.push_back({ 500, 500 });
			//spline.points.push_back({ 700, 700});
			spline.points.push_back({ 543, 324 });
		}

	public:
		void Update(float deltaTime) override
		{
			constexpr float speed = 100;
			distanceAlongSpline += deltaTime * speed;
			Transform().Position() = spline.InterpolateDistance(distanceAlongSpline);
		}

		void DebugDraw(DeluEngine::DebugRenderer& renderer) override
		{			
			renderer.SetDrawColor({ { 255, 0, 0, 1 } });
			DrawSpline(renderer, spline, 100);
		}
	};
}