module;

#include <functional>
#include <variant>
#include <algorithm>
#include <utility>
#include <vector>

export module DeluEngine:GUI;
import xk.Math.Matrix;
import xk.Math.Algorithms;
import SDL2pp;

namespace DeluEngine::GUI
{
	using namespace xk::Math::Aliases;
	//Keeps the size of the element between all screen sizes
	export struct StaticSize
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

	export struct StaticPosition
	{
		Vector2 value;
	};

	export struct RelativePosition
	{
		Vector2 value;
	};

	export using PositionVariant = std::variant<StaticPosition, RelativePosition>;
	export using SizeVariant = std::variant<StaticSize, RelativeSize, AspectRatioRelativeSize, BorderConstantRelativeSize>;

	template<class T, class U> struct IsVariantMember;

	template<class T, class... Ts>
	struct IsVariantMember<T, std::variant<Ts...>> : std::bool_constant<(std::same_as<T, Ts> || ...)>
	{
	};

	template<class Ty, class VariantTy>
	concept VariantMember = (IsVariantMember<Ty, VariantTy>::value);

	export template<VariantMember<PositionVariant> ToType, VariantMember<PositionVariant> FromType>
	ToType ConvertPositionRepresentation(const FromType& position, StaticSize parentSize) = delete;

	template<>
	StaticPosition ConvertPositionRepresentation<StaticPosition, StaticPosition>(const StaticPosition& position, StaticSize parentSize) noexcept
	{
		return position;
	}

	template<>
	RelativePosition ConvertPositionRepresentation<RelativePosition, StaticPosition>(const StaticPosition& position, StaticSize parentSize) noexcept
	{
		return RelativePosition{ xk::Math::HadamardDivision(position.value, parentSize.value) };
	}

	template<>
	StaticPosition ConvertPositionRepresentation<StaticPosition, RelativePosition>(const RelativePosition& position, StaticSize parentSize) noexcept
	{
		return StaticPosition{ xk::Math::HadamardProduct(position.value, parentSize.value) };
	}

	template<>
	RelativePosition ConvertPositionRepresentation<RelativePosition, RelativePosition>(const RelativePosition& position, StaticSize parentSize) noexcept
	{
		return position;
	}

	export template<VariantMember<SizeVariant> ToType, VariantMember<SizeVariant> FromType>
	ToType ConvertSizeRepresentation(const FromType& size, StaticSize parentSize) = delete;

	template<>
	StaticSize ConvertSizeRepresentation<StaticSize, StaticSize>(const StaticSize& size, StaticSize parentSize) noexcept
	{
		return size;
	}

	template<>
	RelativeSize ConvertSizeRepresentation<RelativeSize, StaticSize>(const StaticSize& size, StaticSize parentSize) noexcept
	{
		return RelativeSize{ xk::Math::HadamardDivision(size.value, parentSize.value) };
	}

	template<>
	AspectRatioRelativeSize ConvertSizeRepresentation<AspectRatioRelativeSize, StaticSize>(const StaticSize& size, StaticSize parentSize) noexcept
	{
		return AspectRatioRelativeSize{ .ratio = size.value.X() / size.value.Y(), .value = size.value.X() / parentSize.value.X() };
	}

	template<>
	BorderConstantRelativeSize ConvertSizeRepresentation<BorderConstantRelativeSize, StaticSize>(const StaticSize& size, StaticSize parentSize) noexcept
	{
		const auto [baseSize, variableSize] = [parentSize]()
		{
			const auto [min, max] = std::minmax(parentSize.value.X(), parentSize.value.Y());
			return std::pair(Vector2{ min, min }, Vector2{ max, max } - Vector2{ min, min });
		}();

		return BorderConstantRelativeSize{ xk::Math::HadamardDivision(size.value - baseSize, variableSize) };
	}

	template<>
	StaticSize ConvertSizeRepresentation<StaticSize, RelativeSize>(const RelativeSize& size, StaticSize parentSize) noexcept
	{
		return StaticSize{ xk::Math::HadamardProduct(size.value, parentSize.value) };
	}

	template<>
	RelativeSize ConvertSizeRepresentation<RelativeSize, RelativeSize>(const RelativeSize& size, StaticSize parentSize) noexcept
	{
		return size;
	}

	template<>
	AspectRatioRelativeSize ConvertSizeRepresentation<AspectRatioRelativeSize, RelativeSize>(const RelativeSize& size, StaticSize parentSize) noexcept
	{
		return ConvertSizeRepresentation<AspectRatioRelativeSize>(ConvertSizeRepresentation<StaticSize>(size, parentSize), parentSize);
	}

	template<>
	BorderConstantRelativeSize ConvertSizeRepresentation<BorderConstantRelativeSize, RelativeSize>(const RelativeSize& size, StaticSize parentSize) noexcept
	{
		return ConvertSizeRepresentation<BorderConstantRelativeSize>(ConvertSizeRepresentation<StaticSize>(size, parentSize), parentSize);
	}

	template<>
	StaticSize ConvertSizeRepresentation<StaticSize, AspectRatioRelativeSize>(const AspectRatioRelativeSize& size, StaticSize parentSize) noexcept
	{
		const float parentRatio = parentSize.value.X() / parentSize.value.Y();
		const float parentToElementRatio = parentRatio / std::fabs(size.ratio);
		return (size.ratio >= 0) ?
			StaticSize{ { parentSize.value.X() * size.value, parentSize.value.Y() * size.value * parentToElementRatio } } :
			StaticSize{ { parentSize.value.X() * size.value / parentToElementRatio, parentSize.value.Y() * size.value } };
	}

	template<>
	RelativeSize ConvertSizeRepresentation<RelativeSize, AspectRatioRelativeSize>(const AspectRatioRelativeSize& size, StaticSize parentSize) noexcept
	{
		const Vector2 staticSize = ConvertSizeRepresentation<StaticSize>(size, parentSize).value;
		return RelativeSize{ xk::Math::HadamardDivision(staticSize, parentSize.value) };
	}

	template<>
	AspectRatioRelativeSize ConvertSizeRepresentation<AspectRatioRelativeSize, AspectRatioRelativeSize>(const AspectRatioRelativeSize& size, StaticSize parentSize) noexcept
	{
		return size;
	}

	template<>
	BorderConstantRelativeSize ConvertSizeRepresentation<BorderConstantRelativeSize, AspectRatioRelativeSize>(const AspectRatioRelativeSize& size, StaticSize parentSize) noexcept
	{
		return ConvertSizeRepresentation<BorderConstantRelativeSize>(ConvertSizeRepresentation<StaticSize>(size, parentSize), parentSize);
	}

	template<>
	StaticSize ConvertSizeRepresentation<StaticSize, BorderConstantRelativeSize>(const BorderConstantRelativeSize& size, StaticSize parentSize) noexcept
	{
		if(parentSize.value.X() >= parentSize.value.Y())
		{
			const float baseSize = parentSize.value.X() - parentSize.value.Y();
			const float remainingSize = parentSize.value.X() - baseSize;
			return StaticSize{ { baseSize + remainingSize * size.value.X(), parentSize.value.Y() * size.value.Y() } };
		}
		else
		{
			const float baseSize = parentSize.value.Y() - parentSize.value.X();
			const float remainingSize = parentSize.value.Y() - baseSize;
			return StaticSize{ { parentSize.value.X() * size.value.X(), baseSize + remainingSize * size.value.Y() } };
		}
	}

	template<>
	RelativeSize ConvertSizeRepresentation<RelativeSize, BorderConstantRelativeSize>(const BorderConstantRelativeSize& size, StaticSize parentSize) noexcept
	{
		const Vector2 staticSize = ConvertSizeRepresentation<StaticSize>(size, parentSize).value;
		return RelativeSize{ xk::Math::HadamardDivision(staticSize, parentSize.value) };
	}

	template<>
	AspectRatioRelativeSize ConvertSizeRepresentation<AspectRatioRelativeSize, BorderConstantRelativeSize>(const BorderConstantRelativeSize& size, StaticSize parentSize) noexcept
	{
		return ConvertSizeRepresentation<AspectRatioRelativeSize>(ConvertSizeRepresentation<StaticSize>(size, parentSize), parentSize);
	}

	template<>
	BorderConstantRelativeSize ConvertSizeRepresentation<BorderConstantRelativeSize, BorderConstantRelativeSize>(const BorderConstantRelativeSize& size, StaticSize parentSize) noexcept
	{
		return size;
	}

	//Calculates an equivalent position that would not affect the visual position of an element between different pivots
	export StaticPosition ConvertPivotEquivalentStaticPosition(Vector2 fromPivot, Vector2 toPivot, StaticPosition fromPosition, RelativeSize elementSize, StaticSize parentSize) noexcept
	{
		const Vector2 pivotDiff = toPivot - fromPivot;
		const Vector2 offset = xk::Math::HadamardProduct(pivotDiff, elementSize.value);
		return StaticPosition{ fromPosition.value + ConvertSizeRepresentation<StaticSize>(RelativeSize{ offset }, parentSize).value };
	}

	//Calculates an equivalent position that would not affect the visual position of an element between different pivots
	export RelativePosition ConvertPivotEquivalentRelativePosition(Vector2 fromPivot, Vector2 toPivot, RelativePosition fromPosition, RelativeSize elementSize, StaticSize parentSize) noexcept
	{
		const Vector2 pivotDiff = toPivot - fromPivot;
		const Vector2 offset = xk::Math::HadamardProduct(pivotDiff, elementSize.value);
		return RelativePosition{ fromPosition.value + offset };
	}

	//Calculates an equivalent position that would not affect the visual position of an element between different pivots
	export PositionVariant ConvertPivotEquivalentPosition(Vector2 fromPivot, Vector2 toPivot, PositionVariant fromPosition, RelativeSize elementSize, StaticSize parentSize) noexcept
	{
		static_assert(std::same_as<PositionVariant, std::variant<StaticPosition, RelativePosition>>, "Position variant changed, function requires changes");
		if(auto* position = std::get_if<StaticPosition>(&fromPosition); position)
		{
			return StaticPosition{ ConvertPivotEquivalentStaticPosition(fromPivot, toPivot, *position, elementSize, parentSize) };
		}
		else
		{
			return RelativePosition{ ConvertPivotEquivalentRelativePosition(fromPivot, toPivot, std::get<RelativePosition>(fromPosition), elementSize, parentSize) };
		}
	}

	export struct Rect
	{
		StaticPosition bottomLeft;
		StaticPosition topRight;

		float Width() const noexcept
		{
			return topRight.value.X() - bottomLeft.value.X();
		}

		float Height() const noexcept
		{
			return topRight.value.Y() - bottomLeft.value.Y();
		}

		bool Overlaps(const StaticPosition& other) const noexcept
		{
			return xk::Math::InRange(xk::Math::Inclusive{ bottomLeft.value.X() }, other.value.X(), xk::Math::Inclusive{ topRight.value.X() })
				&& xk::Math::InRange(xk::Math::Inclusive{ bottomLeft.value.Y() }, other.value.Y(), xk::Math::Inclusive{ topRight.value.Y() });
		}

		bool Overlaps(const Rect& other) const noexcept
		{
			return Overlaps(other.bottomLeft) || Overlaps(other.topRight);
		}
	};

	export class UIFrame
	{
	public:
		SDL2pp::unique_ptr<SDL2pp::Texture> internalTexture;
		Vector2 GetSize() const noexcept { return internalTexture->GetSize(); }
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
		KeepStaticTransform,
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
		SDL2pp::shared_ptr<SDL2pp::Texture> texture;

	public:
		UIElement(UIFrame& ownerFrame) :
			m_ownerFrame{ &ownerFrame }
		{

		}

	public:
		Vector2 GetPivot() const noexcept
		{
			return m_pivot;
		}

		template<VariantMember<PositionVariant> Ty>
		Ty GetPivotOffset() const noexcept
		{
			const Vector2 size = GetFrameSizeAs<StaticSize>().value;
			const Vector2 pivotOffset = { GetPivot().X() * -size.X(), GetPivot().Y() * -size.Y() };
			return ConvertPositionRepresentation<Ty>(StaticPosition{ pivotOffset }, StaticSize{ m_ownerFrame->GetSize() });
		}

		template<VariantMember<PositionVariant> Ty>
		Ty GetPivotedFramePositionAs() const noexcept
		{
			return std::visit([this](const auto& val) -> Ty
				{
					const Vector2 size = GetFrameSizeAs<StaticSize>().value;
					const Vector2 pivotOffset = { GetPivot().X() * -size.X(), GetPivot().Y() * -size.Y() };
					return ConvertPositionRepresentation<Ty>(StaticPosition{ GetLocalPositionAs<StaticPosition>().value + GetParentPivotedFrameStaticPosition().value + pivotOffset }, StaticSize{ m_ownerFrame->GetSize() });
				}, m_position);
		}

		template<VariantMember<PositionVariant> Ty>
		Ty GetFramePositionAs() const noexcept
		{
			return std::visit([this](const auto& val) -> Ty
				{
					return ConvertPositionRepresentation<Ty>(StaticPosition{ GetLocalPositionAs<StaticPosition>().value + GetParentPivotedFrameStaticPosition().value }, StaticSize{ m_ownerFrame->GetSize() });
				}, m_position);
		}

		template<VariantMember<SizeVariant> Ty>
		Ty GetFrameSizeAs() const noexcept
		{
			return ConvertSizeRepresentation<Ty>(GetLocalSizeAs<StaticSize>(), StaticSize{ m_ownerFrame->GetSize() });
		}

		template<VariantMember<PositionVariant> Ty>
		Ty GetLocalPositionAs() const noexcept
		{
			return std::visit([this](const auto& val) -> Ty
				{
					return ConvertPositionRepresentation<Ty>(val, GetParentStaticSize());
				}, m_position);
		}

		template<VariantMember<SizeVariant> Ty>
		Ty GetLocalSizeAs() const noexcept
		{
			return std::visit([this](const auto& val) -> Ty
				{
					return ConvertSizeRepresentation<Ty>(val, GetParentStaticSize());
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
					innerVal = ConvertPositionRepresentation<Inner_Ty>(val, GetParentStaticSize());
				}, m_position);
		}

		template<VariantMember<SizeVariant> Ty>
		void SetLocalSize(Ty val) noexcept
		{
			std::visit([this, &val](auto& innerVal)
				{
					using Inner_Ty = std::remove_cvref_t<decltype(innerVal)>;
					innerVal = ConvertSizeRepresentation<Inner_Ty>(val, GetParentStaticSize());
				}, m_size);
		}

		template<VariantMember<PositionVariant> Ty>
		void SetFramePosition(Ty val) noexcept
		{
			std::visit([this, &val](auto& innerVal)
				{
					using Inner_Ty = std::remove_cvref_t<decltype(innerVal)>;
					StaticPosition parentPosition = (m_parent) ? m_parent->GetPivotedFramePositionAs<StaticPosition>() : StaticPosition{};
					StaticPosition requestedPosition = ConvertPositionRepresentation<StaticPosition>(val, StaticSize{ m_ownerFrame->GetSize() });
					innerVal = ConvertPositionRepresentation<Inner_Ty>(StaticPosition{ requestedPosition.value - parentPosition.value }, GetParentStaticSize());
				}, m_position);
		}

		template<VariantMember<SizeVariant> Ty>
		void SetFrameSize(Ty val) noexcept
		{
			std::visit([this, &val](auto& innerVal)
				{
					using Inner_Ty = std::remove_cvref_t<decltype(innerVal)>;
					StaticSize requestedStaticSize = ConvertSizeRepresentation<StaticSize>(val, StaticSize{ m_ownerFrame->GetSize() });
					innerVal = ConvertSizeRepresentation<Inner_Ty>(requestedStaticSize, GetParentStaticSize());
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
				m_position = ConvertPivotEquivalentPosition(m_pivot, pivot, m_position, GetLocalSizeAs<RelativeSize>(), GetParentStaticSize());
				m_pivot = pivot;
				break;
			}
			default:
				break;// Should be unreachable
			}
		}

		StaticSize GetParentStaticSize() const noexcept
		{
			return m_parent ? m_parent->GetFrameSizeAs<StaticSize>() : StaticSize{ m_ownerFrame->GetSize() };
		}

		StaticPosition GetParentPivotedFrameStaticPosition() const noexcept
		{
			return m_parent ? m_parent->GetPivotedFramePositionAs<StaticPosition>() : StaticPosition{};
		}

		const UIFrame& GetFrame() const { return *m_ownerFrame; }
	};
}