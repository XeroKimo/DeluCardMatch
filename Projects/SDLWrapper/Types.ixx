module;
#include <SDL2/SDL.h>
export module SDL2pp:Types;
import xk.Math.Matrix;



export namespace SDL2pp
{
	using Window = SDL_Window;
	using Renderer = SDL_Renderer;
	using Texture = SDL_Texture;
	using Surface = SDL_Surface;
	using Event = SDL_Event;
	using EventType = SDL_EventType;
	using Rect = SDL_Rect;
	using FRect = SDL_FRect;
	using Point = SDL_Point;
	using FPoint = SDL_FPoint;
	using RendererFlip = SDL_RendererFlip;
	using PixelFormat = SDL_PixelFormatEnum;
	using TextureAccess = SDL_TextureAccess;
	using BlendMode = SDL_BlendMode;

	using KeyCode = SDL_KeyCode;

	struct Color
	{
		xk::Math::Aliases::u8Vector4 value;

		auto& R() noexcept { return value.X(); }
		auto& G() noexcept { return value.Y(); }
		auto& B() noexcept { return value.Z(); }
		auto& A() noexcept { return value.W(); }

		const auto& R() const noexcept { return value.X(); }
		const auto& G() const noexcept { return value.Y(); }
		const auto& B() const noexcept { return value.Z(); }
		const auto& A() const noexcept { return value.W(); }
	};

	auto PollEvent(Event& event) { return SDL_PollEvent(&event); }
};
