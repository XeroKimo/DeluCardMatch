module;

#include <functional>
#include <vector>
#include <memory>
#include <chrono>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include <array>
#include <span>

export module DeluGame;
import xk.Math.Matrix;
import DeluEngine;
import SDL2pp;
//import LevelStrip;
//import DeluGame.Player;
//import DeluGame.PlayerState;
//import DeluGame.PowerUpPoint;
//import Enemy;
//import LevelStrip.Event.EnemySpawn;
//import DeluGame.ControllerContextConstants;
using namespace xk::Math::Aliases;

using namespace DeluEngine::GUI;

export struct Card
{
	std::unique_ptr<DeluEngine::GUI::Button> backCardButton;
	std::unique_ptr<DeluEngine::GUI::Image> frontCard;
	std::unique_ptr<DeluEngine::GUI::Image> cardTypeIcon;

	Card(DeluEngine::GUI::UIFrame& frame, DeluEngine::GUI::SizeVariant size, SDL2pp::shared_ptr<SDL2pp::Texture> backTexture, SDL2pp::shared_ptr<SDL2pp::Texture> frontTexture, SDL2pp::shared_ptr<SDL2pp::Texture> cardTypeTexture)
	{
		backCardButton = frame.NewElement<DeluEngine::GUI::Button>(RelativePosition{}, size, Vector2{ 0.5f, 0.5f }, nullptr, backTexture);
		frontCard = frame.NewElement<DeluEngine::GUI::Image>(RelativePosition{}, size, Vector2{ 0.5f, 0.5f }, nullptr, frontTexture);
		cardTypeIcon = frame.NewElement<DeluEngine::GUI::Image>(RelativePosition{ { 0.5f, 0.5f} }, BorderConstantRelativeSize{ .value = { 0.8f, 0.8f } }, Vector2{ 0.5f, 0.5f }, frontCard.get(), cardTypeTexture);
		FlipDown();
	}

	void FlipUp()
	{
		backCardButton->render = false;
		backCardButton->debugEnableRaytrace = false;

		frontCard->render = true;
		cardTypeIcon->render = true;
	}

	void FlipDown()
	{
		backCardButton->render = true;
		backCardButton->debugEnableRaytrace = true;

		frontCard->render = false;
		cardTypeIcon->render = false;
	}

	void SetLocalPosition(PositionVariant position)
	{
		backCardButton->SetLocalPosition(position);
		frontCard->SetLocalPosition(position);
	}

	std::function<void()>& OnClicked() { return backCardButton->onClicked; }
};

export struct CardGrid : public DeluEngine::SceneSystem
{
	std::vector<std::unique_ptr<Card>> cards;

public:
	CardGrid(const gsl::not_null<ECS::Scene*> scene, DeluEngine::GUI::UIFrame& frame, iVector2 gridSize, std::span<SDL2pp::shared_ptr<SDL2pp::Texture>> textures, SDL2pp::shared_ptr<SDL2pp::Texture> cardBack, SDL2pp::shared_ptr<SDL2pp::Texture> cardFront) :
		SceneSystem{ scene }
	{
		cards.push_back(std::make_unique<Card>(frame, AspectRatioRelativeSize{ .ratio = -1, .value = 0.15f }, cardBack, cardFront, textures[4]));
		cards.back()->SetLocalPosition(RelativePosition{ {0.5, 0.5} });
		cards.back()->OnClicked() = [thisCard = cards.back().get()]
		{
			thisCard->FlipUp();
		};

	}
};

export auto CardMatchScene()
{
	return [](ECS::Scene& s)
	{
		std::cout << "Entered card match scene\n";
		DeluEngine::Scene& scene = static_cast<DeluEngine::Scene&>(s);
		DeluEngine::Engine& engine = *s.GetExternalSystemAs<gsl::not_null<DeluEngine::Engine*>>();
		DeluEngine::SceneGUISystem& gui = scene.CreateSystem<DeluEngine::SceneGUISystem>();

		DeluEngine::GUI::UIFrame& frame = gui.NewPersistentFrame();
		frame.internalTexture = engine.renderer.backend->CreateTexture(
			SDL_PIXELFORMAT_RGBA32,
			SDL2pp::TextureAccess(SDL_TEXTUREACCESS_STATIC | SDL_TEXTUREACCESS_TARGET),
			1600, 900);
		frame.internalTexture->SetBlendMode(SDL_BLENDMODE_BLEND);

		std::array pngSurfaces
		{
			IMG_Load("Cards/delu bonk.png"),
			IMG_Load("Cards/deluHi.png"),
			IMG_Load("Cards/deluminlove.png"),
			IMG_Load("Cards/DeluNG.png"),
			IMG_Load("Cards/DeluOk.png"),
			IMG_Load("Cards/delupenlight.png"),
			IMG_Load("Cards/DeluPog.png"),
			IMG_Load("Cards/Deluthug.png"),
			IMG_Load("Cards/deluwu.png"),
			IMG_Load("Cards/FXtaya.png"),
			IMG_Load("Cards/piyotaya.png"),
			IMG_Load("Cards/syobontaya.png"),
		};

		std::array<SDL2pp::shared_ptr<SDL2pp::Texture>, pngSurfaces.size()> cardTextures;
		SDL2pp::shared_ptr<SDL2pp::Texture> cardFrontTexture;
		SDL2pp::shared_ptr<SDL2pp::Texture> cardBackTexture;
		for(size_t i = 0; i < pngSurfaces.size(); i++)
		{
			cardTextures[i] = engine.renderer.backend->CreateTexture(pngSurfaces[i]);
		}
		{
			auto surface = IMG_Load("BlankCard.png");
			cardFrontTexture = engine.renderer.backend->CreateTexture(surface);
			SDL_FreeSurface(surface);
		}
		{
			auto surface = IMG_Load("CardBack.png");
			cardBackTexture = engine.renderer.backend->CreateTexture(surface);
			SDL_FreeSurface(surface);
		}

		for(auto surface : pngSurfaces)
		{
			SDL_FreeSurface(surface);
		}

		CardGrid& grid = scene.CreateSystem<CardGrid>(frame, iVector2{ 4, 4 }, cardTextures, cardBackTexture, cardFrontTexture);
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