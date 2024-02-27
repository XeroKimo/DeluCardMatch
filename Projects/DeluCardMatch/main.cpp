#include <SDL2/SDL.h>
//#include <SDL2/SDL_image.h>
#include <iostream>
#include <format>
#include <gsl/pointers>
#include <memory>
#include <vector>
#include <fstream>
#include <chrono>
#include <cmath>
#include <functional>
#include <box2d/box2d.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <variant>
#include <algorithm>

import DeluEngine;
import xk.Math.Matrix;
import DeluGame;
import SDL2pp;

#undef main;

using namespace xk::Math::Aliases;

struct ApplicationTimer
{
	std::chrono::steady_clock::time_point previousTick = std::chrono::steady_clock::now();
	std::chrono::nanoseconds currentDelta;

	template<std::invocable<std::chrono::nanoseconds> Callback>
	void Tick(Callback&& callback)
	{
		auto currentTick = std::chrono::steady_clock::now();
		currentDelta = currentTick - previousTick;
		callback(currentDelta);
		previousTick = currentTick;
	}
};

//Keeps the size of the element between all screen sizes
struct StaticSize
{
	Vector2 value;

	static StaticSize GetStaticSize(const StaticSize& s, StaticSize parentSize) noexcept
	{
		return s;
	}
};

//Keeps the percentage of the screen size constant between screen sizes
struct RelativeSize
{
	Vector2 value;

	static StaticSize GetStaticSize(const RelativeSize& s, StaticSize parentSize) noexcept
	{
		return StaticSize{ { parentSize.value.X() * s.value.X(), parentSize.value.Y() * s.value.Y() } };
	}
};

template<class Ty>
concept HasStaticSize = requires(Ty t, StaticSize parentSize)
{
	std::invoke(&Ty::GetStaticSize, t, parentSize);
};

template<class Ty>
concept HasStaticPosition = requires(Ty t, StaticSize parentSize)
{
	std::invoke(&Ty::GetStaticPosition, t, parentSize);
};

//Keeps the aspect ratio of the element constant between screen sizes
struct AspectRatioRelativeSize
{
	float ratio;
	float value;

	static StaticSize GetStaticSize(const AspectRatioRelativeSize& s, StaticSize parentSize) noexcept
	{
		const float parentRatio = parentSize.value.X() / parentSize.value.Y();
		const float parentToElementRatio = parentRatio / std::fabs(s.ratio);
		return (s.ratio >= 0) ?
			StaticSize{{ parentSize.value.X() * s.value, parentSize.value.Y() * s.value * parentToElementRatio }} :
			StaticSize{{ parentSize.value.X() * s.value / parentToElementRatio, parentSize.value.Y() * s.value }};
	}
};

//Keeps the edge of the element to the edge of the parent relatively constant between screen sizes
struct BorderConstantRelativeSize
{
	Vector2 value;

	static StaticSize GetStaticSize(const BorderConstantRelativeSize& s, StaticSize parentSize) noexcept
	{
		if(parentSize.value.X() >= parentSize.value.Y())
		{
			const float baseSize = parentSize.value.X() - parentSize.value.Y();
			const float remainingSize = parentSize.value.X() - baseSize;
			return StaticSize{ { baseSize + remainingSize * s.value.X(), parentSize.value.Y() * s.value.Y() } };
		}
		else
		{
			const float baseSize = parentSize.value.Y() - parentSize.value.X();
			const float remainingSize = parentSize.value.Y() - baseSize;
			return StaticSize{ { parentSize.value.X() * s.value.X(), baseSize + remainingSize * s.value.Y() } };
		}
	}
};

struct StaticPosition
{
	Vector2 value;

	static Vector2 GetStaticPosition(const StaticPosition& s, Vector2 parentSize) noexcept
	{
		return s.value;
	}
};

struct RelativePosition
{
	Vector2 value;

	static Vector2 GetStaticPosition(const RelativePosition& s, Vector2 parentSize) noexcept
	{
		return xk::Math::HadamardProduct(s.value, parentSize);
	}
};

using PositionVariant = std::variant<StaticPosition, RelativePosition>;
using SizeVariant = std::variant<StaticSize, RelativeSize, AspectRatioRelativeSize, BorderConstantRelativeSize>;

class UIFrame
{
public:
	SDL2pp::unique_ptr<SDL2pp::Texture> internalTexture;
	Vector2 GetSize() const noexcept { return internalTexture->GetSize(); }
};

enum class PivotChangePolicy
{
	//The underlying position will not change, will result in appearing in a different position however
	NoPositionChange,

	//The underlying position will change which will result in appearing in the same position
	NoVisualChange
};

struct UIElement;

template<class T, class U> struct IsVariantMember;

template<class T, class... Ts> 
struct IsVariantMember<T, std::variant<Ts...>> : std::bool_constant<(std::same_as<T, Ts> || ...)>
{
};

template<class Ty, class VariantTy>
concept VariantMember = (IsVariantMember<Ty, VariantTy>::value);

template<VariantMember<PositionVariant> ToType, VariantMember<PositionVariant> FromType>
ToType ConvertPositionRepresentation(const FromType& position, StaticSize parentStaticSize) = delete;

template<>
StaticPosition ConvertPositionRepresentation<StaticPosition, StaticPosition>(const StaticPosition& position, StaticSize parentStaticSize) noexcept
{
	return position;
}

template<>
RelativePosition ConvertPositionRepresentation<RelativePosition, StaticPosition>(const StaticPosition& position, StaticSize parentStaticSize) noexcept
{
	return RelativePosition{ xk::Math::HadamardDivision(position.value, parentStaticSize.value) };
}

template<>
StaticPosition ConvertPositionRepresentation<StaticPosition, RelativePosition>(const RelativePosition& position, StaticSize parentStaticSize) noexcept
{
	return StaticPosition{ xk::Math::HadamardProduct(position.value, parentStaticSize.value) };
}

template<>
RelativePosition ConvertPositionRepresentation<RelativePosition, RelativePosition>(const RelativePosition& position, StaticSize parentStaticSize) noexcept
{
	return position;
}

template<VariantMember<SizeVariant> ToType, VariantMember<SizeVariant> FromType>
ToType ConvertSizeRepresentation(const FromType& size, StaticSize parentStaticSize) = delete;

template<>
StaticSize ConvertSizeRepresentation<StaticSize, StaticSize>(const StaticSize& size, StaticSize parentStaticSize) noexcept
{
	return size;
}

template<>
RelativeSize ConvertSizeRepresentation<RelativeSize, StaticSize>(const StaticSize& size, StaticSize parentStaticSize) noexcept
{
	return RelativeSize{ xk::Math::HadamardDivision(size.value, parentStaticSize.value) };
}

template<> 
AspectRatioRelativeSize ConvertSizeRepresentation<AspectRatioRelativeSize, StaticSize>(const StaticSize& size, StaticSize parentStaticSize) noexcept
{
	return AspectRatioRelativeSize{ .ratio = size.value.X() / size.value.Y(), .value = size.value.X() / parentStaticSize.value.X() };
}

template<>
BorderConstantRelativeSize ConvertSizeRepresentation<BorderConstantRelativeSize, StaticSize>(const StaticSize& size, StaticSize parentStaticSize) noexcept
{
	const Vector2 baseSize = [parentStaticSize] ()
		{ 
			const float min = std::min(parentStaticSize.value.X(), parentStaticSize.value.Y());
			return Vector2{ min, min };
		}();
	const Vector2 variableSize = [parentStaticSize, baseSize] 
		{ 
			const float max = std::max(parentStaticSize.value.X(), parentStaticSize.value.Y());
			return Vector2{ max, max } - baseSize;
		}();

	return BorderConstantRelativeSize{ xk::Math::HadamardDivision(size.value - baseSize, variableSize) };
}

template<>
StaticSize ConvertSizeRepresentation<StaticSize, RelativeSize>(const RelativeSize& size, StaticSize parentStaticSize) noexcept
{
	return StaticSize{ RelativeSize::GetStaticSize(size, parentStaticSize) };
}

template<>
RelativeSize ConvertSizeRepresentation<RelativeSize, RelativeSize>(const RelativeSize& size, StaticSize parentStaticSize) noexcept
{
	return size;
}

template<>
AspectRatioRelativeSize ConvertSizeRepresentation<AspectRatioRelativeSize, RelativeSize>(const RelativeSize& size, StaticSize parentStaticSize) noexcept
{
	return ConvertSizeRepresentation<AspectRatioRelativeSize>(StaticSize{ RelativeSize::GetStaticSize(size, parentStaticSize) }, parentStaticSize);
}

template<>
BorderConstantRelativeSize ConvertSizeRepresentation<BorderConstantRelativeSize, RelativeSize>(const RelativeSize& size, StaticSize parentStaticSize) noexcept
{
	return ConvertSizeRepresentation<BorderConstantRelativeSize>(StaticSize{ RelativeSize::GetStaticSize(size, parentStaticSize) }, parentStaticSize);
}

template<>
StaticSize ConvertSizeRepresentation<StaticSize, AspectRatioRelativeSize>(const AspectRatioRelativeSize& size, StaticSize parentStaticSize) noexcept
{
	return StaticSize{ AspectRatioRelativeSize::GetStaticSize(size, parentStaticSize) };
}

template<>
RelativeSize ConvertSizeRepresentation<RelativeSize, AspectRatioRelativeSize>(const AspectRatioRelativeSize& size, StaticSize parentStaticSize) noexcept
{
	const Vector2 staticSize = AspectRatioRelativeSize::GetStaticSize(size, parentStaticSize).value;
	return RelativeSize{ xk::Math::HadamardDivision(staticSize, parentStaticSize.value) };
}

template<>
AspectRatioRelativeSize ConvertSizeRepresentation<AspectRatioRelativeSize, AspectRatioRelativeSize>(const AspectRatioRelativeSize& size, StaticSize parentStaticSize) noexcept
{
	return size;
}

template<>
BorderConstantRelativeSize ConvertSizeRepresentation<BorderConstantRelativeSize, AspectRatioRelativeSize>(const AspectRatioRelativeSize& size, StaticSize parentStaticSize) noexcept
{
	return ConvertSizeRepresentation<BorderConstantRelativeSize>(StaticSize{ AspectRatioRelativeSize::GetStaticSize(size, parentStaticSize) }, parentStaticSize);
}

template<>
StaticSize ConvertSizeRepresentation<StaticSize, BorderConstantRelativeSize>(const BorderConstantRelativeSize& size, StaticSize parentStaticSize) noexcept
{
	return StaticSize{ BorderConstantRelativeSize::GetStaticSize(size, parentStaticSize) };
}

template<>
RelativeSize ConvertSizeRepresentation<RelativeSize, BorderConstantRelativeSize>(const BorderConstantRelativeSize& size, StaticSize parentStaticSize) noexcept
{
	const Vector2 staticSize = BorderConstantRelativeSize::GetStaticSize(size, parentStaticSize).value;
	return RelativeSize{ xk::Math::HadamardDivision(staticSize, parentStaticSize.value) };
}

template<>
AspectRatioRelativeSize ConvertSizeRepresentation<AspectRatioRelativeSize, BorderConstantRelativeSize>(const BorderConstantRelativeSize& size, StaticSize parentStaticSize) noexcept
{
	return ConvertSizeRepresentation<AspectRatioRelativeSize>(StaticSize{ BorderConstantRelativeSize::GetStaticSize(size, parentStaticSize) }, parentStaticSize);
}

template<>
BorderConstantRelativeSize ConvertSizeRepresentation<BorderConstantRelativeSize, BorderConstantRelativeSize>(const BorderConstantRelativeSize& size, StaticSize parentStaticSize) noexcept
{
	return size;
}

//Calculates an equivalent position that would not affect the visual position of an element between different pivots
StaticPosition ConvertPivotEquivalentStaticPosition(Vector2 fromPivot, Vector2 toPivot, StaticPosition fromStaticPosition, RelativeSize elementRelativeSize, StaticSize parentSize) noexcept
{
	const Vector2 pivotDiff = toPivot - fromPivot;
	const Vector2 offset = Vector2{ pivotDiff.X() * elementRelativeSize.value.X(), pivotDiff.Y() * elementRelativeSize.value.Y() };
	return StaticPosition{ fromStaticPosition.value + ConvertSizeRepresentation<StaticSize>(RelativeSize{ offset }, parentSize).value };
}

//Calculates an equivalent position that would not affect the visual position of an element between different pivots
RelativePosition ConvertPivotEquivalentRelativePosition(Vector2 fromPivot, Vector2 toPivot, RelativePosition fromRelativePosition, RelativeSize elementRelativeSize, StaticSize parentSize) noexcept
{
	const Vector2 pivotDiff = toPivot - fromPivot;
	const Vector2 offset = Vector2{ pivotDiff.X() * elementRelativeSize.value.X(), pivotDiff.Y() * elementRelativeSize.value.Y() };
	return RelativePosition{ fromRelativePosition.value + offset };
}

//Calculates an equivalent position that would not affect the visual position of an element between different pivots
PositionVariant ConvertPivotEquivalentPosition(Vector2 fromPivot, Vector2 toPivot, PositionVariant fromPosition, RelativeSize elementRelativeSize, StaticSize parentSize) noexcept
{
	static_assert(std::same_as<PositionVariant, std::variant<StaticPosition, RelativePosition>>, "Position variant changed, function requires changes");
	if(auto* position = std::get_if<StaticPosition>(&fromPosition); position)
	{
		return StaticPosition{ ConvertPivotEquivalentStaticPosition(fromPivot, toPivot, *position, elementRelativeSize, parentSize) };
	}
	else
	{
		return RelativePosition{ ConvertPivotEquivalentRelativePosition(fromPivot, toPivot, std::get<RelativePosition>(fromPosition), elementRelativeSize, parentSize) };
	}
}

struct UIElement
{

private:
	UIFrame* m_ownerFrame;
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
	Ty GetFramePositionAs() const noexcept
	{
		return std::visit([this](const auto& val) -> Ty
			{
				return ::ConvertPositionRepresentation<Ty>(StaticPosition{ GetParentFrameStaticPosition().value + GetLocalPositionAs<StaticPosition>().value }, StaticSize{ m_ownerFrame->GetSize() });
			}, m_position);
	}

	template<VariantMember<SizeVariant> Ty>
	Ty GetFrameSizeAs() const noexcept
	{
		return ::ConvertSizeRepresentation<Ty>(GetLocalSizeAs<StaticSize>(), StaticSize{ m_ownerFrame->GetSize() });
	}

	template<VariantMember<PositionVariant> Ty>
	Ty GetLocalPositionAs() const noexcept
	{
		return std::visit([this](const auto& val) -> Ty
			{
				return ::ConvertPositionRepresentation<Ty>(val, GetParentStaticSize());
			}, m_position);
	}

	template<VariantMember<SizeVariant> Ty>
	Ty GetLocalSizeAs() const noexcept
	{
		return std::visit([this](const auto& val) -> Ty
			{
				return ::ConvertSizeRepresentation<Ty>(val, GetParentStaticSize());
			}, m_size);
	}

	template<VariantMember<PositionVariant> Ty>
	void ConvertPositionRepresentation() noexcept
	{
		static_assert(std::same_as<PositionVariant, std::variant<StaticPosition, RelativePosition>>, "Position variant changed");

		m_position = GetLocalPositionAs<Ty>();
	}

	template<VariantMember<SizeVariant> Ty>
	void ConvertSizeRepresentation() noexcept
	{
		m_size = GetLocalSizeAs<Ty>();
	}

	template<VariantMember<PositionVariant> Ty>
	void SetLocalPosition(Ty val) noexcept
	{
		m_position = std::visit([this, &val](const auto& innerVal)
			{
				using Inner_Ty = std::remove_cvref_t<decltype(innerVal)>;
				return ::ConvertPositionRepresentation<Inner_Ty>(val, GetParentStaticSize());
			}, m_position);
	}

	template<VariantMember<SizeVariant> Ty>
	void SetLocalSize(Ty val) noexcept
	{
		m_size = std::visit([this, &val](const auto& innerVal)
			{
				using Inner_Ty = std::remove_cvref_t<decltype(innerVal)>;
				return ::ConvertSizeRepresentation<Inner_Ty>(val, GetParentStaticSize());
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

	void SetStaticPosition(Vector2 newPos) noexcept
	{
		std::visit([this, newPos](auto& position)
			{
				using Ty = std::remove_reference_t<decltype(position)>;
				position.value = ::ConvertPositionRepresentation<Ty>(StaticPosition{ newPos }, GetParentStaticSize()).value;
			}, m_position);
	}

	void SetRelativePosition(Vector2 newPos) noexcept
	{
		std::visit([this, newPos](auto& position)
			{
				using Ty = std::remove_reference_t<decltype(position)>;
				position.value = ::ConvertPositionRepresentation<Ty>(RelativePosition{ newPos }, GetParentStaticSize()).value;
			}, m_position);
	}

	StaticSize GetParentStaticSize() const noexcept
	{
		return StaticSize{ m_ownerFrame->GetSize() };
	}

	StaticPosition GetParentFrameStaticPosition() const noexcept
	{
		Vector2 accumulator;

		return StaticPosition{ accumulator };
	}

	const UIFrame& GetFrame() const { return *m_ownerFrame; }
};

void SetAnchors(UIElement& element, Vector2 minAnchor, Vector2 maxAnchor)
{
	const Vector2 size = maxAnchor - minAnchor;
	element.SetSizeRepresentation(RelativeSize{ size });
	element.SetPositionRepresentation(RelativePosition{ minAnchor + Vector2{ size.X() * element.GetPivot().X(), size.Y() * element.GetPivot().Y() } });
}

SDL2pp::FRect GetRect(const UIElement& element)
{
	SDL2pp::FRect rect;
	const Vector2 size = element.GetFrameSizeAs<StaticSize>().value;
	const Vector2 pivotPositionOffset = { element.GetPivot().X() * -size.X(), (1 - element.GetPivot().Y()) * -size.Y() };
	const Vector2 sdl2YFlip = { 0, element.GetFrame().GetSize().Y() };
	const Vector2 position = xk::Math::HadamardProduct(element.GetFramePositionAs<StaticPosition>().value, Vector2 { 1, -1 }) + pivotPositionOffset + sdl2YFlip;

		rect.x = position.X();
		rect.y = position.Y();
		rect.w = size.X();
		rect.h = size.Y();
		return rect;
}

bool IsOverlapping(Vector2 mousePos, const UIElement& element)
{
	auto size = element.GetFrameSizeAs<RelativeSize>().value;
	auto minPos = ConvertPivotEquivalentRelativePosition(element.GetPivot(), { 0, 0 }, element.GetFramePositionAs<RelativePosition>(), element.GetFrameSizeAs<RelativeSize>(), StaticSize{ element.GetFrame().GetSize() }).value;
	return mousePos.X() >= minPos.X() && mousePos.X() <= minPos.X() + size.X() &&
		mousePos.Y() >= minPos.Y() && mousePos.Y() <= minPos.Y() + size.Y();
}

int main()
{
	ApplicationTimer timer;
	DeluEngine::Engine engine
	{
		.window{ SDL2pp::CreateWindow("Bullet Hell", { 1600, 900 }, SDL2pp::WindowFlag::OpenGL) },
		.renderer{ engine.window.get() }
	};
	engine.box2DCallbacks.engine = &engine;
	engine.physicsWorld.SetContactListener(&engine.box2DCallbacks);
	engine.physicsWorld.SetDebugDraw(&engine.box2DCallbacks);
	DeluEngine::Input::defaultController = &engine.controller;
	engine.CreateScene(GameMain(engine));
	engine.renderer.debugCallbacks.push_back([&engine](DeluEngine::DebugRenderer& renderer)
		{
			engine.scene->DebugDraw(renderer);
			engine.box2DCallbacks.SetFlags(b2Draw::e_shapeBit);
			engine.physicsWorld.DebugDraw();
		});
	std::chrono::duration<float> physicsAccumulator{ 0.f };

	UIFrame frame;
	frame.internalTexture = engine.renderer.backend->CreateTexture(
		SDL_PIXELFORMAT_RGBA32,
		SDL2pp::TextureAccess(SDL_TEXTUREACCESS_STATIC | SDL_TEXTUREACCESS_TARGET),
		1600, 900);
	frame.internalTexture->SetBlendMode(SDL_BLENDMODE_BLEND);

	TTF_Font* testFont = TTF_OpenFont("Arial.ttf", 12);
	//SDL_Surface* testFontSurface = TTF_RenderText_Solid(testFont, "Test test, 1. 2. 3", { 255, 255, 255 });
	SDL_Surface* testFontSurface = TTF_RenderUTF8_Solid_Wrapped(testFont, "Test test, 1. 2. 3", { 255, 255, 255 }, 50);
	//SDL_Surface* testFontSurface = TTF_RenderText_Shaded(testFont, "Test test, 1. 2. 3", { 255, 255, 255 }, { 0, 0, 0 });
	SDL_Texture* testFontTexture = SDL_CreateTextureFromSurface(engine.renderer.backend.get(), testFontSurface);
	SDL_Surface* testSurface = IMG_Load("Cards/syobontaya.png");

	UIElement testElement{ frame };
	testElement.SetPositionRepresentation(RelativePosition{ { 0.5f, 0.5f } });
	testElement.SetSizeRepresentation(RelativeSize({ 0.8f, 0.8f }));
	testElement.SetPivot({ 0.8f, 0.7f });
	testElement.texture = engine.renderer.backend->CreateTexture(testSurface);
	testElement.ConvertPositionRepresentation<StaticPosition>();
	testElement.ConvertSizeRepresentation<StaticSize>();
	testElement.ConvertSizeRepresentation<RelativeSize>();
	UIElement testElement2 = testElement;
	testElement2.SetPivot({ 0.0f, 0.0f });
	//SetAnchors(testElement2, { 0.05f, 0.05f }, { 0.95f, 0.95f });
	//testElement2.SetSizeRepresentation(BorderConstantRelativeSize({ 0.8f, 0.8f }));
	testElement2.SetSizeRepresentation(AspectRatioRelativeSize{ .ratio = 9.f/ 16.f, .value = 0.5f });
	testElement2.SetStaticPosition({ 950.33f, 0.0f });
	//testElement2.position = StaticPosition{ { 500, 300 } };

	//testElement2.SetPivot({ 0.8f, 0.7f });
	//testElement2.SetPivot({ 0.8f, 0.7f }, PivotChangePolicy::NoVisualChange);
	//testElement2.position = ConvertPivotEquivalentPosition(testElement.m_pivot, testElement2.m_pivot, testElement.position, testElement2.GetRelativeSizeToParent(), frame.size);
	//testElement2.position = ConvertPivotEquivalentPosition(testElement2.m_pivot, testElement.m_pivot, testElement2.position, testElement2.GetRelativeSizeToParent(), frame.size);
	//testElement2.pivot = testElement.pivot;
	std::chrono::duration<float> accumulator{ 0 };

	UIElement mouseDebug{ frame };
	{
		mouseDebug.texture = engine.renderer.backend->CreateTexture(SDL_PIXELFORMAT_RGBA32, static_cast<SDL2pp::TextureAccess>(SDL_TEXTUREACCESS_STATIC | SDL_TEXTUREACCESS_TARGET), 8, 8);
	};
	engine.renderer.backend->SetRenderTarget(mouseDebug.texture.get());
	engine.renderer.backend->SetDrawColor(SDL2pp::Color{ 255, 0, 0, 255 });

	engine.renderer.backend->Clear();

	engine.renderer.backend->SetRenderTarget(nullptr);
	mouseDebug.SetSizeRepresentation(StaticSize{ { 8, 8 } });
	mouseDebug.SetPivot({ 0.5f, 0.5f });
	UIElement* hoveredElement = nullptr;
	while(true)
	{
		SDL2pp::Event event;
		if(SDL2pp::PollEvent(event))
		{
			if(event.type == SDL2pp::EventType::SDL_QUIT)
			{
				break;
			}
			if(event.type == SDL2pp::EventType::SDL_MOUSEMOTION)
			{
				mouseDebug.SetRelativePosition(xk::Math::HadamardDivision(Vector2{ event.motion.x, engine.renderer.backend->GetOutputSize().Y() - event.motion.y }, engine.renderer.backend->GetOutputSize()));
				//mouseDebug.position.value = ;
				//std::cout << mouseDebug.position.value.X() << ", " << mouseDebug.position.value.Y() << "\n";
				//mouseDebug.position.value.X() /= static_cast<float>(engine.renderer.backend->GetOutputSize().X());
				//mouseDebug.position.value.Y() /= static_cast<float>(engine.renderer.backend->GetOutputSize().Y());
			}
			if(event.type == SDL2pp::EventType::SDL_MOUSEBUTTONDOWN)
			{
				if(event.button.button == SDL_BUTTON_LEFT && hoveredElement)
				{
					std::cout << "Button clicked\n";
				}
			}
			if(event.type == SDL2pp::EventType::SDL_MOUSEBUTTONUP)
			{
				if(event.button.button == SDL_BUTTON_LEFT && hoveredElement)
				{
					std::cout << "Button Released\n";
				}
			}
			engine.ProcessEvent(event);
		}
		else
		{
			engine.controllerContext.Execute(engine.controller);
			timer.Tick([&](std::chrono::nanoseconds dt)
				{
					//testElement.position.Y() = testElement.size.Y() * std::sin(accumulator.count());
					testElement.SetPivot({ testElement.GetPivot().X(), (std::sin(accumulator.count()) + 1) / 2 });
					//std::cout << testElement.pivot.Y() << "\n";
					std::chrono::duration<float> deltaTime = std::chrono::duration_cast<std::chrono::duration<float>>(dt);
					static constexpr std::chrono::duration<float> physicsStep{ 1.f / 60.f };
					physicsAccumulator += deltaTime;
					accumulator += deltaTime;
					while(physicsAccumulator >= physicsStep)
					{
						engine.physicsWorld.Step(physicsStep.count(), 8, 3);
						physicsAccumulator -= physicsStep;

						//std::cout << "Body pos " << b->GetPosition().x << ", " << b->GetPosition().y << "\n";
					}
					engine.scene->Update(deltaTime.count());
				});

			engine.controller.SwapBuffers();

			engine.renderer.Render();
			{
				engine.renderer.backend->SetRenderTarget(frame.internalTexture.get());
				engine.renderer.backend->SetDrawColor(SDL2pp::Color{ 255, 255, 255, 0 });

				engine.renderer.backend->Clear();
				SDL_Rect textLocation = { 400, 200, testFontSurface->w, testFontSurface->h };
				engine.renderer.backend->Copy(testFontTexture, std::nullopt, textLocation);

				engine.renderer.backend->CopyEx(testElement.texture.get(), std::nullopt, GetRect( testElement), 0, SDL2pp::FPoint{ 0, 0 }, SDL2pp::RendererFlip::SDL_FLIP_NONE);
				engine.renderer.backend->CopyEx(testElement2.texture.get(), std::nullopt, GetRect( testElement2), 0, SDL2pp::FPoint{ 0, 0 }, SDL2pp::RendererFlip::SDL_FLIP_NONE);
				engine.renderer.backend->CopyEx(mouseDebug.texture.get(), std::nullopt, GetRect( mouseDebug), 0, SDL2pp::FPoint{ 0, 0 }, SDL2pp::RendererFlip::SDL_FLIP_NONE);

				hoveredElement = nullptr;

				if(IsOverlapping(mouseDebug.GetFramePositionAs<RelativePosition>().value, testElement))
				{
					hoveredElement = &testElement;
					//std::cout << "Overlapping first element\n";
				}
				if(IsOverlapping(mouseDebug.GetFramePositionAs<RelativePosition>().value, testElement2))
				{
					hoveredElement = &testElement2;
					//std::cout << "Overlapping second element\n";
				}
				engine.renderer.backend->SetRenderTarget(nullptr);
				engine.renderer.backend->Copy(frame.internalTexture.get(), std::nullopt, std::optional<SDL2pp::Rect>(std::nullopt));
			}

			engine.renderer.backend->Present();
		}
	}

	return 0;
}
