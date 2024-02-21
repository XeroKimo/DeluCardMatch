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

struct UIElement
{
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
				Vector2{ parentSize.X() * s.value / parentToElementRatio, parentSize.Y() * s.value } :
				Vector2{ parentSize.X() * s.value, parentSize.Y() * s.value * parentToElementRatio };
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

	UIFrame* ownerFrame;
	PositionVariant position;
	SizeVariant size;
	Vector2 pivot;
	SDL2pp::shared_ptr<SDL2pp::Texture> texture;

	Vector2 GetStaticSize() const noexcept
	{
		return std::visit([this](const auto& val) -> Vector2
			{
				using Ty = std::remove_cvref_t<decltype(val)>;

				static_assert(HasStaticSize<Ty>, "One of position representation does not contain a static position operation");

				return std::invoke(&Ty::GetStaticSize, val, GetParentStaticSize());
			}, size);
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
			}, position);
	}

	Vector2 GetRelativePosition() const noexcept
	{
		return StaticPositionToRelativePosition(GetStaticPosition(), GetParentStaticSize());
	}

	void SetStaticPosition(Vector2 newPos) noexcept
	{
		if(auto* internalPos = std::get_if<StaticPosition>(&position); internalPos)
		{
			internalPos->value = newPos;
		}
		else if(auto* internalPos = std::get_if<RelativePosition>(&position); internalPos)
		{
			internalPos->value = StaticPositionToRelativePosition(newPos, GetParentStaticSize());
		}
	}

	void SetRelativePosition(Vector2 newPos) noexcept
	{
		if(auto* internalPos = std::get_if<StaticPosition>(&position); internalPos)
		{
			internalPos->value = RelativePositionToStaticPosition(newPos, GetParentStaticSize());;
		}
		else if(auto* internalPos = std::get_if<RelativePosition>(&position); internalPos)
		{
			internalPos->value = newPos;
		}
	}

	Vector2 GetParentStaticSize() const noexcept
	{
		return ownerFrame->size;
	}
};

UIElement::PositionVariant CalculateEquivalentPositionBasedOnPivot(Vector2 fromPivot, Vector2 toPivot, UIElement::PositionVariant fromPosition, UIElement::SizeVariant elementSize, Vector2 parentSize)
{
	const Vector2 pivotDiff = toPivot - fromPivot;
	const Vector2 offset = [pivotDiff, &elementSize, &parentSize]() -> Vector2
		{
			if(auto* size = std::get_if<UIElement::StaticSize>(&elementSize); size)
			{
				const Vector2 relativeSize = StaticSizeToRelativeSize(size->value, parentSize);
				return Vector2{ pivotDiff.X() * relativeSize.X(), pivotDiff.Y() * relativeSize.Y() };
			}
			else if(auto* size = std::get_if<UIElement::RelativeSize>(&elementSize); size)
			{
				return Vector2{ pivotDiff.X() * size->value.X(), pivotDiff.Y() * size->value.Y() };
			}
			else if(auto* size = std::get_if<UIElement::AspectRatioRelativeSize>(&elementSize); size)
			{
				const float frameRatio = parentSize.X() / parentSize.Y();
				const float frameToElementRatio = frameRatio / std::fabs(size->ratio);

				return (size->ratio >= 0) ?
					Vector2{ pivotDiff.X() * size->value / frameToElementRatio, pivotDiff.Y() * size->value } :
					Vector2{ pivotDiff.X() * size->value, pivotDiff.Y() * size->value * frameToElementRatio };
			}
			else if(auto* size = std::get_if<UIElement::BorderConstantRelativeSize>(&elementSize); size)
			{
				if(parentSize.X() >= parentSize.Y())
				{
					const float baseSize = parentSize.X() - parentSize.Y();
					const float remainingSize = parentSize.X() - baseSize;
					const Vector2 relativeSize = StaticSizeToRelativeSize({ baseSize + remainingSize * size->value.X(), parentSize.Y() * size->value.Y() }, parentSize);
					return Vector2{ pivotDiff.X() * relativeSize.X(), pivotDiff.Y() * relativeSize.Y() };
				}
				else
				{
					const float baseSize = parentSize.Y() - parentSize.X();
					const float remainingSize = parentSize.Y() - baseSize;
					const Vector2 relativeSize = StaticSizeToRelativeSize({ parentSize.X() * size->value.X(), baseSize + remainingSize * size->value.Y() }, parentSize);
					return Vector2{ pivotDiff.X() * relativeSize.X(), pivotDiff.Y() * relativeSize.Y() };
				}
			}
			else
			{
				throw std::runtime_error("Unhandled case\n");
			}
		}();

		if(auto* position = std::get_if<UIElement::RelativePosition>(&fromPosition); position)
			return UIElement::RelativePosition{ position->value + offset };
		else if(auto* position = std::get_if<UIElement::StaticPosition>(&fromPosition); position)
			return UIElement::StaticPosition{ position->value + RelativeSizeToStaticSize(offset, parentSize) };
		else
			throw std::exception();// should be unreachable
}

void SetAnchors(UIElement& element, Vector2 minAnchor, Vector2 maxAnchor)
{
	const Vector2 size = maxAnchor - minAnchor;
	element.size = UIElement::RelativeSize{ size };
	element.position = UIElement::RelativePosition{ minAnchor + Vector2{ size.X() * element.pivot.X(), size.Y() * element.pivot.Y() } };
}

SDL2pp::FRect GetRect(const UIFrame& frame, const UIElement& element)
{
	SDL2pp::FRect rect;
	const Vector2 size = element.GetStaticSize();
	const Vector2 pivotPositionOffset = { element.pivot.X() * -size.X(), (1 - element.pivot.Y()) * -size.Y() };
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
	auto minPos = CalculateEquivalentPositionBasedOnPivot(element.pivot, { 0, 0 }, element.position, element.size, frame.size);
	auto minRelativePos = [minPos, &element]
		{
			if(auto* posRep = std::get_if<UIElement::StaticPosition>(&minPos); posRep)
			{
				return StaticPositionToRelativePosition(posRep->value, element.GetParentStaticSize());
			}
			else if(auto* posRep = std::get_if<UIElement::RelativePosition>(&minPos); posRep)
			{
				return posRep->value;
			}
		}();
	//return true;
		return mousePos.X() >= minRelativePos.X() && mousePos.X() <= minRelativePos.X() + size.X() &&
			mousePos.Y() >= minRelativePos.Y() && mousePos.Y() <= minRelativePos.Y() + size.Y();
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

	UIElement testElement
	{
		.position = UIElement::RelativePosition{ { 0.5f, 0.5f } },
		.size = UIElement::RelativeSize({ 0.8f, 0.8f }),
		.pivot = { 0.5f, 0.5f },
		.texture = engine.renderer.backend->CreateTexture(testSurface)
	};
	testElement.ownerFrame = &frame;
	UIElement testElement2 = testElement;
	testElement2.pivot = { 0.5f, 0.5f };
	//SetAnchors(testElement2, { 0.05f, 0.05f }, { 0.95f, 0.95f });
	testElement2.size = UIElement::BorderConstantRelativeSize({ 0.8f, 0.8f });
	//testElement2.size = UIElement::AspectRatioRelativeSize{ .ratio = 9.f / 16.f, .value = 0.2f };
	testElement2.position = UIElement::RelativePosition{ { 0.5f, 0.5f } };
	//testElement2.position = UIElement::StaticPosition{ { 500, 300 } };
	//testElement2.position = CalculateEquivalentPositionBasedOnPivot(testElement.pivot, testElement2.pivot, testElement.position, testElement2.size, frame.size);
	//testElement2.position = CalculateEquivalentPositionBasedOnPivot(testElement2.pivot, testElement.pivot, testElement2.position, testElement2.size, frame.size);
	//testElement2.pivot = testElement.pivot;
	std::chrono::duration<float> accumulator{ 0 };

	UIElement mouseDebug
	{
		.texture = engine.renderer.backend->CreateTexture(SDL_PIXELFORMAT_RGBA32, static_cast<SDL2pp::TextureAccess>(SDL_TEXTUREACCESS_STATIC | SDL_TEXTUREACCESS_TARGET), 8, 8)
	};
	engine.renderer.backend->SetRenderTarget(mouseDebug.texture.get());
	engine.renderer.backend->SetDrawColor(SDL2pp::Color{ 255, 0, 0, 255 });

	engine.renderer.backend->Clear();

	engine.renderer.backend->SetRenderTarget(nullptr);
	mouseDebug.ownerFrame = &frame;
	mouseDebug.size = UIElement::StaticSize{ { 8, 8 } };
	mouseDebug.pivot = { 0.5f, 0.5f };
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
					testElement.pivot.Y() = (std::sin(accumulator.count()) + 1) / 2;
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
