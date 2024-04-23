module;

#include <SDL2/SDL.h>
#include <gsl/pointers>
#include <format>
#include <stdexcept>
#include <optional>
#include "MacroHelpers.h"

export module SDL2pp:Renderer;
import xk.Math.Matrix;
import :Impl;
import :Types;

namespace SDL2pp
{
	//using namespace xk::Math::Aliases;

	template<>
	struct SDL2Destructor<Renderer>
	{
		void operator()(Renderer* window) const { SDL_DestroyRenderer(window); }
	};

	template<>
	struct SDL2Destructor<Texture>
	{
		void operator()(Texture* texture) const { SDL_DestroyTexture(texture); }
	};

	template<>
	struct SDL2Destructor<Surface>
	{
		void operator()(Surface* surface) const { SDL_FreeSurface(surface); }
	};

	export struct TextureData
	{
		PixelFormat format;
		TextureAccess access;
		xk::Math::Aliases::iVector2 size;
	};

	template<class DerivedSelf>
	struct SDL2Interface<Texture, DerivedSelf>
	{
		using object_type = Texture;
		using self_type = DerivedSelf;

	public:
		TextureAccess GetTextureAccess() const
		{
			return QueryTexture().access;
		}

		BlendMode GetBlendMode() const
		{
			BlendMode mode;
			ThrowIfFailed(SDL_GetTextureBlendMode(&Get(), &mode));
			return mode;
		}

		PixelFormat GetPixelFormat() const
		{
			return QueryTexture().format;
		}

		xk::Math::Aliases::iVector2 GetSize() const
		{
			return QueryTexture().size;
		}

		void SetBlendMode(BlendMode mode)
		{
			ThrowIfFailed(SDL_SetTextureBlendMode(&Get(), mode));
		}

		TextureData QueryTexture() const
		{
			TextureData data;
			Uint32 format;
			int access;
			ThrowIfFailed(SDL_QueryTexture(&Get(), &format, &access, &data.size.X(), &data.size.Y()));
			data.format = static_cast<PixelFormat>(format);
			data.access = static_cast<TextureAccess>(access);
			return data;
		}


	private:
		const self_type& GetDerived() const noexcept { return static_cast<const self_type&>(*this); }
		object_type& Get() const noexcept { return *GetDerived().get(); }
	};

	template<class DerivedSelf>
	struct SDL2Interface<Renderer, DerivedSelf>
	{
		using object_type = Renderer;
		using self_type = DerivedSelf;

	public:

	//	/* Functions to implement
	//	* SDL_CreateRenderer
	//		SDL_CreateSoftwareRenderer
	//		SDL_CreateWindowAndRenderer
	//		SDL_DestroyRenderer
	//		SDL_GetRenderDrawBlendMode
	//		SDL_GetRenderDrawColor
	//		SDL_GetRendererInfo
	//		SDL_GetRendererOutputSize
	//		SDL_GetRenderTarget
	//		SDL_RenderCopyEx
	//		SDL_RenderCopyExF
	//		SDL_RenderCopyF
	//		SDL_RenderDrawLine
	//		SDL_RenderDrawLineF
	//		SDL_RenderDrawLines
	//		SDL_RenderDrawLinesF
	//		SDL_RenderDrawPoint
	//		SDL_RenderDrawPointF
	//		SDL_RenderDrawPoints
	//		SDL_RenderDrawPointsF
	//		SDL_RenderDrawRect
	//		SDL_RenderDrawRectF
	//		SDL_RenderDrawRects
	//		SDL_RenderDrawRectsF
	//		SDL_RenderFillRect
	//		SDL_RenderFillRectF
	//		SDL_RenderFillRects
	//		SDL_RenderFillRectsF
	//		SDL_RenderFlush
	//		SDL_RenderGeometry
	//		SDL_RenderGeometryRaw
	//		SDL_RenderGetClipRect
	//		SDL_RenderGetD3D11Device
	//		SDL_RenderGetD3D12Device
	//		SDL_RenderGetD3D9Device
	//		SDL_RenderGetIntegerScale
	//		SDL_RenderGetLogicalSize
	//		SDL_RenderGetMetalCommandEncoder
	//		SDL_RenderGetMetalLayer
	//		SDL_RenderGetScale
	//		SDL_RenderGetViewport
	//		SDL_RenderGetWindow
	//		SDL_RenderIsClipEnabled
	//		SDL_RenderLogicalToWindow
	//		SDL_RenderReadPixels
	//		SDL_RenderSetClipRect
	//		SDL_RenderSetIntegerScale
	//		SDL_RenderSetLogicalSize
	//		SDL_RenderSetScale
	//		SDL_RenderSetViewport
	//		SDL_RenderSetVSync
	//		SDL_RenderTargetSupported
	//		SDL_RenderWindowToLogical
	//	*/

		void Clear()
		{
			ThrowIfFailed(SDL_RenderClear(&Get()));
		}

		void Copy(view_ptr<Texture> texture, std::optional<Rect> sourceRect = std::nullopt, std::optional<Rect> destRect = std::nullopt)
		{
			SDL_RenderCopy(&Get(), 
				texture.get(), 
				sourceRect.has_value() ? &sourceRect.value() : static_cast<decltype(sourceRect)::value_type*>(nullptr),
				destRect.has_value() ? &destRect.value() : static_cast<decltype(destRect)::value_type*>(nullptr));
		}

		void Copy(view_ptr<Texture> texture, std::optional<Rect> sourceRect = std::nullopt, std::optional<FRect> destRect = std::nullopt)
		{
			SDL_RenderCopyF(&Get(), 
				texture.get(), 
				sourceRect.has_value() ? &sourceRect.value() : static_cast<decltype(sourceRect)::value_type*>(nullptr),
				destRect.has_value() ? &destRect.value() : static_cast<decltype(destRect)::value_type*>(nullptr));
		}

		void CopyEx(view_ptr<Texture> texture, std::optional<Rect> sourceRect, std::optional<Rect> destRect, 
			const double angle,
			std::optional<Point> center,
			RendererFlip flip)
		{
			SDL_RenderCopyEx(&Get(),
				texture.get(),
				sourceRect.has_value() ? &sourceRect.value() : static_cast<decltype(sourceRect)::value_type*>(nullptr),
				destRect.has_value() ? &destRect.value() : static_cast<decltype(destRect)::value_type*>(nullptr),
				angle,
				center.has_value() ? &center.value() : static_cast<decltype(center)::value_type*>(nullptr),
				flip);
		}

		void CopyEx(view_ptr<Texture> texture, std::optional<Rect> sourceRect, std::optional<FRect> destRect,
			const double angle,
			std::optional<FPoint> center,
			RendererFlip flip)
		{
			SDL_RenderCopyExF(&Get(),
				texture.get(),
				sourceRect.has_value() ? &sourceRect.value() : static_cast<decltype(sourceRect)::value_type*>(nullptr),
				destRect.has_value() ? &destRect.value() : static_cast<decltype(destRect)::value_type*>(nullptr),
				angle,
				center.has_value() ? &center.value() : static_cast<decltype(center)::value_type*>(nullptr),
				flip);
		}

		unique_ptr<Texture> CreateTexture(PixelFormat format, TextureAccess access, int width, int height)
		{
			return unique_ptr<Texture>{ ThrowIfNullptr(SDL_CreateTexture(&Get(), static_cast<Uint32>(format), static_cast<int>(access), width, height), "Failed to create texture")};
		}

		unique_ptr<Texture> CreateTexture(gsl::not_null<Surface*> surface)
		{
			return unique_ptr<Texture>{ ThrowIfNullptr(SDL_CreateTextureFromSurface(&Get(), surface.get()), "Failed to create texture")};
		}

		void DrawLine(xk::Math::Aliases::Vector2 p1, xk::Math::Aliases::Vector2 p2)
		{
			SDL_RenderDrawLineF(&Get(), p1.X(), p1.Y(), p2.X(), p2.Y());
		}

		xk::Math::Aliases::iVector2 GetOutputSize() const
		{
			xk::Math::Aliases::iVector2 size;
			ThrowIfFailed(SDL_GetRendererOutputSize(&Get(), &size.X(), &size.Y()));
			return size;
		}

		void Present() noexcept
		{
			SDL_RenderPresent(&Get());
		}

		void SetDrawBlendMode(SDL_BlendMode mode)
		{
			ThrowIfFailed(SDL_SetRenderDrawBlendMode(&Get(), mode));
		}

		void SetDrawColor(Color color)
		{
			ThrowIfFailed(SDL_SetRenderDrawColor(&Get(), color.R(), color.G(), color.B(), color.A()));
		}

		void SetRenderTarget(Texture* target)
		{
			ThrowIfFailed(SDL_SetRenderTarget(&Get(), target));
		}

	private:
		const self_type& GetDerived() const noexcept { return static_cast<const self_type&>(*this); }
		object_type& Get() const noexcept { return *GetDerived().get(); }
	};

	export enum class RendererFlag : std::underlying_type_t<SDL_RendererFlags>
	{
		Software = SDL_RendererFlags::SDL_RENDERER_SOFTWARE,
		Accelerated = SDL_RendererFlags::SDL_RENDERER_ACCELERATED,
		PresentVSync = SDL_RendererFlags::SDL_RENDERER_PRESENTVSYNC,
		TargetTexture = SDL_RendererFlags::SDL_RENDERER_TARGETTEXTURE
	};

	DECLARE_ENUM_BIT_FLAGS(RendererFlag);

	export unique_ptr<Renderer> CreateRenderer(gsl::not_null<Window*> window, int deviceIndex, RendererFlag flags)
	{
		return unique_ptr<Renderer>{ ThrowIfNullptr(SDL_CreateRenderer(window, deviceIndex, static_cast<std::underlying_type_t<RendererFlag>>(flags)), "Failed to create renderer")};
	}
}