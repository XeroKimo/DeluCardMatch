module;

#include <vector>
#include <memory>
#include <gsl/pointers>
#include <SDL2/SDL_image.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <string_view>
#include <functional>
#include <SDL2/SDL_ttf.h>

export module DeluEngine:Renderer;
export import SDL2pp;
import xk.Math.Angles;
import xk.Math.Matrix;
import xk.Math.Color;

namespace DeluEngine
{
	using namespace xk::Math::Aliases;
	export class Renderer;

	export struct SpriteData
	{
		SDL2pp::shared_ptr<SDL2pp::Texture> texture;
		SDL2pp::Rect drawRect;
	};

	export class Sprite
	{
	public:
		Vector2 position;
		xk::Math::Degree<float> angle;

	private:
		gsl::not_null<Renderer*> m_owningRenderer;
		std::shared_ptr<SpriteData> m_data;

	public:
		Sprite(gsl::not_null<Renderer*> spriteRenderer, std::shared_ptr<SpriteData> spriteData) :
			m_owningRenderer(spriteRenderer),
			m_data(std::move(spriteData))
		{
		}

	public:
		void SetSpriteData(std::shared_ptr<SpriteData> data);

		SpriteData* GetSpriteData() const noexcept
		{
			return m_data.get();
		}

		std::shared_ptr<SpriteData> GetSpriteDataShared() const noexcept
		{
			return m_data;
		}
	};

	export class DebugRenderer
	{
	private:
		SDL2pp::view_ptr<SDL2pp::Renderer> m_backend;

	public:
		DebugRenderer(SDL2pp::view_ptr<SDL2pp::Renderer> backend) :
			m_backend(backend)
		{

		}

		void SetDrawColor(xk::Math::Color color)
		{
			m_backend->SetDrawColor({color[0], color[1], color[2], color[3]});
		}

		void DrawLine(Vector2 p1, Vector2 p2)
		{
			iVector2 outputSize = m_backend->GetOutputSize();
			p1.Y() = -p1.Y() + outputSize.Y();
			p2.Y() = -p2.Y() + outputSize.Y();
			m_backend->DrawLine(p1, p2);
		}
	};

	export class Renderer
	{
	private:
		struct SpriteDeleter
		{
			Renderer* renderer;
			void operator()(Sprite* sprite) { renderer->DeleteSprite(sprite); }
		};

	public:
		using SpriteHandle = std::unique_ptr<Sprite, SpriteDeleter>;

	private:
		std::vector<Sprite*> m_sprites;

	public:
		std::shared_ptr<SpriteData> defaultSpriteData;
		SDL2pp::unique_ptr<SDL2pp::Renderer> backend;
		SDL2pp::Color clearColor = SDL2pp::Color{ 96.f, 128, 255, 255 };
		std::vector<std::function<void(DebugRenderer&)>> debugCallbacks;
		TTF_Font* testFont;
		SDL_Surface* testFontSurface;
		SDL_Texture* testFontTexture;
	public:
		Renderer(SDL2pp::view_ptr<SDL2pp::Window> window, int deviceIndex = -1, SDL2pp::RendererFlag flags = SDL2pp::RendererFlag::Accelerated) :
			backend(SDL2pp::CreateRenderer(window, deviceIndex, flags))
		{
			TTF_Init();
			testFont = TTF_OpenFont("Arial.ttf", 12);
			testFontSurface = TTF_RenderText_Shaded(testFont, "Test test, 1. 2. 3", { 255, 255, 255 }, { 0, 0, 0 });
			testFontTexture = SDL_CreateTextureFromSurface(backend.get(), testFontSurface);
		}

		void Render()
		{
			backend->SetDrawColor(clearColor);
			backend->Clear();

			DrawSprites();
			DebugRenderer debugRenderer{ GetDebugRenderer() };
			for (auto& callback : debugCallbacks) { callback(debugRenderer); }
			SDL_Rect textLocation = { 400, 200, testFontSurface->w, testFontSurface->h };
			backend->Copy(testFontTexture, std::nullopt, textLocation);
		}

		SpriteHandle CreateSprite(std::shared_ptr<SpriteData> spriteData)
		{
			SpriteHandle sprite{ new Sprite(this, spriteData ? std::move(spriteData) : defaultSpriteData), SpriteDeleter{this} };
			m_sprites.push_back(sprite.get());
			return sprite;
		}

		DebugRenderer GetDebugRenderer()
		{
			return { backend.get() };
		}

	private:
		void DrawSprites()
		{
			iVector2 outputSize = backend->GetOutputSize();
			for (const auto& sprite : m_sprites)
			{
				SDL2pp::Rect sourceRect = sprite->GetSpriteData()->drawRect;
				SDL2pp::FRect destRect;
				destRect.w = static_cast<float>(sourceRect.w);
				destRect.h = static_cast<float>(sourceRect.h);
				destRect.x = sprite->position.X();
				destRect.y = -sprite->position.Y() + outputSize.Y();
				backend->CopyEx(sprite->GetSpriteData()->texture.get(), sourceRect, destRect, sprite->angle._value, std::nullopt, SDL2pp::RendererFlip::SDL_FLIP_NONE);
			}
		}		
		
		void DeleteSprite(Sprite* sprite)
		{
			std::erase(m_sprites, sprite);
			delete sprite;
		}
	};

	export using SpriteHandle = Renderer::SpriteHandle;

	export std::vector<std::shared_ptr<SpriteData>> LoadSprites(std::string_view filePath, Renderer& renderer)
	{
		std::vector<std::shared_ptr<SpriteData>> sprites;

		std::ifstream file{ std::string{ filePath } };
		nlohmann::json json = nlohmann::json::parse(file);
		std::string imageFilePath = json["filePath"];
		SDL2pp::unique_ptr<SDL2pp::Surface> surface{ IMG_Load(imageFilePath.data()) };
		for (auto& spritesData : json["sprites"])
		{
			auto rectArray = spritesData["rect"];
			int xMin = rectArray[0];
			int yMin = rectArray[1];
			int xMax = rectArray[2];
			int yMax = rectArray[3];

			SDL2pp::Rect rect
			{
				.x = xMin,
				.y = yMin,
				.w = xMax - xMin,
				.h = yMax - yMin
			};

			sprites.push_back(std::make_shared<SpriteData>(renderer.backend->CreateTexture(surface.get()), rect));
		}

		return sprites;
	}

	void Sprite::SetSpriteData(std::shared_ptr<SpriteData> data)
	{
		m_data = data ? std::move(data) : m_owningRenderer->defaultSpriteData;
	}
};