#include <SDL2/SDL.h>
#include <iostream>
#include <chrono>
#include <box2d/box2d.h>
#include <span>
#include <SDL2/SDL_mixer.h>
#include <Windows.h>

import DeluEngine;
import xk.Math.Matrix;
import DeluGame;
import SDL2pp;

#undef main;

using namespace xk::Math::Aliases;

void SetAnchors(DeluEngine::GUI::UIElement& element, Vector2 minAnchor, Vector2 maxAnchor)
{
	const Vector2 size = maxAnchor - minAnchor;
	element.SetLocalSizeAndRepresentation(DeluEngine::GUI::RelativeSize{ size });
	element.SetLocalPositionAndRepresentation(DeluEngine::GUI::RelativePosition{ minAnchor + Vector2{ size.X() * element.GetPivot().X(), size.Y() * element.GetPivot().Y() } });
}

SDL2pp::FRect GetRect(const DeluEngine::GUI::UIElement& element)
{
	DeluEngine::GUI::Rect baseRect = element.GetRect();
	SDL2pp::FRect rect;
	const Vector2 size = element.GetFrameSizeAs<DeluEngine::GUI::AbsoluteSize>().value;
	const Vector2 pivotOffsetFlip = { 0, size.Y() };
	const Vector2 sdl2YFlip = { 0, element.GetRendererSize().value.Y()  };
	const Vector2 position = xk::Math::HadamardProduct(baseRect.bottomLeft.value + pivotOffsetFlip, Vector2{ 1, -1 }) + sdl2YFlip;

	rect.x = position.X();
	rect.y = position.Y();
	rect.w = size.X();
	rect.h = size.Y();
	return rect;
}

void DrawElement(DeluEngine::Renderer& renderer, DeluEngine::GUI::UIElement& element)
{
	element.HandleEvent(DeluEngine::GUI::DrawEvent{ &renderer });
	//renderer.backend->CopyEx(element.texture.get(), std::nullopt, GetRect(element), 0, SDL2pp::FPoint{ 0, 0 }, SDL2pp::RendererFlip::SDL_FLIP_NONE);

	std::vector<DeluEngine::GUI::UIElement*> children = element.GetChildren();
	for(DeluEngine::GUI::UIElement* child : children)
	{
		DrawElement(renderer, *child);
	}
}

void DrawGUI(DeluEngine::Renderer& renderer, DeluEngine::GUI::GUIEngine& frame)
{
	renderer.backend->SetRenderTarget(frame.internalTexture.get());
	renderer.backend->SetDrawColor(SDL2pp::Color{ { 0, 0, 0, 0 } });
	renderer.backend->Clear();

	std::vector<DeluEngine::GUI::UIElement*> rootElements = frame.rootElements;
	for(DeluEngine::GUI::UIElement* element : rootElements)
	{
		DrawElement(renderer, *element);
	}

	renderer.backend->SetRenderTarget(nullptr);
	renderer.backend->Copy(frame.internalTexture.get(), std::nullopt, std::optional<SDL2pp::Rect>(std::nullopt));
}

void Render(DeluEngine::Engine& engine);
#undef CreateWindow

#ifdef _CONSOLE
int main()
#else
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	PSTR lpCmdLine, int nCmdShow)
#endif
{
	DeluEngine::Engine engine
	{
		.window{ SDL2pp::CreateWindow("Bullet Hell", { 1600, 900 }, SDL2pp::WindowFlag::OpenGL) },
		.renderer{ engine.window.get() },
	};

	DeluEngine::gHeart.RegisterGroup("Game", 0);
	engine.sceneManager.commonScenePreload = [](ECS::Scene& scene)
		{
			scene.CreateSystem<DeluEngine::SceneGUISystem>();
		};
	SDL_Init(SDL_INIT_AUDIO);

	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
	//engine.box2DCallbacks.engine = &engine;
	//engine.physicsWorld.SetContactListener(&engine.box2DCallbacks);
	//engine.physicsWorld.SetDebugDraw(&engine.box2DCallbacks);
	DeluEngine::Input::defaultController = &engine.controller;

	engine.guiEngine.internalTexture = engine.renderer.backend->CreateTexture(
		SDL_PIXELFORMAT_RGBA32,
		SDL2pp::TextureAccess(SDL_TEXTUREACCESS_STATIC | SDL_TEXTUREACCESS_TARGET),
		1600, 900);
	engine.guiEngine.internalTexture->SetBlendMode(SDL_BLENDMODE_BLEND);

	engine.sceneManager.LoadScene(GameMain(engine));
	engine.renderer.debugCallbacks.push_back([&engine](DeluEngine::DebugRenderer& renderer)
		{
			//engine.scene->DebugDraw(renderer);
			//engine.box2DCallbacks.SetFlags(b2Draw::e_shapeBit);
			//engine.physicsWorld.DebugDraw();
		});

	std::chrono::duration<float> physicsAccumulator{ 0.f };
	while(engine.running)
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
			if(engine.queuedScene)
			{
				engine.sceneManager.LoadScene(engine.queuedScene);
				engine.queuedScene = nullptr;
			}
			else
			{
				engine.controllerContext.Execute(engine.controller);
				engine.guiEngine.UpdateHoveredElement();
				engine.guiEngine.DispatchHoveredEvent();

				//timer.Tick([&](std::chrono::nanoseconds dt)
				//	{

				//		std::chrono::duration<float> deltaTime = std::chrono::duration_cast<std::chrono::duration<float>>(dt);
				//		static constexpr std::chrono::duration<float> physicsStep{ 1.f / 60.f };
				//		physicsAccumulator += deltaTime;

				//		while(physicsAccumulator >= physicsStep)
				//		{
				//			//engine.physicsWorld.Step(physicsStep.count(), 8, 3);
				//			physicsAccumulator -= physicsStep;

				//			//std::cout << "Body pos " << b->GetPosition().x << ", " << b->GetPosition().y << "\n";
				//		}
				// 
				//	});

				DeluEngine::gHeart.Pulse();
				engine.controller.SwapBuffers();

				Render(engine);
			}
			//	////Formerly drawn within a frame
			//	//SDL_Rect textLocation = { 400, 200, testFontSurface->w, testFontSurface->h };
			//	//engine.renderer.backend->Copy(testFontTexture, std::nullopt, textLocation);
		}
	}

	return 0;
}

void DrawSprites(DeluEngine::Renderer& renderer, std::span<DeluEngine::Sprite*> sprites);

void Render(DeluEngine::Engine& engine)
{
	engine.renderer.backend->SetDrawColor(engine.renderer.clearColor);
	engine.renderer.backend->Clear();
	DrawSprites(engine.renderer, engine.renderer.GetSprites());

	DrawGUI(engine.renderer, engine.guiEngine);

	for(DeluEngine::DebugRenderer debugRenderer{ engine.renderer.GetDebugRenderer() }; auto& callback : engine.renderer.debugCallbacks)
	{ 
		callback(debugRenderer); 
	}
	engine.renderer.backend->Present();
}

void DrawSprites(DeluEngine::Renderer& renderer, std::span<DeluEngine::Sprite*> sprites)
{
	iVector2 outputSize = renderer.backend->GetOutputSize();
	for(const DeluEngine::Sprite* sprite : sprites)
	{
		SDL2pp::Rect sourceRect = sprite->GetSpriteData()->drawRect;
		SDL2pp::FRect destRect;
		destRect.w = static_cast<float>(sourceRect.w);
		destRect.h = static_cast<float>(sourceRect.h);
		destRect.x = sprite->position.X();
		destRect.y = -sprite->position.Y() + outputSize.Y();
		renderer.backend->CopyEx(sprite->GetSpriteData()->texture.get(), sourceRect, destRect, sprite->angle._value, std::nullopt, SDL2pp::RendererFlip::SDL_FLIP_NONE);
	}
}