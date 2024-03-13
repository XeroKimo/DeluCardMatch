module;

module DeluEngine:ECS;
import :Engine;

namespace DeluEngine
{
	GUI::UIFrame& SceneGUISystem::NewPersistentFrame()
	{
		m_systemOwnedFrames.push_back(std::make_unique<GUI::UIFrame>());
		GetScene().GetEngine().guiEngine.frames.push_back(m_systemOwnedFrames.back().get());
		return *m_systemOwnedFrames.back();
	}
}