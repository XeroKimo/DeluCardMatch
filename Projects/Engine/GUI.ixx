module;

#include <functional>
#include <variant>
#include <algorithm>
#include <utility>
#include <vector>
#include <optional>
#include <memory>
#include <string>

export module DeluEngine:GUI;
import xk.Math.Matrix;
import xk.Math.Algorithms;
import SDL2pp;

namespace DeluEngine::GUI
{
	using namespace xk::Math::Aliases;

	export struct AbsolutePosition
	{
		Vector2 value;
	};

	export struct RelativePosition
	{
		Vector2 value;
	};

	//Keeps the size of the element between all screen sizes
	export struct AbsoluteSize
	{
		Vector2 value;
	};

	//Keeps the percentage of the screen size constant between screen sizes
	export struct RelativeSize
	{
		Vector2 value;
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
		Vector2 value;
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
				return std::pair(Vector2{ min, min }, Vector2{ max, max } - Vector2{ min, min });
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
		const Vector2 absoluteSize = ConvertSizeRepresentation<AbsoluteSize>(size, parentSize).value;
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
		const Vector2 staticSize = ConvertSizeRepresentation<AbsoluteSize>(size, parentSize).value;
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
	export AbsolutePosition ConvertPivotEquivalentAbsolutePosition(Vector2 fromPivot, Vector2 toPivot, AbsolutePosition fromPosition, RelativeSize elementSize, AbsoluteSize parentSize) noexcept
	{
		const Vector2 pivotDiff = toPivot - fromPivot;
		const Vector2 offset = xk::Math::HadamardProduct(pivotDiff, elementSize.value);
		return AbsolutePosition{ fromPosition.value + ConvertSizeRepresentation<AbsoluteSize>(RelativeSize{ offset }, parentSize).value };
	}

	//Calculates an equivalent position that would not affect the visual position of an element between different pivots
	export RelativePosition ConvertPivotEquivalentRelativePosition(Vector2 fromPivot, Vector2 toPivot, RelativePosition fromPosition, RelativeSize elementSize, AbsoluteSize parentSize) noexcept
	{
		const Vector2 pivotDiff = toPivot - fromPivot;
		const Vector2 offset = xk::Math::HadamardProduct(pivotDiff, elementSize.value);
		return RelativePosition{ fromPosition.value + offset };
	}

	//Calculates an equivalent position that would not affect the visual position of an element between different pivots
	export PositionVariant ConvertPivotEquivalentPosition(Vector2 fromPivot, Vector2 toPivot, PositionVariant fromPosition, RelativeSize elementSize, AbsoluteSize parentSize) noexcept
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
			return AbsolutePosition{ xk::Math::HadamardDivision(bottomLeft.value + topRight.value, Vector2{ 2, 2 }) };
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

	export using Event = std::variant<MouseEvent>;

	export class UIElement;

	export class UIFrame
	{
		friend UIElement;

	private:
		std::vector<UIElement*> m_rootElements;

	public:
		SDL2pp::unique_ptr<SDL2pp::Texture> internalTexture;
		Vector2 GetSize() const noexcept { return internalTexture->GetSize(); }

		std::unique_ptr<UIElement> NewElement(PositionVariant position, SizeVariant size, Vector2 pivot, UIElement* parent = nullptr);

		const std::vector<UIElement*>& GetRootElements() const noexcept { return m_rootElements; }
	};

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

	export class UIElement
	{

	private:
		UIFrame* m_ownerFrame;
		UIElement* m_parent = nullptr;
		std::vector<UIElement*> m_children;
		PositionVariant m_position;
		SizeVariant m_size;
		Vector2 m_pivot;

	public:
		std::string debugName;
		bool debugEnableRaytrace = true;
		SDL2pp::shared_ptr<SDL2pp::Texture> texture;

	public:
		UIElement(UIFrame& ownerFrame) :
			m_ownerFrame{ &ownerFrame }
		{

		}

		UIElement(UIFrame& ownerFrame, PositionVariant position, SizeVariant size, Vector2 pivot, UIElement* parent = nullptr) :
			m_ownerFrame{ &ownerFrame },
			m_parent{ parent },
			m_position{ position },
			m_size{ size },
			m_pivot{ pivot }
		{
			SetParent(parent);
		}

		UIElement(UIElement&&) noexcept = default;

		~UIElement()
		{
			while(!m_children.empty())
			{
				m_children.back()->SetParent(m_parent, UIReparentLogic::KeepAbsoluteTransform);
			}
			SetParent(nullptr);
		}

		UIElement& operator=(UIElement&&) noexcept = default;

	public:
		virtual int HandleEvent(const Event& event);

		const std::vector<UIElement*>& GetChildren() const noexcept { return m_children; }

		Rect GetRect() const noexcept
		{
			AbsolutePosition bottomLeft = GetPivotedFramePositionAs<AbsolutePosition>();
			return { bottomLeft, AbsolutePosition{ bottomLeft.value + GetFrameSizeAs<AbsoluteSize>().value} };
		}

		Vector2 GetPivot() const noexcept
		{
			return m_pivot;
		}

		template<VariantMember<PositionVariant> Ty>
		Ty GetPivotOffset() const noexcept
		{
			const Vector2 size = GetFrameSizeAs<AbsoluteSize>().value;
			const Vector2 pivotOffset = { GetPivot().X() * -size.X(), GetPivot().Y() * -size.Y() };
			return ConvertPositionRepresentation<Ty>(AbsolutePosition{ pivotOffset }, AbsoluteSize{ m_ownerFrame->GetSize() });
		}

		template<VariantMember<PositionVariant> Ty>
		Ty GetPivotedFramePositionAs() const noexcept
		{
			return std::visit([this](const auto& val) -> Ty
				{
					const Vector2 size = GetFrameSizeAs<AbsoluteSize>().value;
					const Vector2 pivotOffset = { GetPivot().X() * -size.X(), GetPivot().Y() * -size.Y() };
					return ConvertPositionRepresentation<Ty>(AbsolutePosition{ GetLocalPositionAs<AbsolutePosition>().value + GetParentPivotedFrameAbsolutePosition().value + pivotOffset }, AbsoluteSize{ m_ownerFrame->GetSize() });
				}, m_position);
		}

		template<VariantMember<PositionVariant> Ty>
		Ty GetFramePositionAs() const noexcept
		{
			return std::visit([this](const auto& val) -> Ty
				{
					return ConvertPositionRepresentation<Ty>(AbsolutePosition{ GetLocalPositionAs<AbsolutePosition>().value + GetParentPivotedFrameAbsolutePosition().value }, AbsoluteSize{ m_ownerFrame->GetSize() });
				}, m_position);
		}

		template<VariantMember<SizeVariant> Ty>
		Ty GetFrameSizeAs() const noexcept
		{
			return ConvertSizeRepresentation<Ty>(GetLocalSizeAs<AbsoluteSize>(), AbsoluteSize{ m_ownerFrame->GetSize() });
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

		template<VariantMember<PositionVariant> Ty>
		void SetLocalPosition(Ty val) noexcept
		{
			std::visit([this, &val](auto& innerVal)
				{
					using Inner_Ty = std::remove_cvref_t<decltype(innerVal)>;
					innerVal = ConvertPositionRepresentation<Inner_Ty>(val, GetParentAbsoluteSize());
				}, m_position);
		}

		template<VariantMember<SizeVariant> Ty>
		void SetLocalSize(Ty val) noexcept
		{
			std::visit([this, &val](auto& innerVal)
				{
					using Inner_Ty = std::remove_cvref_t<decltype(innerVal)>;
					innerVal = ConvertSizeRepresentation<Inner_Ty>(val, GetParentAbsoluteSize());
				}, m_size);
		}

		template<VariantMember<PositionVariant> Ty>
		void SetFramePosition(Ty val) noexcept
		{
			std::visit([this, &val](auto& innerVal)
				{
					using Inner_Ty = std::remove_cvref_t<decltype(innerVal)>;
					AbsolutePosition parentPosition = (m_parent) ? m_parent->GetPivotedFramePositionAs<AbsolutePosition>() : AbsolutePosition{};
					AbsolutePosition requestedPosition = ConvertPositionRepresentation<AbsolutePosition>(val, AbsoluteSize{ m_ownerFrame->GetSize() });
					innerVal = ConvertPositionRepresentation<Inner_Ty>(AbsolutePosition{ requestedPosition.value - parentPosition.value }, GetParentAbsoluteSize());
				}, m_position);
		}

		template<VariantMember<SizeVariant> Ty>
		void SetFrameSize(Ty val) noexcept
		{
			std::visit([this, &val](auto& innerVal)
				{
					using Inner_Ty = std::remove_cvref_t<decltype(innerVal)>;
					AbsoluteSize requestedAbsoluteSize = ConvertSizeRepresentation<AbsoluteSize>(val, AbsoluteSize{ m_ownerFrame->GetSize() });
					innerVal = ConvertSizeRepresentation<Inner_Ty>(requestedAbsoluteSize, GetParentAbsoluteSize());
				}, m_size);
		}

		template<VariantMember<PositionVariant> Ty>
		void SetPositionRepresentation(Ty val) noexcept
		{
			m_position = val;
		}

		template<VariantMember<SizeVariant> Ty>
		void SetSizeRepresentation(Ty val) noexcept
		{
			m_size = val;
		}

		void SetParent(UIElement* newParent, UIReparentLogic logic = UIReparentLogic::KeepRelativeTransform);

		void SetPivot(Vector2 pivot, PivotChangePolicy policy = PivotChangePolicy::NoPositionChange) noexcept
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
			return m_parent ? m_parent->GetFrameSizeAs<AbsoluteSize>() : AbsoluteSize{ m_ownerFrame->GetSize() };
		}

		AbsolutePosition GetParentPivotedFrameAbsolutePosition() const noexcept
		{
			return m_parent ? m_parent->GetPivotedFramePositionAs<AbsolutePosition>() : AbsolutePosition{};
		}

		const UIFrame& GetFrame() const { return *m_ownerFrame; }
	};

	std::unique_ptr<UIElement> UIFrame::NewElement(PositionVariant position, SizeVariant size, Vector2 pivot, UIElement* parent)
	{
		return std::make_unique<UIElement>(*this, position, size, pivot, parent);
	}
}