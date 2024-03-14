module;

#include <vector>
#include <variant>
#include <concepts>
#include <iostream>
#include <SDL2/SDL.h>

module DeluEngine:GUI;
import :Renderer;
import SDL2pp;

namespace DeluEngine::GUI
{
	int UIElement::HandleEvent(const Event& event)
	{
		static bool oneTimeHoverCheck = true;
		static bool oneTimeHeldCheck = true;
		return std::visit([&, this](const auto& underlyingEvent) -> int
		{
			using underlying_type = std::remove_cvref_t<decltype(underlyingEvent)>;
			if constexpr(std::same_as<underlying_type, MouseEvent>)
			{
				if(!debugEnableRaytrace)
					return underlyingEvent.unhandledCode;

				//underlyingEvent.renderer->backend->CopyEx(texture.get(), std::nullopt, GUI::GetRect(*this), 0, SDL2pp::FPoint{ 0, 0 }, SDL2pp::RendererFlip::SDL_FLIP_NONE);
				if(underlyingEvent.type == MouseEventType::Overlap)
				{
					std::cout << "Overlapped: " << debugName << "\n";
				}
				else if(underlyingEvent.type == MouseEventType::Unoverlap)
				{
					std::cout << "Uoverlapped: " << debugName << "\n";
					oneTimeHoverCheck = true;
				}
				else if(underlyingEvent.type == MouseEventType::Hover && oneTimeHoverCheck)
				{
					std::cout << "Hover: " << debugName << "\n";
					oneTimeHeldCheck = true;
					oneTimeHoverCheck = false;
				}

				if(underlyingEvent.action)
				{
					if(*underlyingEvent.action == MouseClickType::Pressed)
					{
						std::cout << "Pressed: " << debugName << "\n";
					}
					else if(*underlyingEvent.action == MouseClickType::Released)
					{
						std::cout << "Released: " << debugName << "\n";
						oneTimeHeldCheck = true;
					}
					else if(*underlyingEvent.action == MouseClickType::Clicked)
					{
						std::cout << "Clicked: " << debugName << "\n";
					}
					else if(*underlyingEvent.action == MouseClickType::Held && oneTimeHeldCheck)
					{
						std::cout << "Held: " << debugName << "\n";
						oneTimeHeldCheck = false;
					}
				}


				return underlyingEvent.handledCode;
			}
			return defaultSuccessCode;
		}, event);
	}

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

	static SDL2pp::FRect GetSDLRect(const DeluEngine::GUI::UIElement& element)
	{
		DeluEngine::GUI::Rect baseRect = element.GetRect();
		SDL2pp::FRect rect;
		const Vector2 size = element.GetFrameSizeAs<DeluEngine::GUI::AbsoluteSize>().value;
		const Vector2 pivotOffsetFlip = { 0, size.Y() };
		const Vector2 sdl2YFlip = { 0, element.GetFrame().GetSize().Y() };
		const Vector2 position = xk::Math::HadamardProduct(baseRect.bottomLeft.value + pivotOffsetFlip, Vector2{ 1, -1 }) + sdl2YFlip;

		rect.x = position.X();
		rect.y = position.Y();
		rect.w = size.X();
		rect.h = size.Y();
		return rect;
	}

	int Image::HandleEvent(const Event& event)
	{
		return std::visit([&, this](const auto& underlyingEvent) -> int
			{
				using underlying_type = std::remove_cvref_t<decltype(underlyingEvent)>;
				if constexpr(std::same_as<underlying_type, DrawEvent>)
				{
					underlyingEvent.renderer->backend->CopyEx(texture.get(), std::nullopt, GetSDLRect(*this), 0, SDL2pp::FPoint{ 0, 0 }, SDL2pp::RendererFlip::SDL_FLIP_NONE);

					return defaultSuccessCode;
				}
				else
				{
					return UIElement::HandleEvent(event);
				}
				return defaultSuccessCode;
			}, event);
	}

	void ProcessEvent(GUIEngine& engine, const SDL2pp::Event& event, Vector2 windowSize)
	{
		switch(event.type)
		{
		case SDL2pp::EventType::SDL_MOUSEMOTION:
		{
			engine.mousePosition.value = Vector2{ event.motion.x, windowSize.Y() - event.motion.y };
			break;
		}
		case SDL2pp::EventType::SDL_MOUSEBUTTONDOWN:
		{
			if(event.button.button == SDL_BUTTON_LEFT && engine.hoveredElement)
			{
				engine.leftClickPressed = true;
				engine.hoveredElement->HandleEvent(DeluEngine::GUI::MouseEvent{ DeluEngine::GUI::MouseEventType::Hover, DeluEngine::GUI::MouseClickType::Pressed });
				engine.initialLeftClickedElement = engine.hoveredElement;
				//std::cout << "Button clicked\n";
			}
			break;
		}
		case SDL2pp::EventType::SDL_MOUSEBUTTONUP:
		{
			if(event.button.button == SDL_BUTTON_LEFT && engine.hoveredElement)
			{
				engine.leftClickPressed = false;
				if(engine.hoveredElement == engine.initialLeftClickedElement)
				{
					engine.hoveredElement->HandleEvent(DeluEngine::GUI::MouseEvent{ DeluEngine::GUI::MouseEventType::Hover, DeluEngine::GUI::MouseClickType::Clicked });
				}
				engine.hoveredElement->HandleEvent(DeluEngine::GUI::MouseEvent{ DeluEngine::GUI::MouseEventType::Hover, DeluEngine::GUI::MouseClickType::Released });
				engine.initialLeftClickedElement = nullptr;
				//std::cout << "Button Released\n";
			}
			break;
		}
		default:
			break;
		}
	}
}