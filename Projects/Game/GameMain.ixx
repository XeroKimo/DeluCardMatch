module;

#include <functional>
#include <vector>
#include <memory>
#include <chrono>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <iostream>

export module DeluGame;

import DeluEngine;
//import LevelStrip;
//import DeluGame.Player;
//import DeluGame.PlayerState;
//import DeluGame.PowerUpPoint;
//import Enemy;
//import LevelStrip.Event.EnemySpawn;
//import DeluGame.ControllerContextConstants;

export auto CardMatchScene()
{
	return [](ECS::Scene& s)
	{
			//SDL_Surface* testSurface = IMG_Load("Cards/syobontaya.png");
		std::cout << "Entered card match scene\n";
	};
}

using namespace xk::Math::Aliases;
export auto TitleScene()
{
	return [](ECS::Scene& s)
	{
		DeluEngine::Scene& scene = static_cast<DeluEngine::Scene&>(s);
		DeluEngine::SceneGUISystem& gui = scene.CreateSystem<DeluEngine::SceneGUISystem>();

		DeluEngine::Engine& engine = static_cast<DeluEngine::Scene&>(s).GetEngine();
		//engine.guiEngine.frames.push_back({});
		DeluEngine::GUI::UIFrame& frame = gui.NewPersistentFrame();
		frame.internalTexture = engine.renderer.backend->CreateTexture(
			SDL_PIXELFORMAT_RGBA32,
			SDL2pp::TextureAccess(SDL_TEXTUREACCESS_STATIC | SDL_TEXTUREACCESS_TARGET),
			1600, 900);
		frame.internalTexture->SetBlendMode(SDL_BLENDMODE_BLEND);
		SDL_Surface* quitButtonPNG = IMG_Load("Quit_Button.png");
		SDL_Surface* playButtonPNG = IMG_Load("Play_Button.png");


		auto temp2 = frame.NewElement<DeluEngine::GUI::Button>({}, {}, {}, nullptr, nullptr);
		DeluEngine::GUI::Button* quitButton = temp2.get();
		gui.AddPersistentElement(std::move(temp2));
		quitButton->debugName = "Two";
		quitButton->texture = engine.renderer.backend->CreateTexture(quitButtonPNG);
		quitButton->SetPivot({ 0.5f, 0.0f });
		quitButton->SetLocalPosition(DeluEngine::GUI::RelativePosition{ { 0.5f, 0.2f } });
		quitButton->SetSizeRepresentation(DeluEngine::GUI::AbsoluteSize{ { quitButtonPNG->w, quitButtonPNG->h } });
		quitButton->ConvertUnderlyingSizeRepresentation<DeluEngine::GUI::AspectRatioRelativeSize>();
		quitButton->onClicked = [&engine]
			{
				engine.running = false;
			};


		temp2 = frame.NewElement<DeluEngine::GUI::Button>({}, {}, {}, nullptr);
		DeluEngine::GUI::Button* playButton = temp2.get();
		gui.AddPersistentElement(std::move(temp2));

		playButton->debugName = "Three";
		playButton->texture = engine.renderer.backend->CreateTexture(playButtonPNG);
		playButton->SetPivot({ 0.5f, 0.0f });
		playButton->SetLocalPosition(DeluEngine::GUI::RelativePosition{ { 0.5f, 0.4f } });
		playButton->SetSizeRepresentation(DeluEngine::GUI::AbsoluteSize{ { playButtonPNG->w, playButtonPNG->h } });
		playButton->ConvertUnderlyingSizeRepresentation<DeluEngine::GUI::AspectRatioRelativeSize>();
		playButton->onClicked = [&engine]
			{
				engine.queuedScene = CardMatchScene();
			};

		SDL_FreeSurface(quitButtonPNG);
		SDL_FreeSurface(playButtonPNG);
	};

}

export std::function<void(ECS::Scene&)> GameMain(DeluEngine::Engine& engine)
{
	return TitleScene();
}