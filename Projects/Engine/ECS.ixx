module;

#include <vector>
#include <memory>
#include <concepts>
#include <functional>
#include <gsl/pointers>
#include <assert.h>
#include <any>

export module DeluEngine:ECS;
export import ECS;
import :EngineAware;
import SDL2pp;

namespace DeluEngine
{
	export struct Engine;

	export class GameObject;
	export class DebugRenderer;
	export class Scene;

	export class GameObject : public ECS::GameObject
	{
	public:
		using ObjectBaseClasses = ECS::ObjectBaseClassesHelper<ECS::GameObject>;
		using ECS::GameObject::GameObject;

	public:
		virtual void Update(float deltaTime) = 0;
		virtual void DebugDraw(DebugRenderer& renderer) {}

	public:
		Engine& GetEngine() const noexcept;
		Scene& GetScene() const noexcept;
	};

	export class Scene;

	export class SceneInit
	{

	public:
		virtual void OnInit(Scene& scene) const {}

		void operator()(Scene& scene) const { OnInit(scene); }
	};

	export class SceneSystem : public ECS::SceneSystem
	{
		using ECS::SceneSystem::SceneSystem;
	};

	class Scene : public ECS::Scene
	{
	private:

	public:
		template<std::invocable<ECS::Scene&> InitFunc>
		Scene(gsl::not_null<Engine*> engine, InitFunc&& init) :
			ECS::Scene(init, engine)
		{
		}

		void Update(float deltaTime)
		{
			ForEachSceneSystem([deltaTime](auto& system)
			{
				system.Update(deltaTime);
			});

			ForEachGameObject([deltaTime](ECS::Object& gameObject)
			{
				if (auto* ptr = dynamic_cast<GameObject*>(&gameObject); ptr)
					ptr->Update(deltaTime);
			});

			GetAllocator().ExecuteEvents();
		}

		void DebugDraw(DeluEngine::DebugRenderer& renderer)
		{
			ForEachGameObject([&](ECS::Object& gameObject)
			{
				if (auto* ptr = dynamic_cast<GameObject*>(&gameObject); ptr)
					ptr->DebugDraw(renderer);
			});
		}

		Engine& GetEngine() const
		{
			return *std::any_cast<gsl::not_null<Engine*>>(GetExternalSystem());
		}
	};

	Engine& GameObject::GetEngine() const noexcept
	{
		return static_cast<const Scene&>(ECS::GameObject::GetScene()).GetEngine();
	}

	Scene& GameObject::GetScene() const noexcept
	{
		return static_cast<Scene&>(ECS::GameObject::GetScene());
	}
}
