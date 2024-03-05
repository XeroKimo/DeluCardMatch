
#include <vector>

module DeluEngine:GUI;

namespace DeluEngine::GUI
{
	void UIElement::SetParent(UIElement* newParent, UIReparentLogic logic)
	{
		auto reparent = [this, newParent]()
		{
			auto updateRootElements = [this, oldParent = m_parent, newParent]()
			{
				try
				{
					if(!newParent)
					{
						m_ownerFrame->m_rootElements.push_back(this);
					}
					else if(!oldParent && newParent)
					{
						std::erase(m_ownerFrame->m_rootElements, this);
					}
				}
				catch(...)
				{
					if(!newParent)
					{
						std::erase(m_ownerFrame->m_rootElements, this);
					}
					else if(!oldParent && newParent)
					{
						m_ownerFrame->m_rootElements.push_back(this);
					}
				}
			};
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
				updateRootElements();
				m_parent = newParent;
			}
			catch(...)
			{
				m_parent->m_children.push_back(this);
			}
		};

		switch(logic)
		{
		case UIReparentLogic::KeepAbsoluteTransform:
		{
			AbsoluteSize oldSize = GetFrameSizeAs<AbsoluteSize>();
			AbsolutePosition oldPosition = GetFramePositionAs<AbsolutePosition>();

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