module;

#include <functional>
#include <vector>
#include <memory>
#include <chrono>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include <array>

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


export struct Card
{
	std::unique_ptr<DeluEngine::GUI::Button> backCardButton;
	std::unique_ptr<DeluEngine::GUI::Image> frontCard;
	std::unique_ptr<DeluEngine::GUI::Image> cardTypeIcon;

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
};

export struct CardGrid : public DeluEngine::SceneSystem
{
	Card testCard;

	std::array<SDL2pp::shared_ptr<SDL2pp::Texture>, 12> cardTextures;
	SDL2pp::shared_ptr<SDL2pp::Texture> cardBackTexture;
	SDL2pp::shared_ptr<SDL2pp::Texture> cardFrontTexture;
public:
	CardGrid(const gsl::not_null<ECS::Scene*> scene) :
		SceneSystem{ scene }
	{
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

		DeluEngine::Engine& engine = *GetScene().GetExternalSystemAs<gsl::not_null<DeluEngine::Engine*>>();
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
		CardGrid& grid = scene.CreateSystem<CardGrid>();

		DeluEngine::GUI::UIFrame& frame = gui.NewPersistentFrame();
		frame.internalTexture = engine.renderer.backend->CreateTexture(
			SDL_PIXELFORMAT_RGBA32,
			SDL2pp::TextureAccess(SDL_TEXTUREACCESS_STATIC | SDL_TEXTUREACCESS_TARGET),
			1600, 900);
		frame.internalTexture->SetBlendMode(SDL_BLENDMODE_BLEND);
		using namespace DeluEngine::GUI;
		{
			grid.testCard.backCardButton = std::move(frame.NewElement<DeluEngine::GUI::Button>(RelativePosition{ { 0.4f, 0.5f } }, AspectRatioRelativeSize{ .ratio = -1, .value = 0.15f }, Vector2{ 0.5f, 0.5f }, nullptr, grid.cardBackTexture));
			grid.testCard.backCardButton->onClicked =
				[&card = grid.testCard]
				{
					card.FlipUp();
				};
		}

		{
			grid.testCard.frontCard = std::move(frame.NewElement<DeluEngine::GUI::Image>(RelativePosition{ { 0.4f, 0.5f } }, AspectRatioRelativeSize{ .ratio = -1, .value = 0.15f }, Vector2{ 0.5f, 0.5f }, nullptr, grid.cardFrontTexture));
		}

		{
			grid.testCard.cardTypeIcon = std::move(frame.NewElement<DeluEngine::GUI::Image>(RelativePosition{ { 0.5f, 0.5f} }, BorderConstantRelativeSize{ .value = { 0.8f, 0.8f } }, Vector2{ 0.5f, 0.5f }, grid.testCard.frontCard.get(), grid.cardTextures[4]));

			grid.testCard.cardTypeIcon->debugName = "Card Face";
		}

		grid.testCard.FlipDown();
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