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
	struct PinStaticSize
	{
		Vector2 pin;
		Vector2 size;
	};

	struct PinRelativeSize
	{
		Vector2 pin;
		Vector2 size;
	};

	using PositionSizeVariant = std::variant<PinStaticSize, PinRelativeSize>;

	PositionSizeVariant positionSize;
	Vector2 pivot;
	SDL2pp::shared_ptr<SDL2pp::Texture> texture;

};

SDL2pp::FRect GetRect(const UIFrame& frame, const UIElement& element)
{
	SDL2pp::FRect rect;
	const Vector2 size = [&frame, &element]() -> Vector2
	{
		if(auto* pinStaticSize = std::get_if<UIElement::PinStaticSize>(&element.positionSize); pinStaticSize)
		{
			return pinStaticSize->size;
		}
		else if(auto* pinRelativeSize = std::get_if<UIElement::PinRelativeSize>(&element.positionSize); pinRelativeSize)
		{
			return { frame.size.X() * pinRelativeSize->size.X(), frame.size.Y() * pinRelativeSize->size.Y() };
		}
		else
		{
			throw std::runtime_error("Unhandled case\n");
		}
	}();
	const Vector2 pivotPositionOffset = { element.pivot.X() * -size.X(), (1 - element.pivot.Y()) * -size.Y() };
	const Vector2 sdl2YFlip = { 0, frame.size.Y() };
	const Vector2 position = [&frame, &element] () -> Vector2
	{
			if(auto* pinStaticSize = std::get_if<UIElement::PinStaticSize>(&element.positionSize); pinStaticSize)
			{
				return { frame.size.X() * pinStaticSize->pin.X(), frame.size.Y() * -pinStaticSize->pin.Y() };
			}
			else if(auto* pinRelativeSize = std::get_if<UIElement::PinRelativeSize>(&element.positionSize); pinRelativeSize)
			{
				return { frame.size.X() * pinRelativeSize->pin.X(), frame.size.Y() * -pinRelativeSize->pin.Y() };
			}
			else
			{
				throw std::runtime_error("Unhandled case\n");
			}
	}() + pivotPositionOffset + sdl2YFlip;

	rect.x = position.X();
	rect.y = position.Y();
	rect.w = size.X();
	rect.h = size.Y();
	return rect;
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

	UIFrame frame{ { 1600, 900 } };
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
		UIElement::PinRelativeSize{ .pin ={0.2f, 0.2f}, .size = { 0.2f, 0.2f } },
		{ 1.f, 1.f},
		engine.renderer.backend->CreateTexture(testSurface)
	};
	//testElement.positionSize = ;
	//testElement.texture = std::move(engine.renderer.backend->CreateTexture(testSurface));

	UIElement testElement2 = testElement;
	testElement2.positionSize = UIElement::PinRelativeSize{ .pin = {0.5f, 0.5f}, .size = { 2.f, 2.f } };
	testElement2.pivot = { 0.5f, 0.5f };
	//testElement.size = testElement.texture->GetSize();
	//testElement.pivot.Y() = 1;
	//testElement.position.Y() = testElement.texture->GetSize().Y();
	std::chrono::duration<float> accumulator{ 0 };
	while (true)
	{
		SDL2pp::Event event;
		if (SDL2pp::PollEvent(event))
		{
			if (event.type == SDL2pp::EventType::SDL_QUIT)
			{
				break;
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
				std::cout << testElement.pivot.Y() << "\n";
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
				//auto rect = testElement.UIRect();
				//rect.x += engine.renderer.backend->GetOutputSize().X() * testElement.pin.X();
				//rect.y -= engine.renderer.backend->GetOutputSize().Y() * testElement.pin.Y();
				//rect.y += engine.renderer.backend->GetOutputSize().Y();

				auto rect = GetRect(frame, testElement);
				engine.renderer.backend->CopyEx(testElement.texture.get(), std::nullopt, rect, 0, SDL2pp::FPoint{ 0, 0 }, SDL2pp::RendererFlip::SDL_FLIP_NONE);
				engine.renderer.backend->CopyEx(testElement.texture.get(), std::nullopt, GetRect(frame, testElement2), 0, SDL2pp::FPoint{ 0, 0 }, SDL2pp::RendererFlip::SDL_FLIP_NONE);

				engine.renderer.backend->SetRenderTarget(nullptr);
				engine.renderer.backend->Copy(frame.internalTexture.get(), std::nullopt, std::optional<SDL2pp::Rect>(std::nullopt));
			}
			engine.renderer.backend->Present();
		}
	}

	return 0;
}
