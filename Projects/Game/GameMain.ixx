module;

#include <functional>
#include <vector>
#include <memory>
#include <chrono>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>

export module DeluGame;

import DeluEngine;
//import LevelStrip;
//import DeluGame.Player;
//import DeluGame.PlayerState;
//import DeluGame.PowerUpPoint;
//import Enemy;
//import LevelStrip.Event.EnemySpawn;
//import DeluGame.ControllerContextConstants;

using namespace xk::Math::Aliases;
export auto TestSceneMain()
{
	return [](ECS::Scene& s)
	{
		DeluEngine::Engine& engine = static_cast<DeluEngine::Scene&>(s).GetEngine();
		engine.guiEngine.frames.push_back({});
		DeluEngine::GUI::UIFrame& frame = engine.guiEngine.frames.back();
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

		static std::unique_ptr<DeluEngine::GUI::UIElement> testElement = frame.NewElement({}, {}, {}, nullptr);
		testElement->debugName = "One";
		testElement->SetPositionRepresentation(DeluEngine::GUI::RelativePosition{ { 0.0f, 0.5f } });
		testElement->SetSizeRepresentation(DeluEngine::GUI::RelativeSize({ 0.5f, 0.5f }));
		testElement->SetPivot({ 0.5f, 0.5f });
		testElement->texture = engine.renderer.backend->CreateTexture(testSurface);
		testElement->ConvertUnderlyingPositionRepresentation<DeluEngine::GUI::AbsolutePosition>();
		testElement->ConvertUnderlyingSizeRepresentation<DeluEngine::GUI::AbsoluteSize>();
		testElement->ConvertUnderlyingSizeRepresentation<DeluEngine::GUI::RelativeSize>();
		static std::unique_ptr<DeluEngine::GUI::UIElement> testElement2 = frame.NewElement(testElement->GetFramePositionAs<DeluEngine::GUI::RelativePosition>(), testElement->GetFrameSizeAs<DeluEngine::GUI::RelativeSize>(), testElement->GetPivot(), nullptr);
		testElement2->debugName = "Two";
		testElement2->texture = testElement->texture;
		testElement2->SetPivot({ 0.5f, 0.5f });
		//SetAnchors(testElement2, { 0.05f, 0.05f }, { 0.95f, 0.95f });
		//testElement2->SetSizeRepresentation(BorderConstantRelativeSize({ 0.8f, 0.8f }));
		testElement2->SetLocalPosition(DeluEngine::GUI::RelativePosition{ { 0.85f, 0.5f } });
		testElement2->SetSizeRepresentation(DeluEngine::GUI::AspectRatioRelativeSize{ .ratio = -9.f / 16.f, .value = 0.5f });
		testElement2->SetParent(testElement.get(), DeluEngine::GUI::UIReparentLogic::KeepAbsoluteTransform);
		//testElement2->SetLocalPosition(DeluEngine::GUI::ConvertPivotEquivalentRelativePosition(testElement2->GetPivot(), { 0, 0 }, testElement2->GetLocalPositionAs<DeluEngine::GUI::RelativePosition>(), testElement2->GetLocalSizeAs<DeluEngine::GUI::RelativeSize>(), testElement2->GetParentAbsoluteSize()));

		//testElement2->SetPivot({ 0.0f, 0.0f });
		//testElement2->SetFramePosition(DeluEngine::GUI::ConvertPivotEquivalentRelativePosition(testElement2->GetPivot(), { 0, 0 }, DeluEngine::GUI::RelativePosition{ testElement2->GetPivotedFramePositionAs<DeluEngine::GUI::RelativePosition>().value - testElement2->GetPivotOffset<DeluEngine::GUI::RelativePosition>().value }, testElement2->GetLocalSizeAs<DeluEngine::GUI::RelativeSize>(), DeluEngine::GUI::AbsoluteSize{ testElement2->GetFrame().GetSize() }));
		//testElement2->SetFramePosition(DeluEngine::GUI::ConvertPivotEquivalentRelativePosition(testElement2->GetPivot(), { 0, 0 }, DeluEngine::GUI::RelativePosition{ testElement2->GetFramePositionAs<DeluEngine::GUI::RelativePosition>().value + testElement2->GetPivotOffset<DeluEngine::GUI::RelativePosition>().value }, testElement2->GetLocalSizeAs<DeluEngine::GUI::RelativeSize>(), DeluEngine::GUI::AbsoluteSize{ testElement2->GetFrame().GetSize() }));
		//testElement2->SetFramePosition(DeluEngine::GUI::ConvertPivotEquivalentRelativePosition(testElement2->GetPivot(), { 0, 0 }, DeluEngine::GUI::RelativePosition{ testElement2->GetFramePositionAs<DeluEngine::GUI::RelativePosition>().value + testElement2->GetPivotOffset<DeluEngine::GUI::RelativePosition>().value }, testElement2->GetFrameSizeAs<DeluEngine::GUI::RelativeSize>(), testElement2->GetParentAbsoluteSize()));
		//testElement2->SetFramePosition(DeluEngine::GUI::ConvertPivotEquivalentRelativePosition(testElement2->GetPivot(), { 0, 0 }, testElement2->GetFramePositionAs<DeluEngine::GUI::RelativePosition>(), testElement2->GetFrameSizeAs<DeluEngine::GUI::RelativeSize>(), testElement2->GetParentAbsoluteSize()));

		//testElement2->SetPivot({ 0.0f, 0.0f });
		//testElement2->SetAbsolutePosition({ 950.33f, 0.0f });
		//testElement2->position = DeluEngine::GUI::AbsolutePosition{ { 500, 300 } };

		//testElement2->SetPivot({ 0.8f, 0.7f });
		//testElement2->SetPivot({ 0.8f, 0.7f }, PivotChangePolicy::NoVisualChange);
		//testElement2->position = ConvertPivotEquivalentPosition(testElement->m_pivot, testElement2->m_pivot, testElement->position, testElement2->GetRelativeSizeToParent(), frame.size);
		//testElement2->position = ConvertPivotEquivalentPosition(testElement2->m_pivot, testElement->m_pivot, testElement2->position, testElement2->GetRelativeSizeToParent(), frame.size);
		//testElement2->pivot = testElement->pivot;

		static std::unique_ptr<DeluEngine::GUI::UIElement> testElement3 = frame.NewElement(testElement->GetFramePositionAs<DeluEngine::GUI::RelativePosition>(), testElement->GetFrameSizeAs<DeluEngine::GUI::RelativeSize>(), testElement->GetPivot(), testElement2.get());

		testElement3->debugName = "Three";
		testElement3->texture = testElement->texture;
		//testElement3->SetParent(testElement2.get());
		testElement3->SetPivot({ 0.5f, 0.5f });
		testElement3->SetPositionRepresentation(DeluEngine::GUI::RelativePosition{ { 0.5f, 0.5f } });
		//testElement3->SetFrameSize(DeluEngine::GUI::RelativeSize{ {0.5f, 0.5f} });
		//testElement3->SetFramePosition(DeluEngine::GUI::RelativePosition{ { 0.5f, 0.5f } });
		testElement3->SetLocalSize(DeluEngine::GUI::RelativeSize{ {0.5f, 0.5f} });
		testElement3->SetLocalPosition(DeluEngine::GUI::RelativePosition{ { 0.5f, 0.5f } });

		//std::chrono::duration<float> accumulator{ 0 };

		//static std::unique_ptr<DeluEngine::GUI::UIElement> mouseDebug = frame.NewElement({}, {}, {}, nullptr);
		//{
		//	mouseDebug->texture = engine.renderer.backend->CreateTexture(SDL_PIXELFORMAT_RGBA32, static_cast<SDL2pp::TextureAccess>(SDL_TEXTUREACCESS_STATIC | SDL_TEXTUREACCESS_TARGET), 8, 8);
		//};
		//mouseDebug->debugEnableRaytrace = false;
		//engine.renderer.backend->SetRenderTarget(mouseDebug->texture.get());
		//engine.renderer.backend->SetDrawColor(SDL2pp::Color{ 255, 0, 0, 255 });

		//engine.renderer.backend->Clear();

		//engine.renderer.backend->SetRenderTarget(nullptr);
		//mouseDebug->SetSizeRepresentation(DeluEngine::GUI::AbsoluteSize{ { 8, 8 } });
		//mouseDebug->SetPivot({ 0.5f, 0.5f });
	};

}

export std::function<void(ECS::Scene&)> GameMain(DeluEngine::Engine& engine)
{
	return TestSceneMain();
}