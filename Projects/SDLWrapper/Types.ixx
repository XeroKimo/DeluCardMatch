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

	struct Color : private xk::Math::Aliases::u8Vector4
	{
		using xk::Math::Aliases::u8Vector4::Vector;

		reference R() noexcept { return X(); }
		reference G() noexcept { return Y(); }
		reference B() noexcept { return Z(); }
		reference A() noexcept { return W(); }

		const_reference R() const noexcept { return X(); }
		const_reference G() const noexcept { return Y(); }
		const_reference B() const noexcept { return Z(); }
		const_reference A() const noexcept { return W(); }
	};

	auto PollEvent(Event& event) { return SDL_PollEvent(&event); }
};
