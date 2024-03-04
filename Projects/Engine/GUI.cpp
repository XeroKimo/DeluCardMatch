
#include <vector>

module DeluEngine:GUI;

namespace DeluEngine::GUI
{
	void UIElement::SetParent(UIElement* newParent, UIReparentLogic logic)
	{
		auto reparent = [this, newParent]
			{
				if(m_parent)
				{
					std::erase(m_parent->m_children, this);
				}

				try
				{
					if(newParent)
					{
						newParent->m_children.push_back(this);
					}

					m_parent = newParent;
				}
				catch(...)
				{
					m_parent->m_children.push_back(this);
				}
			};

		switch(logic)
		{
		case UIReparentLogic::KeepStaticTransform:
		{
			StaticSize oldSize = GetFrameSizeAs<StaticSize>();
			StaticPosition oldPosition = GetFramePositionAs<StaticPosition>();

			reparent();

			SetFrameSize(oldSize);
			SetFramePosition(oldPosition);
			break;
		}
		case UIReparentLogic::KeepRelativeTransform:
		{
			RelativeSize oldSize = GetLocalSizeAs<RelativeSize>();
			RelativePosition oldPosition = GetLocalPositionAs<RelativePosition>();

			reparent();

			SetLocalSize(oldSize);
			SetLocalPosition(oldPosition);
			break;
		}
		}
	}
}