module;

#include <gsl/pointers>
#include <memory>
#include <SDL2/SDL.h>
#include <array>
#include <iostream>
#include <numbers>
#include <functional>

export module DeluEngine:Engine;
import :Renderer;
import :ECS;
import :Controller;
import :Physics;
import :ForewardDeclares;
import :GUI;
import SDL2pp;
import xk.Math.Matrix;

namespace DeluEngine
{

	//using namespace xk::Math::Aliases;

	struct Box2DCallbacks : public b2Draw, public b2ContactListener
	{
		Engine* engine;

	private:
		inline static const std::array<xk::Math::Aliases::Vector2, 16> baseCirclePoints
		{
			xk::Math::Aliases::Vector2{ std::cos(0 * std::numbers::pi * 2 / 15.f), std::sin(0 * std::numbers::pi * 2 / 15.f) },
			xk::Math::Aliases::Vector2{ std::cos(1 * std::numbers::pi * 2 / 15.f), std::sin(1 * std::numbers::pi * 2 / 15.f) },
			xk::Math::Aliases::Vector2{ std::cos(2 * std::numbers::pi * 2 / 15.f), std::sin(2 * std::numbers::pi * 2 / 15.f) },
			xk::Math::Aliases::Vector2{ std::cos(3 * std::numbers::pi * 2 / 15.f), std::sin(3 * std::numbers::pi * 2 / 15.f) },
			xk::Math::Aliases::Vector2{ std::cos(4 * std::numbers::pi * 2 / 15.f), std::sin(4 * std::numbers::pi * 2 / 15.f) },
			xk::Math::Aliases::Vector2{ std::cos(5 * std::numbers::pi * 2 / 15.f), std::sin(5 * std::numbers::pi * 2 / 15.f) },
			xk::Math::Aliases::Vector2{ std::cos(6 * std::numbers::pi * 2 / 15.f), std::sin(6 * std::numbers::pi * 2 / 15.f) },
			xk::Math::Aliases::Vector2{ std::cos(7 * std::numbers::pi * 2 / 15.f), std::sin(7 * std::numbers::pi * 2 / 15.f) },
			xk::Math::Aliases::Vector2{ std::cos(8 * std::numbers::pi * 2 / 15.f), std::sin(8 * std::numbers::pi * 2 / 15.f) },
			xk::Math::Aliases::Vector2{ std::cos(9 * std::numbers::pi * 2 / 15.f), std::sin(9 * std::numbers::pi * 2 / 15.f) },
			xk::Math::Aliases::Vector2{ std::cos(10 * std::numbers::pi * 2 / 15.f), std::sin(10 * std::numbers::pi * 2 / 15.f) },
			xk::Math::Aliases::Vector2{ std::cos(11 * std::numbers::pi * 2 / 15.f), std::sin(11 * std::numbers::pi * 2 / 15.f) },
			xk::Math::Aliases::Vector2{ std::cos(12 * std::numbers::pi * 2 / 15.f), std::sin(12 * std::numbers::pi * 2 / 15.f) },
			xk::Math::Aliases::Vector2{ std::cos(13 * std::numbers::pi * 2 / 15.f), std::sin(13 * std::numbers::pi * 2 / 15.f) },
			xk::Math::Aliases::Vector2{ std::cos(14 * std::numbers::pi * 2 / 15.f), std::sin(14 * std::numbers::pi * 2 / 15.f) },
			xk::Math::Aliases::Vector2{ std::cos(15 * std::numbers::pi * 2 / 15.f), std::sin(15 * std::numbers::pi * 2 / 15.f) },
		};
		/// Draw a closed polygon provided in CCW order.
		void DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) override;

		/// Draw a solid closed polygon provided in CCW order.
		void DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) override;

		/// Draw a circle.
		void DrawCircle(const b2Vec2& center, float radius, const b2Color& color) override;

		/// Draw a solid circle.
		void DrawSolidCircle(const b2Vec2& center, float radius, const b2Vec2& axis, const b2Color& color) override;

		/// Draw a line segment.
		void DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color) override;

		/// Draw a transform. Choose your own length scale.
		/// @param xf a transform.
		void DrawTransform(const b2Transform& xf) override;

		/// Draw a point.
		void DrawPoint(const b2Vec2& p, float size, const b2Color& color) override;

		void BeginContact(b2Contact* contact) override;

		void EndContact(b2Contact* contact) override;

		void PreSolve(b2Contact* contact, const b2Manifold* oldManifold) override;

		void PostSolve(b2Contact* contact, const b2ContactImpulse* impulse) override;
	};

	export struct Engine
	{
		SDL2pp::unique_ptr<SDL2pp::Window> window;
		Renderer renderer;
		Controller controller;
		Experimental::ControllerContextManager controllerContext;
		GUI::GUIEngine guiEngine;
		b2World physicsWorld{ {0, -9.8f } };
		std::unique_ptr<Scene> scene;
		Box2DCallbacks box2DCallbacks;
		bool running = true;
		std::function<void(ECS::Scene&)> queuedScene;

		template<std::invocable<ECS::Scene&> InitFunc>
		void CreateScene(InitFunc&& init)
		{
			scene = std::make_unique<Scene>(this, init);
		}

		void ProcessEvent(const SDL2pp::Event& event)
		{
			switch (event.type)
			{
			case SDL_KEYDOWN:
			case SDL_KEYUP:
				UpdateController(controller, event.key);
				break;
			}
		}
	};

	void Box2DCallbacks::DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
	{
		engine->renderer.GetDebugRenderer().SetDrawColor({ { color.r, color.g, color.b, color.a } });
		const b2Vec2* p1 = vertices, * p2 = vertices + 1;
		for (; p2 != vertices + vertexCount; p1++, p2++)
		{
			engine->renderer.GetDebugRenderer().DrawLine({ p1->x, p1->y }, { p2->x, p2->y });
		}
		engine->renderer.GetDebugRenderer().DrawLine({ p1->x, p1->y }, { vertices->x, vertices->y });
	}

	void DeluEngine::Box2DCallbacks::DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
	{
		engine->renderer.GetDebugRenderer().SetDrawColor({ { color.r, color.g, color.b, color.a } });
		const b2Vec2* p1 = vertices, * p2 = vertices + 1;
		for (; p2 != vertices + vertexCount; p1++, p2++)
		{
			engine->renderer.GetDebugRenderer().DrawLine({ p1->x, p1->y }, { p2->x, p2->y });
		}
		engine->renderer.GetDebugRenderer().DrawLine({ p1->x, p1->y }, { vertices->x, vertices->y });
	}
	void DeluEngine::Box2DCallbacks::DrawCircle(const b2Vec2& center, float radius, const b2Color& color)
	{
		engine->renderer.GetDebugRenderer().SetDrawColor({ { color.r, color.g, color.b, color.a } });
		for (auto p1 = baseCirclePoints.begin(), p2 = baseCirclePoints.begin() + 1; p2 != baseCirclePoints.end(); p1++, p2++)
		{
			engine->renderer.GetDebugRenderer().DrawLine((*p1) * radius + xk::Math::Aliases::Vector2{ center.x, center.y }, (*p2) * radius + xk::Math::Aliases::Vector2{ center.x, center.y });
		}
		engine->renderer.GetDebugRenderer().DrawLine(baseCirclePoints.back() * radius + xk::Math::Aliases::Vector2{ center.x, center.y }, baseCirclePoints.front() * radius + xk::Math::Aliases::Vector2{ center.x, center.y });
	}
	void DeluEngine::Box2DCallbacks::DrawSolidCircle(const b2Vec2& center, float radius, const b2Vec2& axis, const b2Color& color)
	{
		engine->renderer.GetDebugRenderer().SetDrawColor({ { color.r, color.g, color.b, color.a } });
		for (auto p1 = baseCirclePoints.begin(), p2 = baseCirclePoints.begin() + 1; p2 != baseCirclePoints.end(); p1++, p2++)
		{
			engine->renderer.GetDebugRenderer().DrawLine((*p1) * radius + xk::Math::Aliases::Vector2{ center.x, center.y }, (*p2) * radius + xk::Math::Aliases::Vector2{ center.x, center.y });
		}
		engine->renderer.GetDebugRenderer().DrawLine(baseCirclePoints.back() * radius + xk::Math::Aliases::Vector2{ center.x, center.y }, baseCirclePoints.front() * radius + xk::Math::Aliases::Vector2{ center.x, center.y });
	}
	void DeluEngine::Box2DCallbacks::DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color)
	{
		engine->renderer.GetDebugRenderer().SetDrawColor({ { color.r, color.g, color.b, color.a } });
		engine->renderer.GetDebugRenderer().DrawLine(xk::Math::Aliases::Vector2{ p1.x, p1.y }, xk::Math::Aliases::Vector2{ p2.x, p2.y });
	}
	void DeluEngine::Box2DCallbacks::DrawTransform(const b2Transform& xf)
	{

	}
	void DeluEngine::Box2DCallbacks::DrawPoint(const b2Vec2& p, float size, const b2Color& color)
	{
		engine->renderer.GetDebugRenderer().SetDrawColor({ { color.r, color.g, color.b, color.a } });
	}
	void DeluEngine::Box2DCallbacks::BeginContact(b2Contact* contact)
	{
		std::cout << "Contact detected\n";
	}
	void Box2DCallbacks::EndContact(b2Contact* contact)
	{
		std::cout << "Contact ended\n";
	}
	void Box2DCallbacks::PreSolve(b2Contact* contact, const b2Manifold* oldManifold)
	{
	}
	void Box2DCallbacks::PostSolve(b2Contact* contact, const b2ContactImpulse* impulse)
	{
	}
}