#include <SDL2/SDL.h>
#include <iostream>
#include <chrono>
#include <box2d/box2d.h>

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

void SetAnchors(DeluEngine::GUI::UIElement& element, Vector2 minAnchor, Vector2 maxAnchor)
{
	const Vector2 size = maxAnchor - minAnchor;
	element.SetSizeRepresentation(DeluEngine::GUI::RelativeSize{ size });
	element.SetPositionRepresentation(DeluEngine::GUI::RelativePosition{ minAnchor + Vector2{ size.X() * element.GetPivot().X(), size.Y() * element.GetPivot().Y() } });
}

SDL2pp::FRect GetRect(const DeluEngine::GUI::UIElement& element)
{
	DeluEngine::GUI::Rect baseRect = element.GetRect();
	SDL2pp::FRect rect;
	const Vector2 size = element.GetFrameSizeAs<DeluEngine::GUI::AbsoluteSize>().value;
	const Vector2 pivotOffsetFlip = { 0, size.Y() };
	const Vector2 sdl2YFlip = { 0, element.GetFrame().GetSize().Y()  };
	const Vector2 position = xk::Math::HadamardProduct(baseRect.bottomLeft.value + pivotOffsetFlip, Vector2{ 1, -1 }) + sdl2YFlip;

	rect.x = position.X();
	rect.y = position.Y();
	rect.w = size.X();
	rect.h = size.Y();
	return rect;
}

void DrawElement(DeluEngine::Renderer& renderer, DeluEngine::GUI::UIElement& element)
{
	renderer.backend->CopyEx(element.texture.get(), std::nullopt, GetRect(element), 0, SDL2pp::FPoint{ 0, 0 }, SDL2pp::RendererFlip::SDL_FLIP_NONE);

	std::vector<DeluEngine::GUI::UIElement*> children = element.GetChildren();
	for(DeluEngine::GUI::UIElement* child : children)
	{
		DrawElement(renderer, *child);
	}
}

void DrawFrame(DeluEngine::Renderer& renderer, DeluEngine::GUI::UIFrame& frame)
{
	renderer.backend->SetRenderTarget(frame.internalTexture.get());
	renderer.backend->SetDrawColor(SDL2pp::Color{ 0, 0, 0, 0 });
	renderer.backend->Clear();

	std::vector<DeluEngine::GUI::UIElement*> rootElements = frame.GetRootElements();
	for(DeluEngine::GUI::UIElement* element : rootElements)
	{
		DrawElement(renderer, *element);
	}

	renderer.backend->SetRenderTarget(nullptr);
	renderer.backend->Copy(frame.internalTexture.get(), std::nullopt, std::optional<SDL2pp::Rect>(std::nullopt));
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

	while(true)
	{
		SDL2pp::Event event;
		if(SDL2pp::PollEvent(event))
		{
			if(event.type == SDL2pp::EventType::SDL_QUIT)
			{
				break;
			}
			ProcessEvent(engine.guiEngine, event, engine.renderer.backend->GetOutputSize());
			engine.ProcessEvent(event);
		}
		else
		{
			engine.controllerContext.Execute(engine.controller);

			timer.Tick([&](std::chrono::nanoseconds dt)
				{
					engine.guiEngine.UpdateHoveredElement();
					engine.guiEngine.DispatchHoveredEvent();
					//testElement->position.Y() = testElement->size.Y() * std::sin(accumulator.count());
					//testElement->SetPivot({ testElement->GetPivot().X(), (std::sin(accumulator.count()) + 1) / 2 });
					//testElement->SetLocalPosition(DeluEngine::GUI::RelativePosition{ { testElement->GetLocalPositionAs<DeluEngine::GUI::RelativePosition>().value.X(), (std::sin(accumulator.count()) + 1) / 2 } });
					//testElement->SetLocalPosition(DeluEngine::GUI::RelativePosition{ { (std::sin(accumulator.count()) + 1) / 2, testElement->GetLocalPositionAs<DeluEngine::GUI::RelativePosition>().value.Y() } });
					//testElement3->SetFramePosition(DeluEngine::GUI::RelativePosition{ { 0.5f, (std::sin(accumulator.count()) + 1) / 2 } });
					//testElement2->SetPivot({ testElement->GetPivot().X(), (std::sin(accumulator.count()) + 1) / 2 });
					//std::cout << testElement->GetPivot().Y() << "\n";
					std::chrono::duration<float> deltaTime = std::chrono::duration_cast<std::chrono::duration<float>>(dt);
					static constexpr std::chrono::duration<float> physicsStep{ 1.f / 60.f };
					physicsAccumulator += deltaTime;

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

				////Formerly drawn within a frame
				//SDL_Rect textLocation = { 400, 200, testFontSurface->w, testFontSurface->h };
				//engine.renderer.backend->Copy(testFontTexture, std::nullopt, textLocation);
				
				DrawFrame(engine.renderer, *engine.guiEngine.frames.back());
			}

			engine.renderer.backend->Present();
		}
	}

	return 0;
}
