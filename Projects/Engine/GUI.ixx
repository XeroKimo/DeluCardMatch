module;

#include <functional>
#include <variant>
#include <algorithm>
#include <utility>
#include <vector>
#include <optional>
#include <memory>
#include <string>
#include <SDL2/SDL_ttf.h>
#include <string_view>

export module DeluEngine:GUI;
import xk.Math.Matrix;
import xk.Math.Algorithms;
import SDL2pp;
import :Renderer;


//namespace DeluEngine
//{
//	export class Renderer;
//}

namespace DeluEngine::GUI
{
	//using namespace xk::Math::Aliases;

	export struct AbsolutePosition
	{
		xk::Math::Aliases::Vector2 value;
	};

	export struct RelativePosition
	{
		xk::Math::Aliases::Vector2 value;
	};

	//Keeps the size of the element between all screen sizes
	export struct AbsoluteSize
	{
		xk::Math::Aliases::Vector2 value;
	};

	//Keeps the percentage of the screen size constant between screen sizes
	export struct RelativeSize
	{
		xk::Math::Aliases::Vector2 value;
	};

	//Keeps the aspect ratio of the element constant between screen sizes
	export struct AspectRatioRelativeSize
	{
		float ratio;
		float value;
	};

	//Keeps the edge of the element to the edge of the parent relatively constant between screen sizes
	export struct BorderConstantRelativeSize
	{
		xk::Math::Aliases::Vector2 value;
	};

	template<class T, class U> struct IsVariantMember;

	template<class T, class... Ts>
	struct IsVariantMember<T, std::variant<Ts...>> : std::bool_constant<(std::same_as<T, Ts> || ...)>
	{
	};

	template<class Ty, class VariantTy>
	concept VariantMember = (IsVariantMember<Ty, VariantTy>::value);

	export using PositionVariant = std::variant<AbsolutePosition, RelativePosition>;
	export using SizeVariant = std::variant<AbsoluteSize, RelativeSize, AspectRatioRelativeSize, BorderConstantRelativeSize>;

	export template<VariantMember<PositionVariant> ToType, VariantMember<PositionVariant> FromType>
		ToType ConvertPositionRepresentation(const FromType& position, AbsoluteSize parentSize) = delete;

	template<>
	AbsolutePosition ConvertPositionRepresentation<AbsolutePosition, AbsolutePosition>(const AbsolutePosition& position, AbsoluteSize parentSize) noexcept
	{
		return position;
	}

	template<>
	RelativePosition ConvertPositionRepresentation<RelativePosition, AbsolutePosition>(const AbsolutePosition& position, AbsoluteSize parentSize) noexcept
	{
		return RelativePosition{ xk::Math::HadamardDivision(position.value, parentSize.value) };
	}

	template<>
	AbsolutePosition ConvertPositionRepresentation<AbsolutePosition, RelativePosition>(const RelativePosition& position, AbsoluteSize parentSize) noexcept
	{
		return AbsolutePosition{ xk::Math::HadamardProduct(position.value, parentSize.value) };
	}

	template<>
	RelativePosition ConvertPositionRepresentation<RelativePosition, RelativePosition>(const RelativePosition& position, AbsoluteSize parentSize) noexcept
	{
		return position;
	}

	export template<VariantMember<SizeVariant> ToType, VariantMember<SizeVariant> FromType>
		ToType ConvertSizeRepresentation(const FromType& size, AbsoluteSize parentSize) = delete;

	template<>
	AbsoluteSize ConvertSizeRepresentation<AbsoluteSize, AbsoluteSize>(const AbsoluteSize& size, AbsoluteSize parentSize) noexcept
	{
		return size;
	}

	template<>
	RelativeSize ConvertSizeRepresentation<RelativeSize, AbsoluteSize>(const AbsoluteSize& size, AbsoluteSize parentSize) noexcept
	{
		return RelativeSize{ xk::Math::HadamardDivision(size.value, parentSize.value) };
	}

	template<>
	AspectRatioRelativeSize ConvertSizeRepresentation<AspectRatioRelativeSize, AbsoluteSize>(const AbsoluteSize& size, AbsoluteSize parentSize) noexcept
	{
		return AspectRatioRelativeSize{ .ratio = size.value.X() / size.value.Y(), .value = size.value.X() / parentSize.value.X() };
	}

	template<>
	BorderConstantRelativeSize ConvertSizeRepresentation<BorderConstantRelativeSize, AbsoluteSize>(const AbsoluteSize& size, AbsoluteSize parentSize) noexcept
	{
		const auto [baseSize, variableSize] = [parentSize]()
			{
				const auto [min, max] = std::minmax(parentSize.value.X(), parentSize.value.Y());
				if(parentSize.value.X() >= parentSize.value.Y())
					return std::pair(xk::Math::Aliases::Vector2{ max - min, 0 }, xk::Math::Aliases::Vector2{ min, min });
				else
					return std::pair(xk::Math::Aliases::Vector2{ 0, max - min }, xk::Math::Aliases::Vector2{ min, min });
			}();

			return BorderConstantRelativeSize{ xk::Math::HadamardDivision(size.value - baseSize, variableSize) };
	}

	template<>
	AbsoluteSize ConvertSizeRepresentation<AbsoluteSize, RelativeSize>(const RelativeSize& size, AbsoluteSize parentSize) noexcept
	{
		return AbsoluteSize{ xk::Math::HadamardProduct(size.value, parentSize.value) };
	}

	template<>
	RelativeSize ConvertSizeRepresentation<RelativeSize, RelativeSize>(const RelativeSize& size, AbsoluteSize parentSize) noexcept
	{
		return size;
	}

	template<>
	AspectRatioRelativeSize ConvertSizeRepresentation<AspectRatioRelativeSize, RelativeSize>(const RelativeSize& size, AbsoluteSize parentSize) noexcept
	{
		return ConvertSizeRepresentation<AspectRatioRelativeSize>(ConvertSizeRepresentation<AbsoluteSize>(size, parentSize), parentSize);
	}

	template<>
	BorderConstantRelativeSize ConvertSizeRepresentation<BorderConstantRelativeSize, RelativeSize>(const RelativeSize& size, AbsoluteSize parentSize) noexcept
	{
		return ConvertSizeRepresentation<BorderConstantRelativeSize>(ConvertSizeRepresentation<AbsoluteSize>(size, parentSize), parentSize);
	}

	template<>
	AbsoluteSize ConvertSizeRepresentation<AbsoluteSize, AspectRatioRelativeSize>(const AspectRatioRelativeSize& size, AbsoluteSize parentSize) noexcept
	{
		const float parentRatio = parentSize.value.X() / parentSize.value.Y();
		const float parentToElementRatio = parentRatio / std::fabs(size.ratio);
		return (size.ratio >= 0) ?
			AbsoluteSize{ { parentSize.value.X() * size.value, parentSize.value.Y() * size.value * parentToElementRatio } } :
			AbsoluteSize{ { parentSize.value.X() * size.value / parentToElementRatio, parentSize.value.Y() * size.value } };
	}

	template<>
	RelativeSize ConvertSizeRepresentation<RelativeSize, AspectRatioRelativeSize>(const AspectRatioRelativeSize& size, AbsoluteSize parentSize) noexcept
	{
		const xk::Math::Aliases::Vector2 absoluteSize = ConvertSizeRepresentation<AbsoluteSize>(size, parentSize).value;
		return RelativeSize{ xk::Math::HadamardDivision(absoluteSize, parentSize.value) };
	}

	template<>
	AspectRatioRelativeSize ConvertSizeRepresentation<AspectRatioRelativeSize, AspectRatioRelativeSize>(const AspectRatioRelativeSize& size, AbsoluteSize parentSize) noexcept
	{
		return size;
	}

	template<>
	BorderConstantRelativeSize ConvertSizeRepresentation<BorderConstantRelativeSize, AspectRatioRelativeSize>(const AspectRatioRelativeSize& size, AbsoluteSize parentSize) noexcept
	{
		return ConvertSizeRepresentation<BorderConstantRelativeSize>(ConvertSizeRepresentation<AbsoluteSize>(size, parentSize), parentSize);
	}

	template<>
	AbsoluteSize ConvertSizeRepresentation<AbsoluteSize, BorderConstantRelativeSize>(const BorderConstantRelativeSize& size, AbsoluteSize parentSize) noexcept
	{
		if(parentSize.value.X() >= parentSize.value.Y())
		{
			const float baseSize = parentSize.value.X() - parentSize.value.Y();
			const float remainingSize = parentSize.value.X() - baseSize;
			return AbsoluteSize{ { baseSize + remainingSize * size.value.X(), parentSize.value.Y() * size.value.Y() } };
		}
		else
		{
			const float baseSize = parentSize.value.Y() - parentSize.value.X();
			const float remainingSize = parentSize.value.Y() - baseSize;
			return AbsoluteSize{ { parentSize.value.X() * size.value.X(), baseSize + remainingSize * size.value.Y() } };
		}
	}

	template<>
	RelativeSize ConvertSizeRepresentation<RelativeSize, BorderConstantRelativeSize>(const BorderConstantRelativeSize& size, AbsoluteSize parentSize) noexcept
	{
		const xk::Math::Aliases::Vector2 staticSize = ConvertSizeRepresentation<AbsoluteSize>(size, parentSize).value;
		return RelativeSize{ xk::Math::HadamardDivision(staticSize, parentSize.value) };
	}

	template<>
	AspectRatioRelativeSize ConvertSizeRepresentation<AspectRatioRelativeSize, BorderConstantRelativeSize>(const BorderConstantRelativeSize& size, AbsoluteSize parentSize) noexcept
	{
		return ConvertSizeRepresentation<AspectRatioRelativeSize>(ConvertSizeRepresentation<AbsoluteSize>(size, parentSize), parentSize);
	}

	template<>
	BorderConstantRelativeSize ConvertSizeRepresentation<BorderConstantRelativeSize, BorderConstantRelativeSize>(const BorderConstantRelativeSize& size, AbsoluteSize parentSize) noexcept
	{
		return size;
	}

	//Calculates an equivalent position that would not affect the visual position of an element between different pivots
	export AbsolutePosition ConvertPivotEquivalentAbsolutePosition(xk::Math::Aliases::Vector2 fromPivot, xk::Math::Aliases::Vector2 toPivot, AbsolutePosition fromPosition, RelativeSize elementSize, AbsoluteSize parentSize) noexcept
	{
		const xk::Math::Aliases::Vector2 pivotDiff = toPivot - fromPivot;
		const xk::Math::Aliases::Vector2 offset = xk::Math::HadamardProduct(pivotDiff, elementSize.value);
		return AbsolutePosition{ fromPosition.value + ConvertSizeRepresentation<AbsoluteSize>(RelativeSize{ offset }, parentSize).value };
	}

	//Calculates an equivalent position that would not affect the visual position of an element between different pivots
	export RelativePosition ConvertPivotEquivalentRelativePosition(xk::Math::Aliases::Vector2 fromPivot, xk::Math::Aliases::Vector2 toPivot, RelativePosition fromPosition, RelativeSize elementSize, AbsoluteSize parentSize) noexcept
	{
		const xk::Math::Aliases::Vector2 pivotDiff = toPivot - fromPivot;
		const xk::Math::Aliases::Vector2 offset = xk::Math::HadamardProduct(pivotDiff, elementSize.value);
		return RelativePosition{ fromPosition.value + offset };
	}

	//Calculates an equivalent position that would not affect the visual position of an element between different pivots
	export PositionVariant ConvertPivotEquivalentPosition(xk::Math::Aliases::Vector2 fromPivot, xk::Math::Aliases::Vector2 toPivot, PositionVariant fromPosition, RelativeSize elementSize, AbsoluteSize parentSize) noexcept
	{
		static_assert(std::same_as<PositionVariant, std::variant<AbsolutePosition, RelativePosition>>, "Position variant changed, function requires changes");
		if(auto* position = std::get_if<AbsolutePosition>(&fromPosition); position)
		{
			return AbsolutePosition{ ConvertPivotEquivalentAbsolutePosition(fromPivot, toPivot, *position, elementSize, parentSize) };
		}
		else
		{
			return RelativePosition{ ConvertPivotEquivalentRelativePosition(fromPivot, toPivot, std::get<RelativePosition>(fromPosition), elementSize, parentSize) };
		}
	}

	export struct Rect
	{
		AbsolutePosition bottomLeft;
		AbsolutePosition topRight;

		AbsolutePosition Center() const noexcept
		{
			return AbsolutePosition{ xk::Math::HadamardDivision(bottomLeft.value + topRight.value, xk::Math::Aliases::Vector2{ 2, 2 }) };
		}

		float Width() const noexcept
		{
			return topRight.value.X() - bottomLeft.value.X();
		}

		float Height() const noexcept
		{
			return topRight.value.Y() - bottomLeft.value.Y();
		}

		bool Overlaps(const AbsolutePosition& other) const noexcept
		{
			return xk::Math::InRange(xk::Math::Inclusive{ bottomLeft.value.X() }, other.value.X(), xk::Math::Inclusive{ topRight.value.X() })
				&& xk::Math::InRange(xk::Math::Inclusive{ bottomLeft.value.Y() }, other.value.Y(), xk::Math::Inclusive{ topRight.value.Y() });
		}

		bool Overlaps(const Rect& other) const noexcept
		{
			return Overlaps(other.bottomLeft) || Overlaps(other.topRight);
		}
	};

	constexpr int defaultSuccessCode = 0;

	export enum class MouseEventType
	{
		Overlap,
		Unoverlap,
		Hover,
	};

	export enum class MouseClickType
	{
		Pressed,
		Released,
		Clicked,
		Held,
	};

	export struct MouseEvent
	{
		MouseEventType type;
		std::optional<MouseClickType> action;
		//AbsolutePosition frameCursorPosition;

		static constexpr int handledCode = 0;
		static constexpr int unhandledCode = 1;
	};

	export struct DrawEvent
	{
		DeluEngine::Renderer* renderer;
	};

	export using Event = std::variant<MouseEvent, DrawEvent>;

	export class UIElement;

	export enum class PivotChangePolicy
	{
		//The underlying position will not change, will result in appearing in a different position however
		NoPositionChange,

		//The underlying position will change which will result in appearing in the same position
		NoVisualChange
	};

	export enum class UIReparentLogic
	{
		KeepAbsoluteTransform,
		KeepRelativeTransform
	};

	export struct GUIEngine;
	struct UIElementDeleter;

	export class UIElement
	{
		friend GUIEngine;
		friend UIElementDeleter;

	private:
		GUIEngine* m_engine = nullptr;
		UIElement* m_parent = nullptr;
		std::vector<UIElement*> m_children;
		PositionVariant m_position;
		SizeVariant m_size;
		xk::Math::Aliases::Vector2 m_pivot;

	public:
		std::string debugName;
		bool debugEnableRaytrace = false;

	public:
		UIElement(GUIEngine& engine) :
			m_engine{ &engine }
		{

		}

		UIElement(GUIEngine& engine, PositionVariant position, SizeVariant size, xk::Math::Aliases::Vector2 pivot) :
			m_engine{ &engine },
			m_position{ position },
			m_size{ size },
			m_pivot{ pivot }
		{
		}

		UIElement(const UIElement&) = delete;
		UIElement(UIElement&&) noexcept = default;

		virtual ~UIElement();

		UIElement& operator=(const UIElement&) = delete;
		UIElement& operator=(UIElement&&) noexcept = default;

	public:
		virtual int HandleEvent(const Event& event);

		const std::vector<UIElement*>& GetChildren() const noexcept { return m_children; }

		Rect GetRect() const noexcept
		{
			AbsolutePosition bottomLeft = GetPivotedFramePositionAs<AbsolutePosition>();
			return { bottomLeft, AbsolutePosition{ bottomLeft.value + GetFrameSizeAs<AbsoluteSize>().value} };
		}

		xk::Math::Aliases::Vector2 GetPivot() const noexcept
		{
			return m_pivot;
		}

		template<VariantMember<PositionVariant> Ty>
		Ty GetPivotOffset() const noexcept
		{
			const xk::Math::Aliases::Vector2 size = GetFrameSizeAs<AbsoluteSize>().value;
			const xk::Math::Aliases::Vector2 pivotOffset = { GetPivot().X() * -size.X(), GetPivot().Y() * -size.Y() };
			return ConvertPositionRepresentation<Ty>(AbsolutePosition{ pivotOffset }, GetRendererSize());
		}

		template<VariantMember<PositionVariant> Ty>
		Ty GetPivotedFramePositionAs() const noexcept
		{
			return std::visit([this](const auto& val) -> Ty
				{
					const xk::Math::Aliases::Vector2 size = GetFrameSizeAs<AbsoluteSize>().value;
					const xk::Math::Aliases::Vector2 pivotOffset = { GetPivot().X() * -size.X(), GetPivot().Y() * -size.Y() };
					return ConvertPositionRepresentation<Ty>(AbsolutePosition{ GetLocalPositionAs<AbsolutePosition>().value + GetParentPivotedFrameAbsolutePosition().value + pivotOffset }, GetRendererSize());
				}, m_position);
		}

		template<VariantMember<PositionVariant> Ty>
		Ty GetFramePositionAs() const noexcept
		{
			return std::visit([this](const auto& val) -> Ty
				{
					return ConvertPositionRepresentation<Ty>(AbsolutePosition{ GetLocalPositionAs<AbsolutePosition>().value + GetParentPivotedFrameAbsolutePosition().value }, GetRendererSize());
				}, m_position);
		}

		template<VariantMember<SizeVariant> Ty>
		Ty GetFrameSizeAs() const noexcept
		{
			return ConvertSizeRepresentation<Ty>(GetLocalSizeAs<AbsoluteSize>(), GetRendererSize());
		}

		template<VariantMember<PositionVariant> Ty>
		Ty GetLocalPositionAs() const noexcept
		{
			return std::visit([this](const auto& val) -> Ty
				{
					return ConvertPositionRepresentation<Ty>(val, GetParentAbsoluteSize());
				}, m_position);
		}

		template<VariantMember<SizeVariant> Ty>
		Ty GetLocalSizeAs() const noexcept
		{
			return std::visit([this](const auto& val) -> Ty
				{
					return ConvertSizeRepresentation<Ty>(val, GetParentAbsoluteSize());
				}, m_size);
		}

		UIElement* GetParent() const noexcept
		{
			return m_parent;
		}

		template<VariantMember<PositionVariant> Ty>
		void ConvertUnderlyingPositionRepresentation() noexcept
		{
			m_position = GetLocalPositionAs<Ty>();
		}

		template<VariantMember<SizeVariant> Ty>
		void ConvertUnderlyingSizeRepresentation() noexcept
		{
			m_size = GetLocalSizeAs<Ty>();
		}

		void SetLocalPosition(PositionVariant newPos) noexcept
		{
			std::visit([this](auto& innerVal, auto& newVal)
				{
					using Inner_Ty = std::remove_cvref_t<decltype(innerVal)>;
					innerVal = ConvertPositionRepresentation<Inner_Ty>(newVal, GetParentAbsoluteSize());
				}, m_position, newPos);
		}

		void SetLocalSize(SizeVariant newSize) noexcept
		{
			std::visit([this](auto& innerVal, auto& newVal)
				{
					using Inner_Ty = std::remove_cvref_t<decltype(innerVal)>;
					innerVal = ConvertSizeRepresentation<Inner_Ty>(newVal, GetParentAbsoluteSize());
				}, m_size, newSize);
		}

		void SetFramePosition(PositionVariant newPos) noexcept
		{
			std::visit([this](auto& innerVal, auto& newVal)
				{
					using Inner_Ty = std::remove_cvref_t<decltype(innerVal)>;
					AbsolutePosition parentPosition = (m_parent) ? m_parent->GetPivotedFramePositionAs<AbsolutePosition>() : AbsolutePosition{};
					AbsolutePosition requestedPosition = ConvertPositionRepresentation<AbsolutePosition>(newVal, GetRendererSize());
					innerVal = ConvertPositionRepresentation<Inner_Ty>(AbsolutePosition{ requestedPosition.value - parentPosition.value }, GetParentAbsoluteSize());
				}, m_position, newPos);
		}

		void SetFrameSize(SizeVariant newSize) noexcept
		{
			std::visit([this](auto& innerVal, auto& newVal)
				{
					using Inner_Ty = std::remove_cvref_t<decltype(innerVal)>;
					AbsoluteSize requestedAbsoluteSize = ConvertSizeRepresentation<AbsoluteSize>(newVal, GetRendererSize());
					innerVal = ConvertSizeRepresentation<Inner_Ty>(requestedAbsoluteSize, GetParentAbsoluteSize());
				}, m_size, newSize);
		}

		void SetPositionRepresentation(PositionVariant val) noexcept
		{
			m_position = val;
		}

		void SetSizeRepresentation(SizeVariant val) noexcept
		{
			m_size = val;
		}

		void SetParent(UIElement* newParent, UIReparentLogic logic = UIReparentLogic::KeepRelativeTransform);

		void SetPivot(xk::Math::Aliases::Vector2 pivot, PivotChangePolicy policy = PivotChangePolicy::NoPositionChange) noexcept
		{
			switch(policy)
			{
			case PivotChangePolicy::NoPositionChange:
			{
				m_pivot = pivot;
				break;
			}
			case PivotChangePolicy::NoVisualChange:
			{
				m_position = ConvertPivotEquivalentPosition(m_pivot, pivot, m_position, GetLocalSizeAs<RelativeSize>(), GetParentAbsoluteSize());
				m_pivot = pivot;
				break;
			}
			default:
				break;// Should be unreachable
			}
		}

		AbsoluteSize GetParentAbsoluteSize() const noexcept
		{
			return m_parent ? m_parent->GetFrameSizeAs<AbsoluteSize>() : GetRendererSize();
		}

		AbsolutePosition GetParentPivotedFrameAbsolutePosition() const noexcept
		{
			return m_parent ? m_parent->GetPivotedFramePositionAs<AbsolutePosition>() : AbsolutePosition{};
		}

		AbsoluteSize GetRendererSize() const noexcept;
	};

	export class Image : public UIElement
	{
	public:
		SDL2pp::shared_ptr<SDL2pp::Texture> texture;
		bool render = true;

	public:
		Image(GUIEngine& engine) :
			UIElement{ engine }
		{

		}

		Image(GUIEngine& engine, PositionVariant position, SizeVariant size, xk::Math::Aliases::Vector2 pivot, SDL2pp::shared_ptr<SDL2pp::Texture> texture = nullptr) :
			UIElement{ engine, position, size, pivot },
			texture{ texture }
		{
		}

		virtual int HandleEvent(const Event& event);
	};

	export class Button : public UIElement
	{
	public:
		SDL2pp::shared_ptr<SDL2pp::Texture> texture;
		std::function<void()> onClicked;
		bool render = true;

	public:
		Button(GUIEngine& engine) :
			UIElement{ engine }
		{
			debugEnableRaytrace = true;
		}

		Button(GUIEngine& engine, PositionVariant position, SizeVariant size, xk::Math::Aliases::Vector2 pivot, SDL2pp::shared_ptr<SDL2pp::Texture> texture = nullptr) :
			UIElement{ engine, position, size, pivot },
			texture{ texture }
		{
			debugEnableRaytrace = true;
		}

		virtual int HandleEvent(const Event& event);
	};

	export class Text : public UIElement
	{
	private:
		TTF_Font* m_font;
		std::string m_text;
		AbsoluteSize m_textBounds;
		SDL2pp::unique_ptr<SDL2pp::Texture> m_texture;
		bool m_dirty = true;

	public:
		bool render = true;

	public:
		Text(GUIEngine& engine) :
			UIElement{ engine }
		{
		}

		Text(GUIEngine& engine, PositionVariant position, SizeVariant size, xk::Math::Aliases::Vector2 pivot) :
			UIElement{ engine, position, size, pivot }
		{
		}

		void SetText(std::string_view text);
		void SetFont(TTF_Font* font)
		{
			m_font = font;
			m_dirty = true;
		}

		virtual int HandleEvent(const Event& event);
	};

	DeluEngine::GUI::UIElement* GetHoveredElement(DeluEngine::GUI::UIElement& element, DeluEngine::GUI::AbsolutePosition mousePos)
	{
		std::vector<DeluEngine::GUI::UIElement*> children = element.GetChildren();
		for(DeluEngine::GUI::UIElement* child : children)
		{
			if(auto hoveredElement = GetHoveredElement(*child, mousePos); hoveredElement)
				return hoveredElement;
		}

		if(!element.debugEnableRaytrace)
			return nullptr;

		if(element.GetRect().Overlaps(mousePos))
		{
			return &element;
		}
		return nullptr;
	}

	struct UIElementDeleter
	{
		void operator()(UIElement* element);
	};

	export template<std::derived_from<UIElement> Ty>
	using UniqueHandle = std::unique_ptr<Ty, UIElementDeleter>;

	export struct GUIEngine
	{
		bool leftClickPressed = false;
		bool previousLeftClickPressed = false;

		SDL2pp::unique_ptr<SDL2pp::Texture> internalTexture;
		AbsolutePosition mousePosition;
		std::vector<UIElement*> rootElements;
		std::vector<UIElement*> pendingDeletedElements;
		UIElement* hoveredElement = nullptr;
		UIElement* previousHoveredElement = nullptr;
		UIElement* initialLeftClickedElement = nullptr;

		template<std::derived_from<UIElement> Ty, class... ExtraConstructorParams>
		UniqueHandle<Ty> NewElement(PositionVariant position, SizeVariant size, xk::Math::Aliases::Vector2 pivot, UIElement* parent = nullptr, ExtraConstructorParams&&... params)
		{
			UniqueHandle<Ty> element{ new Ty(*this, position, size, pivot, std::forward<ExtraConstructorParams>(params)...), {} };
			if(parent)
			{
				element->m_parent = parent;
				element->m_parent->m_children.push_back(element.get());
			}
			else
			{
				rootElements.push_back(element.get());
			}
			return element;
		}

		void UpdateHoveredElement()
		{
			if(std::any_of(pendingDeletedElements.begin(), pendingDeletedElements.end(), [this](UIElement* other) { return other == hoveredElement; }))
				hoveredElement = nullptr;

			for(UIElement* element : pendingDeletedElements)
			{
				delete element;
			}
			pendingDeletedElements.clear();

			previousHoveredElement = std::exchange(hoveredElement, nullptr);
			for(auto it = rootElements.rbegin(); it != rootElements.rend() && !hoveredElement; it++)
				hoveredElement = GetHoveredElement(*(*it), mousePosition);
		}

		void DispatchHoveredEvent()
		{
			if(hoveredElement != previousHoveredElement)
			{
				if(previousHoveredElement)
				{
					previousHoveredElement->HandleEvent(DeluEngine::GUI::MouseEvent{ DeluEngine::GUI::MouseEventType::Unoverlap, std::nullopt });
				}
				if(hoveredElement)
				{
					hoveredElement->HandleEvent(DeluEngine::GUI::MouseEvent{ DeluEngine::GUI::MouseEventType::Overlap, std::nullopt });
				}
			}

			if(hoveredElement)
			{
				std::optional action = [this]() -> std::optional<DeluEngine::GUI::MouseClickType>
					{
						if(leftClickPressed && previousLeftClickPressed)
							return DeluEngine::GUI::MouseClickType::Held;
						else
							return std::nullopt;
					}();
					hoveredElement->HandleEvent(DeluEngine::GUI::MouseEvent{ DeluEngine::GUI::MouseEventType::Hover, action });
			}
			previousLeftClickPressed = leftClickPressed;
		}

		AbsoluteSize GetFrameSize() const noexcept
		{
			return { internalTexture->GetSize() };
		}
	};

	UIElement::~UIElement()
	{
		while(!m_children.empty())
		{
			m_children.back()->SetParent(m_parent, UIReparentLogic::KeepAbsoluteTransform);
		}
		SetParent(nullptr);
		std::erase(m_engine->rootElements, this);
	}

	AbsoluteSize DeluEngine::GUI::UIElement::GetRendererSize() const noexcept
	{
		return m_engine->GetFrameSize();
	}
	
	void UIElementDeleter::operator()(UIElement* element)
	{
		element->m_engine->pendingDeletedElements.push_back(element);
	}

	export void ProcessEvent(GUIEngine& engine, const SDL2pp::Event& event, xk::Math::Aliases::Vector2 windowSize);
}