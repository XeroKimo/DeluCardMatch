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

template<class Ty>
concept HasStaticSize = requires(Ty t, Vector2 parentSize)
{
	std::invoke(&Ty::GetStaticSize, t, parentSize);
};

template<class Ty>
concept HasStaticPosition = requires(Ty t, Vector2 parentSize)
{
	std::invoke(&Ty::GetStaticPosition, t, parentSize);
};


	//Keeps the size of the element between all screen sizes
struct StaticSize
{
	Vector2 value;

	static Vector2 GetStaticSize(const StaticSize& s, Vector2 parentSize) noexcept
	{
		return s.value;
	}
};

//Keeps the percentage of the screen size constant between screen sizes
struct RelativeSize
{
	Vector2 value;

	static Vector2 GetStaticSize(const RelativeSize& s, Vector2 parentSize) noexcept
	{
		return { parentSize.X() * s.value.X(), parentSize.Y() * s.value.Y() };
	}
};

//Keeps the aspect ratio of the element constant between screen sizes
struct AspectRatioRelativeSize
{
	float ratio;
	float value;

	static Vector2 GetStaticSize(const AspectRatioRelativeSize& s, Vector2 parentSize) noexcept
	{
		const float parentRatio = parentSize.X() / parentSize.Y();
		const float parentToElementRatio = parentRatio / std::fabs(s.ratio);
		return (s.ratio >= 0) ?
			Vector2{ parentSize.X() * s.value, parentSize.Y() * s.value * parentToElementRatio } :
			Vector2{ parentSize.X() * s.value / parentToElementRatio, parentSize.Y() * s.value };
	}
};

//Keeps the edge of the element to the edge of the parent relatively constant between screen sizes
struct BorderConstantRelativeSize
{
	Vector2 value;

	static Vector2 GetStaticSize(const BorderConstantRelativeSize& s, Vector2 parentSize) noexcept
	{
		if(parentSize.X() >= parentSize.Y())
		{
			const float baseSize = parentSize.X() - parentSize.Y();
			const float remainingSize = parentSize.X() - baseSize;
			return { baseSize + remainingSize * s.value.X(), parentSize.Y() * s.value.Y() };
		}
		else
		{
			const float baseSize = parentSize.Y() - parentSize.X();
			const float remainingSize = parentSize.Y() - baseSize;
			return { parentSize.X() * s.value.X(), baseSize + remainingSize * s.value.Y() };
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

struct UIFrame
{
	Vector2 size;
	SDL2pp::unique_ptr<SDL2pp::Texture> internalTexture;
};

Vector2 StaticPositionToRelativePosition(Vector2 requestedPosition, Vector2 parentSize) noexcept
{
	return xk::Math::HadamardDivision(requestedPosition, parentSize);
}

Vector2 RelativePositionToStaticPosition(Vector2 requestedPosition, Vector2 parentSize) noexcept
{
	return xk::Math::HadamardProduct(requestedPosition, parentSize);
}

Vector2 StaticSizeToRelativeSize(Vector2 requestedSize, Vector2 parentSize)
{
	return { requestedSize.X() / parentSize.X(), requestedSize.Y() / parentSize.Y() };
}

Vector2 RelativeSizeToStaticSize(Vector2 requestedSize, Vector2 parentSize)
{
	return { requestedSize.X() * parentSize.X(), requestedSize.Y() * parentSize.Y() };
}

enum class PivotChangePolicy
{
	//The underlying position will not change, will result in appearing in a different position however
	NoPositionChange,

	//The underlying position will change which will result in appearing in the same position
	NoVisualChange
};

struct UIElement;

Vector2 ConvertPivotEquivalentStaticPosition(Vector2 fromPivot, Vector2 toPivot, Vector2 fromStaticPosition, Vector2 elementRelativeSize, Vector2 parentSize) noexcept
{
	const Vector2 pivotDiff = toPivot - fromPivot;
	const Vector2 offset = Vector2{ pivotDiff.X() * elementRelativeSize.X(), pivotDiff.Y() * elementRelativeSize.Y() };
	return fromStaticPosition + RelativeSizeToStaticSize(offset, parentSize);
}

Vector2 ConvertPivotEquivalentRelativePosition(Vector2 fromPivot, Vector2 toPivot, Vector2 fromRelativePosition, Vector2 elementRelativeSize, Vector2 parentSize) noexcept
{
	const Vector2 pivotDiff = toPivot - fromPivot;
	const Vector2 offset = Vector2{ pivotDiff.X() * elementRelativeSize.X(), pivotDiff.Y() * elementRelativeSize.Y() };
	return fromRelativePosition + offset;
}

PositionVariant ConvertPivotEquivalentPosition(Vector2 fromPivot, Vector2 toPivot, PositionVariant fromPosition, Vector2 elementRelativeSize, Vector2 parentSize) noexcept
{
	static_assert(std::same_as<PositionVariant, std::variant<StaticPosition, RelativePosition>>, "Position variant changed, function requires changes");
	if(auto* position = std::get_if<StaticPosition>(&fromPosition); position)
	{
		return StaticPosition{ ConvertPivotEquivalentStaticPosition(fromPivot, toPivot, position->value, elementRelativeSize, parentSize) };
	}
	else
	{
		return RelativePosition{ ConvertPivotEquivalentRelativePosition(fromPivot, toPivot, std::get<RelativePosition>(fromPosition).value, elementRelativeSize, parentSize) };
	}
}

template<class T, class U> struct IsVariantMember;

template<class T, class... Ts> 
struct IsVariantMember<T, std::variant<Ts...>> : std::bool_constant<(std::same_as<T, Ts> || ...)>
{
};

template<class Ty, class VariantTy>
concept VariantMember = (IsVariantMember<Ty, VariantTy>::value);

struct UIElement
{

	UIFrame* ownerFrame;

private:
	PositionVariant m_position;
	SizeVariant m_size;
	Vector2 m_pivot;

public:
	SDL2pp::shared_ptr<SDL2pp::Texture> texture;

	Vector2 GetPivot() const noexcept
	{
		return m_pivot;
	}

	Vector2 GetStaticSize() const noexcept
	{
		return std::visit([this](const auto& val) -> Vector2
			{
				using Ty = std::remove_cvref_t<decltype(val)>;

				static_assert(HasStaticSize<Ty>, "One of size representation does not contain a static size operation");

				return std::invoke(&Ty::GetStaticSize, val, GetParentStaticSize());
			}, m_size);
	}

	Vector2 GetRelativeSizeToParent() const noexcept
	{
		return StaticSizeToRelativeSize(GetStaticSize(), GetParentStaticSize());
	}

	Vector2 GetRelativeSizeToFrame() const noexcept
	{
		return StaticSizeToRelativeSize(GetStaticSize(), ownerFrame->size);
	}

	Vector2 GetStaticPosition() const noexcept
	{
		return std::visit([this](const auto& val) -> Vector2
			{
				using Ty = std::remove_cvref_t<decltype(val)>;

				static_assert(HasStaticPosition<Ty>, "One of position representation does not contain a static position operation");

				return std::invoke(&Ty::GetStaticPosition, val, GetParentStaticSize());
			}, m_position);
	}

	Vector2 GetRelativePosition() const noexcept
	{
		return StaticPositionToRelativePosition(GetStaticPosition(), GetParentStaticSize());
	}

	template<VariantMember<PositionVariant> Ty>
	void ConvertPositionRepresentation() noexcept
	{
		static_assert(std::same_as<PositionVariant, std::variant<StaticPosition, RelativePosition>>, "Position variant changed");
		if constexpr(std::same_as<Ty, StaticPosition>)
		{
			if(auto internalPos = std::get_if<RelativePosition>(&m_position); internalPos)
			{
				m_position = StaticPosition{ RelativePositionToStaticPosition(internalPos->value, GetParentStaticSize()) };
			}
		}
		else if constexpr(std::same_as<Ty, RelativePosition>)
		{
			if(auto internalPos = std::get_if<StaticPosition>(&m_position); internalPos)
			{
				m_position = RelativePosition{ StaticPositionToRelativePosition(internalPos->value, GetParentStaticSize()) };
			}
		}
	}

	template<VariantMember<SizeVariant> Ty>
	void ConvertSizeRepresentation()
	{
		if constexpr(std::same_as<Ty, StaticSize>)
		{
			m_size = StaticSize{ GetStaticSize() };
		}
		else if constexpr(std::same_as<Ty, RelativeSize>)
		{
			m_size = RelativeSize{ GetRelativeSizeToParent() };
		}
		else if constexpr(std::same_as<Ty, AspectRatioRelativeSize>)
		{
			const Vector2 staticSize = GetStaticSize();
			const Vector2 parentSize = GetParentSize();
			m_size = AspectRatioRelativeSize{ .ratio = staticSize.X() / staticSize.Y(), staticSize.X() / parentSize.X() };
		}
		else if constexpr(std::same_as<Ty, BorderConstantRelativeSize>)
		{
			const Vector2 parentSize = GetParentStaticSize();
			const Vector2 baseSize = Vector2{ std::min(parentSize.X(), parentSize.Y())};
			const Vector2 variableSize = Vector2{ std::max(parentSize.X(), parentSize.Y()) };
			
			m_size = BorderConstantRelativeSize{ xk::Math::HadamardDivision(GetStaticSize() - baseSize, variableSize) };
		}
	}

	template<VariantMember<PositionVariant> Ty>
	void SetPositionRepresentation(Ty val)
	{
		m_position = val;
	}

	template<VariantMember<SizeVariant> Ty>
	void SetSizeRepresentation(Ty val)
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
			m_position = ConvertPivotEquivalentPosition(m_pivot, pivot, m_position, GetRelativeSizeToParent(), GetParentStaticSize());
			m_pivot = pivot;
			break;
		}
		default:
			break;// Should be unreachable
		}
	}

	void SetStaticPosition(Vector2 newPos) noexcept
	{
		if(auto* internalPos = std::get_if<StaticPosition>(&m_position); internalPos)
		{
			internalPos->value = newPos;
		}
		else if(auto* internalPos = std::get_if<RelativePosition>(&m_position); internalPos)
		{
			internalPos->value = StaticPositionToRelativePosition(newPos, GetParentStaticSize());
		}
	}

	void SetRelativePosition(Vector2 newPos) noexcept
	{
		if(auto* internalPos = std::get_if<StaticPosition>(&m_position); internalPos)
		{
			internalPos->value = RelativePositionToStaticPosition(newPos, GetParentStaticSize());;
		}
		else if(auto* internalPos = std::get_if<RelativePosition>(&m_position); internalPos)
		{
			internalPos->value = newPos;
		}
	}

	Vector2 GetParentStaticSize() const noexcept
	{
		return ownerFrame->size;
	}
};

void SetAnchors(UIElement& element, Vector2 minAnchor, Vector2 maxAnchor)
{
	const Vector2 size = maxAnchor - minAnchor;
	element.SetSizeRepresentation(RelativeSize{ size });
	element.SetPositionRepresentation(RelativePosition{ minAnchor + Vector2{ size.X() * element.GetPivot().X(), size.Y() * element.GetPivot().Y() } });
}

SDL2pp::FRect GetRect(const UIFrame& frame, const UIElement& element)
{
	SDL2pp::FRect rect;
	const Vector2 size = element.GetStaticSize();
	const Vector2 pivotPositionOffset = { element.GetPivot().X() * -size.X(), (1 - element.GetPivot().Y()) * -size.Y() };
	const Vector2 sdl2YFlip = { 0, frame.size.Y() };
	const Vector2 position = [&frame, &element]() -> Vector2
		{
			return { frame.size.X() * element.GetRelativePosition().X(), frame.size.Y() * -element.GetRelativePosition().Y() };

		}() + pivotPositionOffset + sdl2YFlip;

		rect.x = position.X();
		rect.y = position.Y();
		rect.w = size.X();
		rect.h = size.Y();
		return rect;
}

bool IsOverlapping(Vector2 mousePos, const UIElement& element, const UIFrame& frame)
{
	auto size = element.GetRelativeSizeToFrame();
	auto minPos = ConvertPivotEquivalentRelativePosition(element.GetPivot(), { 0, 0 }, element.GetRelativePosition(), element.GetRelativeSizeToFrame(), frame.size);
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

	UIFrame frame{ { 1600, 900} };
	frame.internalTexture = engine.renderer.backend->CreateTexture(
		SDL_PIXELFORMAT_RGBA32,
		SDL2pp::TextureAccess(SDL_TEXTUREACCESS_STATIC | SDL_TEXTUREACCESS_TARGET),
		frame.size.X(), frame.size.Y());
	frame.internalTexture->SetBlendMode(SDL_BLENDMODE_BLEND);

	TTF_Font* testFont = TTF_OpenFont("Arial.ttf", 12);
	//SDL_Surface* testFontSurface = TTF_RenderText_Solid(testFont, "Test test, 1. 2. 3", { 255, 255, 255 });
	SDL_Surface* testFontSurface = TTF_RenderUTF8_Solid_Wrapped(testFont, "Test test, 1. 2. 3", { 255, 255, 255 }, 50);
	//SDL_Surface* testFontSurface = TTF_RenderText_Shaded(testFont, "Test test, 1. 2. 3", { 255, 255, 255 }, { 0, 0, 0 });
	SDL_Texture* testFontTexture = SDL_CreateTextureFromSurface(engine.renderer.backend.get(), testFontSurface);
	SDL_Surface* testSurface = IMG_Load("Cards/syobontaya.png");

	UIElement testElement;
	testElement.SetPositionRepresentation(RelativePosition{ { 0.5f, 0.5f } });
	testElement.SetSizeRepresentation(RelativeSize({ 0.8f, 0.8f }));
	testElement.SetPivot({ 0.8f, 0.7f });
	testElement.texture = engine.renderer.backend->CreateTexture(testSurface);
	testElement.ownerFrame = &frame;
	testElement.ConvertPositionRepresentation<StaticPosition>();
	UIElement testElement2 = testElement;
	testElement2.SetPivot({ 0.0f, 0.0f });
	//SetAnchors(testElement2, { 0.05f, 0.05f }, { 0.95f, 0.95f });
	//testElement2.SetSizeRepresentation(BorderConstantRelativeSize({ 0.8f, 0.8f }));
	testElement2.SetSizeRepresentation(AspectRatioRelativeSize{ .ratio = 9.f/ 16.f, .value = 0.5f });
	testElement2.SetRelativePosition({ 0.0f, 0.0f });
	//testElement2.position = StaticPosition{ { 500, 300 } };

	//testElement2.SetPivot({ 0.8f, 0.7f });
	//testElement2.SetPivot({ 0.8f, 0.7f }, PivotChangePolicy::NoVisualChange);
	//testElement2.position = ConvertPivotEquivalentPosition(testElement.m_pivot, testElement2.m_pivot, testElement.position, testElement2.GetRelativeSizeToParent(), frame.size);
	//testElement2.position = ConvertPivotEquivalentPosition(testElement2.m_pivot, testElement.m_pivot, testElement2.position, testElement2.GetRelativeSizeToParent(), frame.size);
	//testElement2.pivot = testElement.pivot;
	std::chrono::duration<float> accumulator{ 0 };

	UIElement mouseDebug;
	{
		mouseDebug.texture = engine.renderer.backend->CreateTexture(SDL_PIXELFORMAT_RGBA32, static_cast<SDL2pp::TextureAccess>(SDL_TEXTUREACCESS_STATIC | SDL_TEXTUREACCESS_TARGET), 8, 8);
	};
	engine.renderer.backend->SetRenderTarget(mouseDebug.texture.get());
	engine.renderer.backend->SetDrawColor(SDL2pp::Color{ 255, 0, 0, 255 });

	engine.renderer.backend->Clear();

	engine.renderer.backend->SetRenderTarget(nullptr);
	mouseDebug.ownerFrame = &frame;
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

				engine.renderer.backend->CopyEx(testElement.texture.get(), std::nullopt, GetRect(frame, testElement), 0, SDL2pp::FPoint{ 0, 0 }, SDL2pp::RendererFlip::SDL_FLIP_NONE);
				engine.renderer.backend->CopyEx(testElement2.texture.get(), std::nullopt, GetRect(frame, testElement2), 0, SDL2pp::FPoint{ 0, 0 }, SDL2pp::RendererFlip::SDL_FLIP_NONE);
				engine.renderer.backend->CopyEx(mouseDebug.texture.get(), std::nullopt, GetRect(frame, mouseDebug), 0, SDL2pp::FPoint{ 0, 0 }, SDL2pp::RendererFlip::SDL_FLIP_NONE);

				hoveredElement = nullptr;
				if(IsOverlapping(mouseDebug.GetRelativePosition(), testElement, frame))
				{
					hoveredElement = &testElement;
					//std::cout << "Overlapping first element\n";
				}
				if(IsOverlapping(mouseDebug.GetRelativePosition(), testElement2, frame))
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
