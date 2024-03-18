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
#include <cstdlib>
#include <ctime>

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

	void SetParent(UIElement* parent)
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

	void SetLocalPosition(PositionVariant position)
	{
		backCardButton->SetLocalPosition(position);
		frontCard->SetLocalPosition(position);
	}

	auto GetType() const { return cardTypeIcon->texture; }

	std::function<void()>& OnClicked() { return backCardButton->onClicked; }
};

export struct CardMatchSceneLoader
{

	void operator()(ECS::Scene& s) const;
};

export CardMatchSceneLoader CardMatchScene();

export struct VictoryScreen
{
	std::unique_ptr<DeluEngine::GUI::Button> retryButton;
	std::unique_ptr<DeluEngine::GUI::Button> quitButton;

	VictoryScreen(DeluEngine::Engine& engine, DeluEngine::GUI::UIFrame& frame)
	{
		SDL_Surface* quitButtonPNG = IMG_Load("Quit_Button.png");
		SDL_Surface* retryButtonPNG = IMG_Load("PlayAgain_Button.png");

		quitButton = frame.NewElement<DeluEngine::GUI::Button>(RelativePosition{ { 0.40f, 0.33f } }, DeluEngine::GUI::AbsoluteSize{ { quitButtonPNG->w, quitButtonPNG->h } }, Vector2{ 0.5f, 0.0f }, nullptr, engine.renderer.backend->CreateTexture(quitButtonPNG));
		quitButton->ConvertUnderlyingSizeRepresentation<AspectRatioRelativeSize>();
		retryButton = frame.NewElement<DeluEngine::GUI::Button>(RelativePosition{ { 0.60f, 0.33f } }, DeluEngine::GUI::AbsoluteSize{ { retryButtonPNG->w, retryButtonPNG->h } }, Vector2{ 0.5f, 0.0f }, nullptr, engine.renderer.backend->CreateTexture(retryButtonPNG));
		retryButton->ConvertUnderlyingSizeRepresentation<AspectRatioRelativeSize>();

		retryButton->onClicked = [&engine]
			{
				engine.queuedScene = CardMatchScene();
			};

		quitButton->onClicked = [&engine]
			{
				engine.running = false;
			};

		SDL_FreeSurface(quitButtonPNG);
		SDL_FreeSurface(retryButtonPNG);
	}
};

export struct CardGrid : public DeluEngine::SceneSystem
{
	std::vector<std::unique_ptr<Card>> cards;
	std::unique_ptr<DeluEngine::GUI::UIElement> gridAligningParent;

	std::unique_ptr<VictoryScreen> victoryScreen;

	std::array<Card*, 2> selectedCards{};
	DeluEngine::GUI::UIFrame* owningFrame;
	float timer = 0;

public:
	CardGrid(const gsl::not_null<ECS::Scene*> scene, DeluEngine::GUI::UIFrame& frame, iVector2 gridSize, std::span<SDL2pp::shared_ptr<SDL2pp::Texture>> textures, SDL2pp::shared_ptr<SDL2pp::Texture> cardBack, SDL2pp::shared_ptr<SDL2pp::Texture> cardFront) :
		SceneSystem{ scene }
	{
		owningFrame = &frame;
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
						}
					};
			};
		
		gridAligningParent = frame.NewElement<UIElement>(RelativePosition{ { 0.5f, 0.5f } }, AspectRatioRelativeSize{ .ratio = -1, .value = 0.9f }, { 0.5f, 0.5f }, nullptr);
		{
			size_t totalCards = gridSize.X() * gridSize.Y();
			for(size_t i = 0, cardTextureCounter = 0; i < totalCards; i += 2, cardTextureCounter++)
			{
				cards.push_back(std::make_unique<Card>(frame, RelativeSize{ { 1.f / gridSize.X(), 1.f / gridSize.Y() } }, cardBack, cardFront, textures[cardTextureCounter % textures.size()]));
				cards.back()->SetParent(gridAligningParent.get());
				cards.back()->OnClicked() = makeCardsOnClicked(cards.back().get());
				cards.push_back(std::make_unique<Card>(frame, RelativeSize{ { 1.f / gridSize.X(), 1.f / gridSize.Y() } }, cardBack, cardFront, textures[cardTextureCounter % textures.size()]));
				cards.back()->SetParent(gridAligningParent.get());
				cards.back()->OnClicked() = makeCardsOnClicked(cards.back().get());
			}
		}

		//for(auto& card : cards)
		//{
		//	std::swap(card, cards[rand() % cards.size()]);
		//}

		for(size_t y = 0; y < gridSize.Y(); y++)
		{
			for(size_t x = 0; x < gridSize.X(); x++)
			{
				auto cardSize = cards[y * gridSize.X() + x]->frontCard->GetLocalSizeAs<RelativeSize>();
				cards[y * gridSize.X() + x]->SetLocalPosition(ConvertPivotEquivalentRelativePosition({ 0, 0 }, { 0.5f, 0.5f }, RelativePosition{ { static_cast<float>(x) / gridSize.X(), static_cast<float>(y) / gridSize.Y() } }, cardSize, gridAligningParent->GetFrameSizeAs<AbsoluteSize>()));
			}
		}
	}

	void Update(float deltaTime) override
	{
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
			victoryScreen = std::make_unique<VictoryScreen>(*GetScene().GetExternalSystemAs<gsl::not_null<DeluEngine::Engine*>>(), *owningFrame);
		}
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
};


void CardMatchSceneLoader::operator()(ECS::Scene& s) const
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
}

export CardMatchSceneLoader CardMatchScene()
{
	return CardMatchSceneLoader();
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
	std::srand(std::time(nullptr));
	return TitleScene();
}
