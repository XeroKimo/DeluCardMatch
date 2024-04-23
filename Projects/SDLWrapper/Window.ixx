module;

#include <SDL2/SDL.h>
#include <string_view>
#include <type_traits>
#include <format>
#include <stdexcept>
#include <optional>
#include "MacroHelpers.h"

export module SDL2pp:Window;
import :Types;
import :Impl;
import xk.Math.Matrix;

namespace SDL2pp
{
	template<>
	struct SDL2Destructor<Window>
	{
		void operator()(Window* window) const { SDL_DestroyWindow(window); }
	};


	template<class DerivedSelf>
	struct SDL2Interface<Window, DerivedSelf>
	{
		using object_type = Window;
		using self_type = DerivedSelf;

		void SetPosition(xk::Math::Aliases::iVector2 position)
		{
			SDL_SetWindowPosition(&Get(), position.X(), position.Y());
		}

		xk::Math::Aliases::iVector2 GetPosition() const noexcept
		{
			xk::Math::Aliases::iVector2 position;
			SDL_GetWindowPosition(&Get(), &position.X(), &position.Y());
			return position;
		}

		xk::Math::Aliases::iVector2 GetSize() const noexcept
		{
			xk::Math::Aliases::iVector2 size;
			SDL_GetWindowSize(&Get(), &size.X(), &size.Y());
			return size;
		}

	private:
		const self_type& GetDerived() const noexcept { return static_cast<const self_type&>(*this); }
		object_type& Get() const noexcept { return *GetDerived().get(); }
	};

	export enum class WindowFlag : std::underlying_type_t<SDL_WindowFlags>
	{
		/*
		* Flags that still need to be re-implemented
			SDL_WINDOW_OPENGL
			SDL_WINDOW_VULKAN
			SDL_WINDOW_SHOWN
			SDL_WINDOW_HIDDEN
			SDL_WINDOW_INPUT_GRABBED
			SDL_WINDOW_INPUT_FOCUS
			SDL_WINDOW_MOUSE_FOCUS
			SDL_WINDOW_FOREIGN
			SDL_WINDOW_ALLOW_HIGHDPI
			SDL_WINDOW_MOUSE_CAPTURE
			SDL_WINDOW_ALWAYS_ON_TOP
			SDL_WINDOW_SKIP_TASKBAR
			SDL_WINDOW_UTILITY
			SDL_WINDOW_TOOLTIP
			SDL_WINDOW_POPUP_MENU
		* 
		*/

		FullScreen = SDL_WINDOW_FULLSCREEN,
		FullScreen_Desktop = SDL_WINDOW_FULLSCREEN_DESKTOP,
		OpenGL = SDL_WINDOW_OPENGL,
		Borderless = SDL_WINDOW_BORDERLESS,
		Resizable = SDL_WINDOW_RESIZABLE,
		Minimized = SDL_WINDOW_MINIMIZED,
		Maximized = SDL_WINDOW_MAXIMIZED,
	};

	DECLARE_ENUM_BIT_FLAGS(WindowFlag);

	export unique_ptr<Window> CreateWindow(std::string_view title, xk::Math::Aliases::iVector2 size, WindowFlag flags, std::optional<xk::Math::Aliases::iVector2> position = std::nullopt)
	{
		xk::Math::Aliases::iVector2 unwrappedPosition = position.value_or(xk::Math::Aliases::iVector2{ SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED });
		return unique_ptr<Window>{ ThrowIfNullptr(SDL_CreateWindow(title.data(), unwrappedPosition.X(), unwrappedPosition.Y(), size.X(), size.Y(), static_cast<std::underlying_type_t<WindowFlag>>(flags)), "Failed to create window")};
	}
};