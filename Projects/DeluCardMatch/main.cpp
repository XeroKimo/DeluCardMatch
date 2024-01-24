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
	Vector2 position;
	Vector2 size;
	Vector2 pivot;
	SDL2pp::unique_ptr<SDL2pp::Texture> texture;

	SDL2pp::FRect UIRect() const
	{
		SDL2pp::FRect rect;
		rect.x = pivot.X() * -size.X() + position.X();
		rect.y = (1 - pivot.Y()) * -size.Y() - position.Y();
		rect.w = size.X();
		rect.h = size.Y();
		return rect;
	}
};

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

	UIFrame frame{ { 1280, 720 } };
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
	testElement.texture = engine.renderer.backend->CreateTexture(testSurface);
	testElement.size = testElement.texture->GetSize();
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
				//testElement.pivot.Y() = (std::sin(accumulator.count()) + 1) / 2;
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
				auto rect = testElement.UIRect();
				rect.y += engine.renderer.backend->GetOutputSize().Y();
				std::cout << rect.y << "\n";
				engine.renderer.backend->CopyEx(testElement.texture.get(), std::nullopt, rect, 0, SDL2pp::FPoint{ 0, 0 }, SDL2pp::RendererFlip::SDL_FLIP_NONE);

				engine.renderer.backend->SetRenderTarget(nullptr);
				engine.renderer.backend->Copy(frame.internalTexture.get(), std::nullopt, std::optional<SDL2pp::Rect>(std::nullopt));
			}
			engine.renderer.backend->Present();
		}
	}

	return 0;
}
