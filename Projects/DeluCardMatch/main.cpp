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

struct UIFrame
{
	Vector2 size;
	SDL2pp::unique_ptr<SDL2pp::Texture> internalTexture;
};

struct UIElement
{
	//Keeps the size of the element between all screen sizes
	struct StaticSize
	{
		Vector2 value;
	};

	//Keeps the percentage of the screen size constant between screen sizes
	struct RelativeSize
	{
		Vector2 value;
	};

	//Keeps the aspect ratio of the element constant between screen sizes
	struct AspectRatioRelativeSize
	{
		float ratio;
		float value;
	};

	//Keeps the edge of the element to the edge of the parent relatively constant between screen sizes
	struct BorderConstantRelativeSize
	{
		Vector2 value;
	};

	struct Pin
	{
		Vector2 value;
	};

	using PositionVariant = Pin;
	using SizeVariant = std::variant<StaticSize, RelativeSize, AspectRatioRelativeSize, BorderConstantRelativeSize>;

	PositionVariant position;
	SizeVariant size;
	Vector2 pivot;
	SDL2pp::shared_ptr<SDL2pp::Texture> texture;

};

Vector2 StaticSizeToRelativeSize(Vector2 requestedSize, Vector2 parentSize)
{
	return { requestedSize.X() / parentSize.X(), requestedSize.Y() / parentSize.Y() };
}

Vector2 RelativeSizeToStaticSize(Vector2 requestedSize, Vector2 parentSize)
{
	return { requestedSize.X() * parentSize.X(), requestedSize.Y() * parentSize.Y() };
}

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

	return { fromPosition.value + offset };
}

void SetAnchors(UIElement& element, Vector2 minAnchor, Vector2 maxAnchor)
{
	const Vector2 size = maxAnchor - minAnchor;
	element.size = UIElement::RelativeSize{ size };
	element.position = UIElement::PositionVariant{ minAnchor + Vector2{ size.X() * element.pivot.X(), size.Y() * element.pivot.Y() } };
}

Vector2 GetStaticSize(const UIFrame& frame, const UIElement& element)
{
	return [&frame, &element]() -> Vector2
		{
			if(auto* size = std::get_if<UIElement::StaticSize>(&element.size); size)
			{
				return size->value;
			}
			else if(auto* size = std::get_if<UIElement::RelativeSize>(&element.size); size)
			{
				return { frame.size.X() * size->value.X(), frame.size.Y() * size->value.Y() };
			}
			else if(auto* size = std::get_if<UIElement::AspectRatioRelativeSize>(&element.size); size)
			{
				const float frameRatio = frame.size.X() / frame.size.Y();
				const float frameToElementRatio = frameRatio / std::fabs(size->ratio);
				return (size->ratio >= 0) ?
					Vector2{ frame.size.X() * size->value / frameToElementRatio, frame.size.Y() * size->value } :
					Vector2{ frame.size.X() * size->value, frame.size.Y() * size->value * frameToElementRatio };
			}
			else if(auto* size = std::get_if<UIElement::BorderConstantRelativeSize>(&element.size); size)
			{
				if(frame.size.X() >= frame.size.Y())
				{
					const float baseSize = frame.size.X() - frame.size.Y();
					const float remainingSize = frame.size.X() - baseSize;
					return { baseSize + remainingSize * size->value.X(), frame.size.Y() * size->value.Y() };
				}
				else
				{
					const float baseSize = frame.size.Y() - frame.size.X();
					const float remainingSize = frame.size.Y() - baseSize;
					return { frame.size.X() * size->value.X(), baseSize + remainingSize * size->value.Y() };
				}
			}
			else
			{
				throw std::runtime_error("Unhandled case\n");
			}
		}();
}

SDL2pp::FRect GetRect(const UIFrame& frame, const UIElement& element)
{
	SDL2pp::FRect rect;
	const Vector2 size = GetStaticSize(frame, element);
	const Vector2 pivotPositionOffset = { element.pivot.X() * -size.X(), (1 - element.pivot.Y()) * -size.Y() };
	const Vector2 sdl2YFlip = { 0, frame.size.Y() };
	const Vector2 position = [&frame, &element] () -> Vector2
	{
		return { frame.size.X() * element.position.value.X(), frame.size.Y() * -element.position.value.Y() };

	}() + pivotPositionOffset + sdl2YFlip;

	rect.x = position.X();
	rect.y = position.Y();
	rect.w = size.X();
	rect.h = size.Y();
	return rect;
}

bool IsOverlapping(Vector2 mousePos, const UIElement& element, const UIFrame& frame)
{
	auto size = StaticSizeToRelativeSize(GetStaticSize(frame, element), frame.size);
	auto minPos = CalculateEquivalentPositionBasedOnPivot(element.pivot, { 0, 0 }, element.position, element.size, frame.size);
	return mousePos.X() >= minPos.value.X() && mousePos.X() <= minPos.value.X() + size.X() &&
		mousePos.Y() >= minPos.value.Y() && mousePos.Y() <= minPos.value.Y() + size.Y();
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
	std::chrono::duration<float> physicsAccumulator{0.f};

	UIFrame frame{ { 900, 900} };
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
		.position = { { 0.2f, 0.2f }},
		.size = UIElement::RelativeSize({ 0.2f, 0.2f }),
		.pivot = { 1.0f, 1.0f },
		.texture = engine.renderer.backend->CreateTexture(testSurface)
	};

	UIElement testElement2 = testElement;
	testElement2.pivot = { 0.3f, 0.8f };
	//SetAnchors(testElement2, { 0.05f, 0.05f }, { 0.95f, 0.95f });
	//testElement2.size = UIElement::BorderConstantRelativeSize({ 0.5f, 0.5f });
	testElement2.size = UIElement::AspectRatioRelativeSize{ .ratio = 9.f / 16.f, .value = 0.2f };
	testElement2.position = { { 0.6f, 0.4f } };
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
	mouseDebug.size = UIElement::StaticSize{ { 8, 8 } };
	mouseDebug.pivot = { 0.5f, 0.5f };

	while (true)
	{
		SDL2pp::Event event;
		if (SDL2pp::PollEvent(event))
		{
			if (event.type == SDL2pp::EventType::SDL_QUIT)
			{
				break;
			}
			if(event.type == SDL2pp::EventType::SDL_MOUSEMOTION)
			{
				mouseDebug.position.value = { event.motion.x, engine.renderer.backend->GetOutputSize().Y() - event.motion.y };
				std::cout << mouseDebug.position.value.X() << ", " << mouseDebug.position.value.Y() << "\n";
				mouseDebug.position.value.X() /= static_cast<float>(engine.renderer.backend->GetOutputSize().X());
				mouseDebug.position.value.Y() /= static_cast<float>(engine.renderer.backend->GetOutputSize().Y());
			}
			engine.ProcessEvent(event);
		}
		else
		{
			engine.controllerContext.Execute(engine.controller);
			timer.Tick([&](std::chrono::nanoseconds dt)
			{
				//testElement.position.Y() = testElement.size.Y() * std::sin(accumulator.count());
				testElement.pivot.Y() = (std::sin(accumulator.count()));
				//std::cout << testElement.pivot.Y() << "\n";
				std::chrono::duration<float> deltaTime = std::chrono::duration_cast<std::chrono::duration<float>>(dt);
				static constexpr std::chrono::duration<float> physicsStep{ 1.f / 60.f };
				physicsAccumulator += deltaTime;
				accumulator += deltaTime;
				while (physicsAccumulator >= physicsStep)
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
				
				if(IsOverlapping(mouseDebug.position.value, testElement, frame))
					std::cout << "Overlapping first element\n";
				if(IsOverlapping(mouseDebug.position.value, testElement2, frame))
					std::cout << "Overlapping second element\n";

				engine.renderer.backend->SetRenderTarget(nullptr);
				engine.renderer.backend->Copy(frame.internalTexture.get(), std::nullopt, std::optional<SDL2pp::Rect>(std::nullopt));
			}

			engine.renderer.backend->Present();
		}
	}

	return 0;
}
