#include <SDL2/SDL.h>
#include <iostream>
#include <chrono>
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

void SetAnchors(DeluEngine::GUI::UIElement& element, Vector2 minAnchor, Vector2 maxAnchor)
{
	const Vector2 size = maxAnchor - minAnchor;
	element.SetSizeRepresentation(DeluEngine::GUI::RelativeSize{ size });
	element.SetPositionRepresentation(DeluEngine::GUI::RelativePosition{ minAnchor + Vector2{ size.X() * element.GetPivot().X(), size.Y() * element.GetPivot().Y() } });
}

SDL2pp::FRect GetRect(const DeluEngine::GUI::UIElement& element)
{
	SDL2pp::FRect rect;
	const Vector2 size = element.GetFrameSizeAs<DeluEngine::GUI::AbsoluteSize>().value;
	const Vector2 pivotOffsetFlip = { 0, size.Y() };
	const Vector2 sdl2YFlip = { 0, element.GetFrame().GetSize().Y()  };
	const Vector2 position = xk::Math::HadamardProduct(element.GetPivotedFramePositionAs<DeluEngine::GUI::AbsolutePosition>().value + pivotOffsetFlip, Vector2{ 1, -1 }) + sdl2YFlip;

		rect.x = position.X();
		rect.y = position.Y();
		rect.w = size.X();
		rect.h = size.Y();
		return rect;
}

bool IsOverlapping(Vector2 mousePos, const DeluEngine::GUI::UIElement& element)
{
	auto size = element.GetFrameSizeAs<DeluEngine::GUI::RelativeSize>().value;
	auto minPos = element.GetPivotedFramePositionAs<DeluEngine::GUI::RelativePosition>().value;//DeluEngine::GUI::ConvertPivotEquivalentRelativePosition(element.GetPivot(), { 0, 0 }, element.GetFramePositionAs<DeluEngine::GUI::RelativePosition>(), element.GetFrameSizeAs<DeluEngine::GUI::RelativeSize>(), DeluEngine::GUI::AbsoluteSize{ element.GetFrame().GetSize() }).value;
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

	DeluEngine::GUI::UIFrame frame;
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

	std::unique_ptr<DeluEngine::GUI::UIElement> testElement = frame.NewElement({}, {}, {}, nullptr);
	testElement->SetPositionRepresentation(DeluEngine::GUI::RelativePosition{ { 0.0f, 0.5f } });
	testElement->SetSizeRepresentation(DeluEngine::GUI::RelativeSize({ 0.5f, 0.5f }));
	testElement->SetPivot({ 0.5f, 0.5f });
	testElement->texture = engine.renderer.backend->CreateTexture(testSurface);
	testElement->ConvertUnderlyingPositionRepresentation<DeluEngine::GUI::AbsolutePosition>();
	testElement->ConvertUnderlyingSizeRepresentation<DeluEngine::GUI::AbsoluteSize>();
	testElement->ConvertUnderlyingSizeRepresentation<DeluEngine::GUI::RelativeSize>();
	std::unique_ptr<DeluEngine::GUI::UIElement> testElement2 = frame.NewElement(testElement->GetFramePositionAs<DeluEngine::GUI::RelativePosition>(), testElement->GetFrameSizeAs<DeluEngine::GUI::RelativeSize>(), testElement->GetPivot(), nullptr);
	testElement2->texture = testElement->texture;
	testElement2->SetPivot({ 0.5f, 0.5f });
	//SetAnchors(testElement2, { 0.05f, 0.05f }, { 0.95f, 0.95f });
	//testElement2->SetSizeRepresentation(BorderConstantRelativeSize({ 0.8f, 0.8f }));
	testElement2->SetLocalPosition(DeluEngine::GUI::RelativePosition{ { 0.85f, 0.5f } });
	testElement2->SetSizeRepresentation(DeluEngine::GUI::AspectRatioRelativeSize{ .ratio = -9.f/ 16.f, .value = 0.5f });
	testElement2->SetParent(testElement.get(), DeluEngine::GUI::UIReparentLogic::KeepAbsoluteTransform);
	//testElement2->SetLocalPosition(DeluEngine::GUI::ConvertPivotEquivalentRelativePosition(testElement2->GetPivot(), { 0, 0 }, testElement2->GetLocalPositionAs<DeluEngine::GUI::RelativePosition>(), testElement2->GetLocalSizeAs<DeluEngine::GUI::RelativeSize>(), testElement2->GetParentAbsoluteSize()));

	//testElement2->SetPivot({ 0.0f, 0.0f });
	//testElement2->SetFramePosition(DeluEngine::GUI::ConvertPivotEquivalentRelativePosition(testElement2->GetPivot(), { 0, 0 }, DeluEngine::GUI::RelativePosition{ testElement2->GetPivotedFramePositionAs<DeluEngine::GUI::RelativePosition>().value - testElement2->GetPivotOffset<DeluEngine::GUI::RelativePosition>().value }, testElement2->GetLocalSizeAs<DeluEngine::GUI::RelativeSize>(), DeluEngine::GUI::AbsoluteSize{ testElement2->GetFrame().GetSize() }));
	//testElement2->SetFramePosition(DeluEngine::GUI::ConvertPivotEquivalentRelativePosition(testElement2->GetPivot(), { 0, 0 }, DeluEngine::GUI::RelativePosition{ testElement2->GetFramePositionAs<DeluEngine::GUI::RelativePosition>().value + testElement2->GetPivotOffset<DeluEngine::GUI::RelativePosition>().value }, testElement2->GetLocalSizeAs<DeluEngine::GUI::RelativeSize>(), DeluEngine::GUI::AbsoluteSize{ testElement2->GetFrame().GetSize() }));
	//testElement2->SetFramePosition(DeluEngine::GUI::ConvertPivotEquivalentRelativePosition(testElement2->GetPivot(), { 0, 0 }, DeluEngine::GUI::RelativePosition{ testElement2->GetFramePositionAs<DeluEngine::GUI::RelativePosition>().value + testElement2->GetPivotOffset<DeluEngine::GUI::RelativePosition>().value }, testElement2->GetFrameSizeAs<DeluEngine::GUI::RelativeSize>(), testElement2->GetParentAbsoluteSize()));
	testElement2->SetFramePosition(DeluEngine::GUI::ConvertPivotEquivalentRelativePosition(testElement2->GetPivot(), { 0, 0 }, testElement2->GetFramePositionAs<DeluEngine::GUI::RelativePosition>(), testElement2->GetFrameSizeAs<DeluEngine::GUI::RelativeSize>(), testElement2->GetParentAbsoluteSize()));

	testElement2->SetPivot({ 0.0f, 0.0f });
	//testElement2->SetAbsolutePosition({ 950.33f, 0.0f });
	//testElement2->position = DeluEngine::GUI::AbsolutePosition{ { 500, 300 } };

	//testElement2->SetPivot({ 0.8f, 0.7f });
	//testElement2->SetPivot({ 0.8f, 0.7f }, PivotChangePolicy::NoVisualChange);
	//testElement2->position = ConvertPivotEquivalentPosition(testElement->m_pivot, testElement2->m_pivot, testElement->position, testElement2->GetRelativeSizeToParent(), frame.size);
	//testElement2->position = ConvertPivotEquivalentPosition(testElement2->m_pivot, testElement->m_pivot, testElement2->position, testElement2->GetRelativeSizeToParent(), frame.size);
	//testElement2->pivot = testElement->pivot;

	std::unique_ptr<DeluEngine::GUI::UIElement> testElement3 = frame.NewElement(testElement->GetFramePositionAs<DeluEngine::GUI::RelativePosition>(), testElement->GetFrameSizeAs<DeluEngine::GUI::RelativeSize>(), testElement->GetPivot(), testElement2.get());
	testElement3->texture = testElement->texture; 
	//testElement3->SetParent(testElement2.get());
	testElement3->SetPivot({ 0.5f, 0.5f });
	testElement3->SetPositionRepresentation(DeluEngine::GUI::RelativePosition{ { 0.5f, 0.5f } });
	//testElement3->SetFrameSize(DeluEngine::GUI::RelativeSize{ {0.5f, 0.5f} });
	//testElement3->SetFramePosition(DeluEngine::GUI::RelativePosition{ { 0.5f, 0.5f } });
	testElement3->SetLocalSize(DeluEngine::GUI::RelativeSize{ {0.5f, 0.5f} });
	testElement3->SetLocalPosition(DeluEngine::GUI::RelativePosition{ { 0.5f, 0.5f } });

	std::chrono::duration<float> accumulator{ 0 };

	std::unique_ptr<DeluEngine::GUI::UIElement> mouseDebug = frame.NewElement({}, {}, {}, nullptr);
	{
		mouseDebug->texture = engine.renderer.backend->CreateTexture(SDL_PIXELFORMAT_RGBA32, static_cast<SDL2pp::TextureAccess>(SDL_TEXTUREACCESS_STATIC | SDL_TEXTUREACCESS_TARGET), 8, 8);
	};
	engine.renderer.backend->SetRenderTarget(mouseDebug->texture.get());
	engine.renderer.backend->SetDrawColor(SDL2pp::Color{ 255, 0, 0, 255 });

	engine.renderer.backend->Clear();

	engine.renderer.backend->SetRenderTarget(nullptr);
	mouseDebug->SetSizeRepresentation(DeluEngine::GUI::AbsoluteSize{ { 8, 8 } });
	mouseDebug->SetPivot({ 0.5f, 0.5f });
	DeluEngine::GUI::UIElement* hoveredElement = nullptr;
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
				mouseDebug->SetLocalPosition(DeluEngine::GUI::RelativePosition{ xk::Math::HadamardDivision(Vector2{ event.motion.x, engine.renderer.backend->GetOutputSize().Y() - event.motion.y }, engine.renderer.backend->GetOutputSize()) });
				//mouseDebug->position.value = ;
				//std::cout << mouseDebug->position.value.X() << ", " << mouseDebug->position.value.Y() << "\n";
				//mouseDebug->position.value.X() /= static_cast<float>(engine.renderer.backend->GetOutputSize().X());
				//mouseDebug->position.value.Y() /= static_cast<float>(engine.renderer.backend->GetOutputSize().Y());
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
					//testElement->position.Y() = testElement->size.Y() * std::sin(accumulator.count());
					//testElement->SetPivot({ testElement->GetPivot().X(), (std::sin(accumulator.count()) + 1) / 2 });
					//testElement->SetLocalPosition(DeluEngine::GUI::RelativePosition{ { testElement->GetLocalPositionAs<DeluEngine::GUI::RelativePosition>().value.X(), (std::sin(accumulator.count()) + 1) / 2 } });
					testElement->SetLocalPosition(DeluEngine::GUI::RelativePosition{ { (std::sin(accumulator.count()) + 1) / 2, testElement->GetLocalPositionAs<DeluEngine::GUI::RelativePosition>().value.Y() } });
					//testElement3->SetFramePosition(DeluEngine::GUI::RelativePosition{ { 0.5f, (std::sin(accumulator.count()) + 1) / 2 } });
					//testElement2->SetPivot({ testElement->GetPivot().X(), (std::sin(accumulator.count()) + 1) / 2 });
					//std::cout << testElement->GetPivot().Y() << "\n";
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

				engine.renderer.backend->CopyEx(testElement->texture.get(), std::nullopt, GetRect(*testElement), 0, SDL2pp::FPoint{ 0, 0 }, SDL2pp::RendererFlip::SDL_FLIP_NONE);
				engine.renderer.backend->CopyEx(testElement2->texture.get(), std::nullopt, GetRect(*testElement2), 0, SDL2pp::FPoint{ 0, 0 }, SDL2pp::RendererFlip::SDL_FLIP_NONE);
				engine.renderer.backend->CopyEx(testElement3->texture.get(), std::nullopt, GetRect(*testElement3), 0, SDL2pp::FPoint{ 0, 0 }, SDL2pp::RendererFlip::SDL_FLIP_NONE);
				engine.renderer.backend->CopyEx(mouseDebug->texture.get(), std::nullopt, GetRect(*mouseDebug), 0, SDL2pp::FPoint{ 0, 0 }, SDL2pp::RendererFlip::SDL_FLIP_NONE);

				hoveredElement = nullptr;

				if(IsOverlapping(mouseDebug->GetFramePositionAs<DeluEngine::GUI::RelativePosition>().value, *testElement))
				{
					hoveredElement = testElement.get();
					//std::cout << "Overlapping first element\n";
				}
				if(IsOverlapping(mouseDebug->GetFramePositionAs<DeluEngine::GUI::RelativePosition>().value, *testElement2))
				{
					hoveredElement = testElement2.get();
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
