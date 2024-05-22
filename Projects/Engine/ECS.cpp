module;

#include <gsl/pointers>
#include <vector>

module DeluEngine:ECS;
import ECS;
import :Engine;

namespace DeluEngine
{
	SceneGUISystem::~SceneGUISystem()
	{
		m_systemOwnedElements.clear();
	}

	Engine& Scene::GetEngine() const
	{
		return *GetExternalSystemAs<gsl::not_null<Engine*>>();
	}

	Engine& GameObject::GetEngine() const noexcept
	{
		return static_cast<const Scene&>(ECS::GameObject::GetScene()).GetEngine();
	}
}