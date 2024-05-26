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
import :ForwardDeclares;
import :EngineAware;
import :GUI;
import SDL2pp;

namespace DeluEngine
{
	export Engine& GetEngine(const ECS::Scene& scene);

	//export class GameObject;
	//export class DebugRenderer;
	//export class Scene;

	//export class GameObject : public ECS::GameObject
	//{
	//public:
	//	using ObjectBaseClasses = ECS::ObjectBaseClassesHelper<ECS::GameObject>;
	//	//using ECS::GameObject::GameObject;
	//	GameObject(const ECS::ObjectInitializer& initializer, const ECS::UserGameObjectInitializer& goInitializer) :
	//		ECS::GameObject(initializer, goInitializer)
	//	{
	//	}

	//public:
	//	virtual void Update(float deltaTime) = 0;
	//	virtual void DebugDraw(DebugRenderer& renderer) {}

	//public:
	//	Engine& GetEngine() const noexcept;
	//	Scene& GetScene() const noexcept;
	//};

	//export class SceneInit
	//{

	//public:
	//	virtual void OnInit(Scene& scene) const {}

	//	void operator()(Scene& scene) const { OnInit(scene); }
	//};

	export class SceneSystem : public ECS::SceneSystem
	{
	public:		
		SceneSystem(const gsl::not_null<ECS::Scene*> scene) :
			ECS::SceneSystem{ scene }
		{

		}

	public:
		Engine& GetEngine() const
		{
			return DeluEngine::GetEngine(GetScene());
		}
	};

	export class SceneGUISystem : public SceneSystem
	{
	private:
		std::vector<DeluEngine::GUI::UniqueHandle<GUI::UIElement>> m_systemOwnedElements;

	public:
		SceneGUISystem(const gsl::not_null<ECS::Scene*> scene) :
			SceneSystem{ scene }
		{

		}

		~SceneGUISystem();

	public:
		void AddPersistentElement(DeluEngine::GUI::UniqueHandle<GUI::UIElement> element)
		{
			m_systemOwnedElements.push_back(std::move(element));
		}
	};

	//Scene& GameObject::GetScene() const noexcept
	//{
	//	return static_cast<Scene&>(ECS::GameObject::GetScene());
	//}



	//Scene& SceneSystem::GetScene() const noexcept
	//{
	//	return static_cast<Scene&>(ECS::SceneSystem::GetScene());
	//}
}
