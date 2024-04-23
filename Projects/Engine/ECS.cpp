module;

#include <gsl/pointers>

module DeluEngine:ECS;
import ECS;
import :Engine;

namespace DeluEngine
{
	SceneGUISystem::~SceneGUISystem()
	{
		m_systemOwnedElements.clear();
		for(auto& ptr : m_systemOwnedFrames)
		{
			std::erase(GetScene().GetEngine().guiEngine.frames, ptr.get());
		}
	}

	GUI::UIFrame& SceneGUISystem::NewPersistentFrame()
	{
		m_systemOwnedFrames.push_back(std::make_unique<GUI::UIFrame>());
		GetScene().GetEngine().guiEngine.frames.push_back(m_systemOwnedFrames.back().get());
		return *m_systemOwnedFrames.back();
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