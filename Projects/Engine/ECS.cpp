module;

module DeluEngine:ECS;
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
}