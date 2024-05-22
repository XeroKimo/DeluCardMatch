module;

#include <functional>
#include <vector>
#include <memory>
#include <chrono>
//#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include <array>
#include <span>
#include <cstdlib>
#include <ctime>
#include <format>
#include <SDL2/SDL_mixer.h>

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

std::array<Mix_Chunk*, 3> correctEffects;
std::array<Mix_Chunk*, 4> missEffects;
Mix_Music* bgm;
using SDL2pp::SDL2Interface;
export struct Card
{
	std::unique_ptr<DeluEngine::GUI::Button> backCardButton;
	std::unique_ptr<DeluEngine::GUI::Image> frontCard;
	std::unique_ptr<DeluEngine::GUI::Image> cardTypeIcon;

	Card(DeluEngine::GUI::GUIEngine& frame, DeluEngine::GUI::SizeVariant size, SDL2pp::shared_ptr<SDL2pp::Texture> backTexture, SDL2pp::shared_ptr<SDL2pp::Texture> frontTexture, SDL2pp::shared_ptr<SDL2pp::Texture> cardTypeTexture)
	{
		backCardButton = frame.NewElement<DeluEngine::GUI::Button>(DeluEngine::GUI::RelativePosition{}, size, xk::Math::Aliases::Vector2{ 0.5f, 0.5f }, nullptr, backTexture);
		frontCard = frame.NewElement<DeluEngine::GUI::Image>(DeluEngine::GUI::RelativePosition{}, size, xk::Math::Aliases::Vector2{ 0.5f, 0.5f }, nullptr, frontTexture);
		cardTypeIcon = frame.NewElement<DeluEngine::GUI::Image>(DeluEngine::GUI::RelativePosition{ { 0.5f, 0.5f} }, DeluEngine::GUI::BorderConstantRelativeSize{ .value = { 0.8f, 0.8f } }, xk::Math::Aliases::Vector2{ 0.5f, 0.5f }, frontCard.get(), cardTypeTexture);
		FlipDown();
	}

	void SetParent(DeluEngine::GUI::UIElement* parent)
	{
		backCardButton->SetParent(parent);
		frontCard->SetParent(parent);
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

	void SetLocalPosition(DeluEngine::GUI::PositionVariant position)
	{
		backCardButton->SetLocalPosition(position);
		frontCard->SetLocalPosition(position);
	}

	auto GetType() const { return cardTypeIcon->texture; }

	std::function<void()>& OnClicked() { return backCardButton->onClicked; }
};

export struct CardMatchSceneLoader
{
	xk::Math::Aliases::iVector2 cardCount;
	void operator()(ECS::Scene& s) const;
};

export CardMatchSceneLoader CardMatchScene(xk::Math::Aliases::iVector2 cardCount);

export struct VictoryScreen
{
	std::unique_ptr<DeluEngine::GUI::Button> retryButton;
	std::unique_ptr<DeluEngine::GUI::Button> quitButton;

	VictoryScreen(DeluEngine::Engine& engine, DeluEngine::GUI::GUIEngine& frame)
	{
		SDL_Surface* quitButtonPNG = IMG_Load("Quit_Button.png");
		SDL_Surface* retryButtonPNG = IMG_Load("PlayAgain_Button.png");
		quitButton = frame.NewElement<DeluEngine::GUI::Button>(DeluEngine::GUI::RelativePosition{ { 0.40f, 0.33f } }, DeluEngine::GUI::AbsoluteSize{ { quitButtonPNG->w, quitButtonPNG->h } }, xk::Math::Aliases::Vector2{ 0.5f, 0.0f }, nullptr, engine.renderer.backend->CreateTexture(quitButtonPNG));
		quitButton->ConvertUnderlyingSizeRepresentation<DeluEngine::GUI::AspectRatioRelativeSize>();
		retryButton = frame.NewElement<DeluEngine::GUI::Button>(DeluEngine::GUI::RelativePosition{ { 0.60f, 0.33f } }, DeluEngine::GUI::AbsoluteSize{ { retryButtonPNG->w, retryButtonPNG->h } }, xk::Math::Aliases::Vector2{ 0.5f, 0.0f }, nullptr, engine.renderer.backend->CreateTexture(retryButtonPNG));
		retryButton->ConvertUnderlyingSizeRepresentation<DeluEngine::GUI::AspectRatioRelativeSize>();

		retryButton->onClicked = [&engine]
			{
				engine.queuedScene = CardMatchScene({ 6, 6 });
			};

		quitButton->onClicked = [&engine]
			{
				engine.running = false;
			};

		SDL_FreeSurface(quitButtonPNG);
		SDL_FreeSurface(retryButtonPNG);
	}
};

export struct PauseScreen
{
	std::unique_ptr<DeluEngine::GUI::Image> blankScreen;
	std::unique_ptr<DeluEngine::GUI::Button> quitButton;
	std::unique_ptr<DeluEngine::GUI::Button> retryButton;
	std::unique_ptr<DeluEngine::GUI::Button> resumeButton;
	DeluEngine::Engine* e;
	PauseScreen(DeluEngine::Engine& engine, DeluEngine::GUI::GUIEngine& frame)
	{
		

		e = &engine;
		SDL2pp::view_ptr<SDL2pp::Renderer> rendererBackend = engine.renderer.backend.get();
		SDL2pp::unique_ptr<SDL2pp::Renderer> test;

		auto blankScreenTexture = rendererBackend->CreateTexture(
			SDL_PIXELFORMAT_RGBA32,
			SDL2pp::TextureAccess(SDL_TEXTUREACCESS_STATIC | SDL_TEXTUREACCESS_TARGET),
			64, 64);

		blankScreen = frame.NewElement<DeluEngine::GUI::Image>({}, DeluEngine::GUI::RelativeSize{ { 1.f, 1.f } }, {}, nullptr, std::move(blankScreenTexture));

		engine.controllerContext.PushContext("Pause");

		SDL_Surface* quitButtonPNG = IMG_Load("Quit_Button.png");
		SDL_Surface* retryButtonPNG = IMG_Load("PlayAgain_Button.png");
		SDL_Surface* resumeButtonPNG = IMG_Load("Resume_Button.png");

		quitButton = frame.NewElement<DeluEngine::GUI::Button>(DeluEngine::GUI::RelativePosition{ { 0.3f, 0.33f } }, DeluEngine::GUI::AbsoluteSize{ { quitButtonPNG->w, quitButtonPNG->h } }, xk::Math::Aliases::Vector2{ 0.5f, 0.0f }, nullptr, engine.renderer.backend->CreateTexture(quitButtonPNG));
		quitButton->ConvertUnderlyingSizeRepresentation<DeluEngine::GUI::AspectRatioRelativeSize>();
		retryButton = frame.NewElement<DeluEngine::GUI::Button>(DeluEngine::GUI::RelativePosition{ { 0.5f, 0.33f } }, DeluEngine::GUI::AbsoluteSize{ { retryButtonPNG->w, retryButtonPNG->h } }, xk::Math::Aliases::Vector2{ 0.5f, 0.0f }, nullptr, engine.renderer.backend->CreateTexture(retryButtonPNG));
		retryButton->ConvertUnderlyingSizeRepresentation<DeluEngine::GUI::AspectRatioRelativeSize>();
		resumeButton = frame.NewElement<DeluEngine::GUI::Button>(DeluEngine::GUI::RelativePosition{ { 0.7f, 0.33f } }, DeluEngine::GUI::AbsoluteSize{ { resumeButtonPNG->w, resumeButtonPNG->h } }, xk::Math::Aliases::Vector2{ 0.5f, 0.0f }, nullptr, engine.renderer.backend->CreateTexture(resumeButtonPNG));
		resumeButton->ConvertUnderlyingSizeRepresentation<DeluEngine::GUI::AspectRatioRelativeSize>();


		retryButton->onClicked = [&]
			{
				engine.queuedScene = CardMatchScene({ 6, 6 });
			};

		quitButton->onClicked = [&]
			{
				engine.running = false;
			};

		SDL_FreeSurface(quitButtonPNG);
		SDL_FreeSurface(retryButtonPNG);
		SDL_FreeSurface(resumeButtonPNG);

	}

	~PauseScreen()
	{
		e->controllerContext.PopContext();
	}
};

export struct CardGrid : public DeluEngine::SceneSystem
{
	std::vector<std::unique_ptr<Card>> cards;
	std::unique_ptr<DeluEngine::GUI::UIElement> gridAligningParent;
	std::unique_ptr<DeluEngine::GUI::Text> gameTimeText;
	std::unique_ptr<DeluEngine::GUI::Text> moveCountText;
	std::unique_ptr<VictoryScreen> victoryScreen;
	std::unique_ptr<PauseScreen> pauseScreen;

	std::array<Card*, 2> selectedCards{};
	DeluEngine::Engine* engine;
	float timer = 0;
	float gameTime = 0;
	int moveCount = 0;
	bool pendingClosePauseScreen = false;

public:
	CardGrid(const gsl::not_null<ECS::Scene*> scene, DeluEngine::GUI::GUIEngine& frame, xk::Math::Aliases::iVector2 gridSize, std::span<SDL2pp::shared_ptr<SDL2pp::Texture>> textures, SDL2pp::shared_ptr<SDL2pp::Texture> cardBack, SDL2pp::shared_ptr<SDL2pp::Texture> cardFront) :
		SceneSystem{ scene }
	{

		engine = &static_cast<DeluEngine::Scene&>(GetScene()).GetEngine();
		engine->controllerContext.PushContext("Game");
		engine->controllerContext.GetCurrentContext().FindAction("Pause").BindButton([this](bool) { OpenPauseMenu();  });

		TTF_Font* arialFont = TTF_OpenFont("arial.ttf", 20);

		moveCountText = frame.NewElement<DeluEngine::GUI::Text>(DeluEngine::GUI::RelativePosition{ {0.05f, 0.9f} }, DeluEngine::GUI::RelativeSize{ { 0.15f, 0.1f } }, { 0, 1 }, nullptr);
		gameTimeText = frame.NewElement<DeluEngine::GUI::Text>(DeluEngine::GUI::RelativePosition{ {0.05f, 0.95f} }, DeluEngine::GUI::RelativeSize{ { 0.15f, 0.1f } }, { 0, 1 }, nullptr);

		moveCountText->SetText(std::format("Moves: {}", moveCount));
		moveCountText->SetFont(arialFont);
		gameTimeText->SetText(std::format("Time: {}", timer));
		gameTimeText->SetFont(arialFont);
		auto makeCardsOnClicked = [this](Card* thisCard)
			{
				return [this, thisCard]
					{
						if(selectedCards[0] != nullptr && selectedCards[1] != nullptr)
						{
							CheckMatchingCards();
						}

						if(selectedCards[0] == nullptr)
						{
							selectedCards[0] = thisCard;
							thisCard->FlipUp();
						}
						else if(selectedCards[1] == nullptr)
						{
							selectedCards[1] = thisCard;
							thisCard->FlipUp();
							moveCount++;
							moveCountText->SetText(std::format("Moves: {}", moveCount));
							PlayCardPairAudio();
						}
					};
			};
		
		gridAligningParent = frame.NewElement<DeluEngine::GUI::UIElement>(DeluEngine::GUI::RelativePosition{ { 0.5f, 0.5f } }, DeluEngine::GUI::AspectRatioRelativeSize{ .ratio = -1, .value = 0.9f }, { 0.5f, 0.5f }, nullptr);
		{
			size_t totalCards = gridSize.X() * gridSize.Y();
			for(size_t i = 0, cardTextureCounter = 0; i < totalCards; i += 2, cardTextureCounter++)
			{
				cards.push_back(std::make_unique<Card>(frame, DeluEngine::GUI::RelativeSize{ { 1.f / gridSize.X(), 1.f / gridSize.Y() } }, cardBack, cardFront, textures[cardTextureCounter % textures.size()]));
				cards.back()->SetParent(gridAligningParent.get());
				cards.back()->OnClicked() = makeCardsOnClicked(cards.back().get());
				cards.push_back(std::make_unique<Card>(frame, DeluEngine::GUI::RelativeSize{ { 1.f / gridSize.X(), 1.f / gridSize.Y() } }, cardBack, cardFront, textures[cardTextureCounter % textures.size()]));
				cards.back()->SetParent(gridAligningParent.get());
				cards.back()->OnClicked() = makeCardsOnClicked(cards.back().get());
			}
		}

		for(auto& card : cards)
		{
			std::swap(card, cards[rand() % cards.size()]);
		}

		for(size_t y = 0; y < gridSize.Y(); y++)
		{
			for(size_t x = 0; x < gridSize.X(); x++)
			{
				auto cardSize = cards[y * gridSize.X() + x]->frontCard->GetLocalSizeAs<DeluEngine::GUI::RelativeSize>();
				cards[y * gridSize.X() + x]->SetLocalPosition(DeluEngine::GUI::ConvertPivotEquivalentRelativePosition({ 0, 0 }, { 0.5f, 0.5f }, DeluEngine::GUI::RelativePosition{ { static_cast<float>(x) / gridSize.X(), static_cast<float>(y) / gridSize.Y() } }, cardSize, gridAligningParent->GetFrameSizeAs<DeluEngine::GUI::AbsoluteSize>()));
			}
		}
	}

	~CardGrid()
	{
		DeluEngine::Engine& engine = static_cast<DeluEngine::Scene&>(GetScene()).GetEngine();
		engine.controllerContext.PopContext();
	}

	void Update(float deltaTime) override
	{
		if(pendingClosePauseScreen)
		{
			ClosePauseMenu();
			pendingClosePauseScreen = false;
		}
		if(pauseScreen)
			return;

		if(!victoryScreen)
		{
			gameTime += deltaTime;
			gameTimeText->SetText(std::format("Time: {}", gameTime));
		}

		if(selectedCards[0] != nullptr && selectedCards[1] != nullptr)
		{
			timer += deltaTime;
		}

		if(timer >= 1)
		{
			CheckMatchingCards();
		}

		if(!victoryScreen && cards.size() == 0)
		{
			victoryScreen = std::make_unique<VictoryScreen>(*engine, engine->guiEngine);
			gameTimeText->SetFramePosition(DeluEngine::GUI::RelativePosition{ {0.5f, 0.8f} });
			gameTimeText->SetPivot({0.5f, 0.5f});

			moveCountText->SetFramePosition(DeluEngine::GUI::RelativePosition{ {0.5f, 0.75f} });
			moveCountText->SetPivot({0.5f, 0.5f});
		}
	}

	void PlayCardPairAudio()
	{
		if(selectedCards[0]->GetType() == selectedCards[1]->GetType())
			Mix_PlayChannel(-1, correctEffects[std::rand() % correctEffects.size()], 0);
		else
			Mix_PlayChannel(-1, missEffects[std::rand() % missEffects.size()], 0);
	}

	void CheckMatchingCards()
	{
		if(selectedCards[0]->GetType() == selectedCards[1]->GetType())
		{
			std::erase_if(cards, [selectedCard = selectedCards[0]](auto& card) { return card.get() == selectedCard; });
			std::erase_if(cards, [selectedCard = selectedCards[1]](auto& card) { return card.get() == selectedCard; });
		}
		else
		{
			selectedCards[0]->FlipDown();
			selectedCards[1]->FlipDown();
		}
		selectedCards[0] = selectedCards[1] = nullptr;
		timer = 0;
	}

	void OpenPauseMenu()
	{
		pauseScreen = std::make_unique<PauseScreen>(*engine, engine->guiEngine);

		DeluEngine::Engine& engine = *GetScene().GetExternalSystemAs<gsl::not_null<DeluEngine::Engine*>>();
		engine.controllerContext.GetCurrentContext().FindAction("Resume").BindButton([this](bool) { ClosePauseMenu();  });

		pauseScreen->resumeButton->onClicked = [this] { pendingClosePauseScreen = true; };
	}

	void ClosePauseMenu()
	{
		pauseScreen = nullptr;
		DeluEngine::Engine& engine = *GetScene().GetExternalSystemAs<gsl::not_null<DeluEngine::Engine*>>();
		engine.guiEngine.ResetHoveredElements();
	}
};


void CardMatchSceneLoader::operator()(ECS::Scene& s) const
{
	std::cout << "Entered card match scene\n";
	DeluEngine::Scene& scene = static_cast<DeluEngine::Scene&>(s);
	DeluEngine::Engine& engine = *s.GetExternalSystemAs<gsl::not_null<DeluEngine::Engine*>>();
	DeluEngine::SceneGUISystem& gui = scene.CreateSystem<DeluEngine::SceneGUISystem>();

	//DeluEngine::GUI::UIFrame& frame = gui.NewPersistentFrame();
	//frame.internalTexture = engine.renderer.backend->CreateTexture(
	//	SDL_PIXELFORMAT_RGBA32,
	//	SDL2pp::TextureAccess(SDL_TEXTUREACCESS_STATIC | SDL_TEXTUREACCESS_TARGET),
	//	1600, 900);
	//frame.internalTexture->SetBlendMode(SDL_BLENDMODE_BLEND);

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

	for(auto& card : cardTextures)
	{
		card.swap(cardTextures[rand() % cardTextures.size()]);
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

	CardGrid& grid = scene.CreateSystem<CardGrid>(engine.guiEngine, cardCount, cardTextures, cardBackTexture, cardFrontTexture);
}

export CardMatchSceneLoader CardMatchScene(xk::Math::Aliases::iVector2 cardCount)
{
	return CardMatchSceneLoader(cardCount);
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
		//DeluEngine::GUI::UIFrame& frame = gui.NewPersistentFrame();
		//frame.internalTexture = engine.renderer.backend->CreateTexture(
		//	SDL_PIXELFORMAT_RGBA32,
		//	SDL2pp::TextureAccess(SDL_TEXTUREACCESS_STATIC | SDL_TEXTUREACCESS_TARGET),
		//	1600, 900);
		//frame.internalTexture->SetBlendMode(SDL_BLENDMODE_BLEND);
		DeluEngine::GUI::GUIEngine& frame = engine.guiEngine;
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
				engine.queuedScene = CardMatchScene({5, 5});
			};

		SDL_FreeSurface(quitButtonPNG);
		SDL_FreeSurface(playButtonPNG);
	};

}

export std::function<void(ECS::Scene&)> GameMain(DeluEngine::Engine& engine)
{
	std::srand(std::time(nullptr));

	{
		DeluEngine::Experimental::ControllerContext gameContext;
		DeluEngine::Experimental::ControllerAction pause;
		pause.name = "Pause";
		pause.invocationState = DeluEngine::KeyState::Pressed;
		pause.action = DeluEngine::Experimental::Button{ .inputs{{ DeluEngine::Key::Escape }} };
		gameContext.actions.push_back(pause);

		engine.controllerContext.RegisterContext("Game", gameContext);
	}
	{
		DeluEngine::Experimental::ControllerContext pauseContext;
		DeluEngine::Experimental::ControllerAction resume;
		resume.name = "Resume";
		resume.invocationState = DeluEngine::KeyState::Pressed;
		resume.action = DeluEngine::Experimental::Button{ .inputs{ {DeluEngine::Key::Escape }} };
		pauseContext.actions.push_back(resume);

		engine.controllerContext.RegisterContext("Pause", pauseContext);
	}
	
	correctEffects[0] = Mix_LoadWAV("delu_wao.wav");
	correctEffects[1] = Mix_LoadWAV("delu_doya.wav");
	correctEffects[2] = Mix_LoadWAV("delu_nice.wav");
	missEffects[0] = Mix_LoadWAV("delu_a.wav");
	missEffects[1] = Mix_LoadWAV("delu_EEEE.wav");
	missEffects[2] = Mix_LoadWAV("delu_UWEEEE.wav");
	missEffects[3] = Mix_LoadWAV("delu_oya.wav");
	bgm = Mix_LoadMUS("You_and_Me_2.wav");
	Mix_PlayMusic(bgm, -1);
	Mix_VolumeChunk(correctEffects[0], MIX_MAX_VOLUME / 3);
	Mix_VolumeChunk(correctEffects[1], MIX_MAX_VOLUME / 3);
	Mix_VolumeChunk(correctEffects[2], MIX_MAX_VOLUME / 3);
	Mix_VolumeChunk(missEffects[0], MIX_MAX_VOLUME / 3);
	Mix_VolumeChunk(missEffects[1], MIX_MAX_VOLUME / 3);
	Mix_VolumeChunk(missEffects[3], MIX_MAX_VOLUME / 3);
	Mix_VolumeMusic(MIX_MAX_VOLUME / 2);
	return TitleScene();
}
