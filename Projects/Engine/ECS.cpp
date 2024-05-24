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

	Engine& GetEngine(const ECS::Scene& scene)
	{
		return scene.GetExternalSystem().As<Engine>();
	}
}