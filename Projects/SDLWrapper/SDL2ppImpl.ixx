module;
#include <memory>
#include <SDL2/SDL.h>
#include <format>
#include <stdexcept>
#include <concepts>
#include <string_view>
#include <compare>

export module SDL2pp:Impl;

namespace SDL2pp
{
	export template<class Ty>
	struct SDL2Destructor
	{

	};

	//Temporary, do not actually specialize outside of this library
	//If intellisense is giving issues when trying to use SDL2 interfaces
	//bring this interface into scope to fix the intellise with using SDL2pp::SDL2Interface
	export template<class Ty, class DerivedSelf>
	struct SDL2Interface
	{

	};

	export class Error : public std::runtime_error
	{
	public:
		using std::runtime_error::runtime_error;
	};

	template<class Ty>
	Ty* ThrowIfNullptr(Ty* value, std::string_view message)
	{
		if (!value)
			throw Error{ std::format("{}. Error: {}", message,  SDL_GetError() ) };
		else
			return value;
	}

	void ThrowIfFailed(int statusCode)
	{
		if (statusCode == 0)
			return;
		else
			throw Error{ std::format("Error {}: {}", statusCode,  SDL_GetError()) };
	}

	export template<class Ty>
	class view_ptr : private SDL2Interface<Ty, view_ptr<Ty>>
	{
		using interface_type = SDL2Interface<Ty, view_ptr<Ty>>;

		friend view_ptr;
		friend interface_type;

	private:
		Ty* m_ptr = nullptr;

	public:
		view_ptr() noexcept = default;
		view_ptr(Ty* ptr) noexcept : m_ptr(ptr) {}
		view_ptr(std::nullptr_t) noexcept {}

		view_ptr& operator=(Ty* ptr) noexcept
		{
			m_ptr = ptr;
			return *this;
		}
		view_ptr& operator=(std::nullptr_t) noexcept
		{
			m_ptr = nullptr;
			return *this;
		}

		Ty* get() const noexcept { return m_ptr; }
		Ty* release() noexcept { return std::exchange(m_ptr, nullptr); }
		Ty* reset(Ty* ptr) noexcept { return std::exchange(m_ptr, ptr); }

		operator Ty*() const& noexcept { return get(); }
		operator Ty*() && noexcept { return std::move(*this).get(); }

		template<class Ty1, class Ty2>
		friend auto operator<=>(const view_ptr<Ty1>& lh, const view_ptr<Ty2>& rh) noexcept
		{
			return lh.get() <=> rh.get();
		}

		template<class Ty1>
		friend bool operator<=>(const view_ptr<Ty1>& lh, std::nullptr_t) noexcept
		{
			return lh.get() <=> static_cast<Ty1*>(nullptr);
		}

		interface_type& operator*() noexcept { return static_cast<interface_type&>(*this); }
		const interface_type& operator*() const noexcept { return static_cast<const interface_type&>(*this); }

		interface_type* operator->() noexcept { return static_cast<interface_type*>(this); }
		const interface_type* operator->() const noexcept { return static_cast<const interface_type*>(this); }
	};

	export template<class Ty>
	class unique_ptr : private SDL2Interface<Ty, unique_ptr<Ty>>
	{
		using interface_type = SDL2Interface<Ty, unique_ptr<Ty>>;
		friend interface_type;
	private:
		std::unique_ptr<Ty, SDL2Destructor<Ty>> m_ptr;

	public:
		unique_ptr() noexcept = default;
		unique_ptr(std::nullptr_t) noexcept {}
		unique_ptr(Ty* ptr) noexcept : m_ptr{ ptr } {}
		unique_ptr(const unique_ptr<Ty>&) noexcept = delete;
		unique_ptr(unique_ptr<Ty>&& other) noexcept : m_ptr{ std::move(other.m_ptr) } {}
		~unique_ptr() = default;

		unique_ptr<Ty>& operator=(std::nullptr_t) noexcept { m_ptr = nullptr; return *this; }
		unique_ptr<Ty>& operator=(const unique_ptr<Ty>&) noexcept = delete;
		unique_ptr<Ty>& operator=(unique_ptr<Ty>&& other) noexcept { m_ptr = std::move(other.m_ptr); return *this; }


		template<class Ty1, class Ty2>
		friend auto operator<=>(const unique_ptr<Ty1>& lh, const unique_ptr<Ty2>& rh) noexcept
		{
			return lh.m_ptr <=> rh.m_ptr;
		}
		template<class Ty1>
		friend auto operator<=>(const unique_ptr<Ty1>& lh, std::nullptr_t) noexcept
		{
			return lh.m_ptr <=> nullptr;
		}

		const interface_type* operator->() const noexcept { return static_cast<const interface_type*>(this); }
		const interface_type& operator*() const noexcept { return static_cast<const interface_type&>(*this); }

		interface_type* operator->() noexcept { return static_cast<interface_type*>(this); }
		interface_type& operator*() noexcept { return static_cast<interface_type&>(*this); }

		operator bool() const noexcept { return static_cast<bool>(m_ptr); }
	public:
		auto release() noexcept
		{
			return m_ptr.release();
		}

		void reset(Ty* ptr) noexcept
		{
			m_ptr.reset(ptr);
		}

		void swap(unique_ptr& other) noexcept
		{
			m_ptr.swap(other.m_ptr);
		}

		auto get() const noexcept { return m_ptr.get(); }

		auto get_deleter() const noexcept { return m_ptr.get_deleter(); }
	};

	export template<class Ty>
	class shared_ptr : private SDL2Interface<Ty, shared_ptr<Ty>>
	{
		using interface_type = SDL2Interface<Ty, shared_ptr<Ty>>;
		friend interface_type;
	private:
		std::shared_ptr<Ty> m_ptr;

	public:
		shared_ptr() noexcept = default;
		shared_ptr(std::nullptr_t) noexcept {}
		shared_ptr(Ty* ptr) noexcept : m_ptr{ ptr, SDL2Destructor<Ty>{} } {}
		shared_ptr(unique_ptr<Ty> other) noexcept : m_ptr{ other.release(), SDL2Destructor<Ty>{} } {}
		shared_ptr(const shared_ptr<Ty>& other) noexcept : m_ptr{ other.m_ptr } {}
		shared_ptr(shared_ptr<Ty>&& other) noexcept : m_ptr{ std::move(other.m_ptr) } {}
		~shared_ptr() = default;

		shared_ptr<Ty>& operator=(std::nullptr_t) noexcept { m_ptr = nullptr; return *this; }
		shared_ptr<Ty>& operator=(const shared_ptr<Ty>& other) { m_ptr = other.m_ptr; return *this; }
		shared_ptr<Ty>& operator=(shared_ptr<Ty>&& other) noexcept { m_ptr = std::move(other.m_ptr); return *this; }


		template<class Ty1, class Ty2>
		friend auto operator<=>(const shared_ptr<Ty1>& lh, const shared_ptr<Ty2>& rh) noexcept
		{
			return lh.m_ptr <=> rh.m_ptr;
		}
		template<class Ty1>
		friend auto operator<=>(const shared_ptr<Ty1>& lh, std::nullptr_t) noexcept
		{
			return lh.m_ptr <=> nullptr;
		}

		template<class Ty1, class Ty2>
		friend bool operator==(const shared_ptr<Ty1>& lh, const shared_ptr<Ty2>& rh) noexcept
		{
			return lh.m_ptr == rh.m_ptr;
		}

		const interface_type* operator->() const noexcept { return static_cast<const interface_type*>(this); }
		const interface_type& operator*() const noexcept { return static_cast<const interface_type&>(*this); }

		interface_type* operator->() noexcept { return static_cast<interface_type*>(this); }
		interface_type& operator*() noexcept { return static_cast<interface_type&>(*this); }

		operator bool() const noexcept { return static_cast<bool>(m_ptr); }
	public:
		auto release() noexcept
		{
			return m_ptr.release();
		}

		void reset(Ty* ptr) noexcept
		{
			m_ptr.reset(ptr);
		}

		void swap(shared_ptr& other) noexcept
		{
			m_ptr.swap(other.m_ptr);
		}

		auto get() const noexcept { return m_ptr.get(); }
	};

	//export template<class Ty>
	//class weak_ptr : private std::weak_ptr<Ty>
	//{

	//};
}