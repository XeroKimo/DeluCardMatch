module;


export module SDL2pp:Impl;
export import <memory>			;
export import <SDL2/SDL.h>		;
export import <format>			;
export import <stdexcept>		;
export import <concepts>		;
export import <string_view>		;
export import <compare>			;

namespace SDL2pp
{
	template<class Ty>
	struct SDL2Destructor;

	template<class Ty, class DerivedSelf>
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
	class unique_ptr : private std::unique_ptr<Ty, SDL2Destructor<Ty>>, private SDL2Interface<Ty, unique_ptr<Ty>>
	{
		using base_type = std::unique_ptr<Ty, SDL2Destructor<Ty>>;
		using interface_type = SDL2Interface<Ty, unique_ptr<Ty>>;

		friend unique_ptr;
		friend interface_type;

	public:
		using base_type::unique_ptr;
		using base_type::operator=;
		using base_type::release;
		using base_type::reset;
		using base_type::get;
		using base_type::get_deleter;
		using base_type::operator bool;

		//For a quick and dirty implementation, operator<=> for some reason prioritizes std::shared_ptr's one
		//so I must implement comparison operators the old fashioned way
		template<class Ty1, class Ty2>
		friend bool operator==(const unique_ptr<Ty1>& lh, const unique_ptr<Ty2>& rh) noexcept
		{
			return static_cast<const unique_ptr<Ty1>::base_type&>(lh) == static_cast<const unique_ptr<Ty2>::base_type&>(rh);
		}

		template<class Ty1, class Ty2>
		friend bool operator!=(const unique_ptr<Ty1>& lh, const unique_ptr<Ty2>& rh) noexcept
		{
			return static_cast<const unique_ptr<Ty1>::base_type&>(lh) != static_cast<const unique_ptr<Ty2>::base_type&>(rh);
		}

		template<class Ty1, class Ty2>
		friend bool operator<(const unique_ptr<Ty1>& lh, const unique_ptr<Ty2>& rh) noexcept
		{
			return static_cast<const unique_ptr<Ty1>::base_type&>(lh) < static_cast<const unique_ptr<Ty2>::base_type&>(rh);
		}

		template<class Ty1, class Ty2>
		friend bool operator<= (const unique_ptr<Ty1>&lh, const unique_ptr<Ty2>&rh) noexcept
		{
			return static_cast<const unique_ptr<Ty1>::base_type&>(lh) <= static_cast<const unique_ptr<Ty2>::base_type&>(rh);
		}

		template<class Ty1, class Ty2>
		friend bool operator>(const unique_ptr<Ty1>& lh, const unique_ptr<Ty2>& rh) noexcept
		{
			return static_cast<const unique_ptr<Ty1>::base_type&>(lh) > static_cast<const unique_ptr<Ty2>::base_type&>(rh);
		}

		template<class Ty1, class Ty2>
		friend bool operator<=(const unique_ptr<Ty1>& lh, const unique_ptr<Ty2>& rh) noexcept
		{
			return static_cast<const unique_ptr<Ty1>::base_type&>(lh) <= static_cast<const unique_ptr<Ty2>::base_type&>(rh);
		}

		template<class Ty1>
		friend bool operator==(const unique_ptr<Ty1>& lh, std::nullptr_t) noexcept
		{
			return static_cast<const base_type&>(lh) == nullptr;
		}

		template<class Ty1>
		friend bool operator!=(const unique_ptr<Ty1>& lh, std::nullptr_t) noexcept
		{
			return static_cast<const base_type&>(lh) != nullptr;
		}

		template<class Ty1>
		friend bool operator<(const unique_ptr<Ty1>&lh, std::nullptr_t) noexcept
		{
			return static_cast<const base_type&>(lh) < nullptr;
		}

		template<class Ty1>
		friend bool operator<=(const unique_ptr<Ty1>&lh, std::nullptr_t) noexcept
		{
			return static_cast<const base_type&>(lh) <= nullptr;
		}

		template<class Ty1>
		friend bool operator>(const unique_ptr<Ty1>&lh, std::nullptr_t) noexcept
		{
			return static_cast<const base_type&>(lh) > nullptr;
		}

		template<class Ty1>
		friend bool operator<=(const unique_ptr<Ty1>&lh, std::nullptr_t) noexcept
		{
			return static_cast<const base_type&>(lh) >= nullptr;
		}
		
		interface_type& operator*() noexcept { return static_cast<interface_type&>(*this); }
		const interface_type& operator*() const noexcept { return static_cast<const interface_type&>(*this); }
		
		interface_type* operator->() noexcept { return static_cast<interface_type*>(this); }
		const interface_type* operator->() const noexcept { return static_cast<const interface_type*>(this); }
	};

	export template<class Ty>
		class shared_ptr : private std::shared_ptr<Ty>, private SDL2Interface<Ty, shared_ptr<Ty>>
	{
		using base_type = std::shared_ptr<Ty>;
		using interface_type = SDL2Interface<Ty, shared_ptr<Ty>>;

		friend shared_ptr;
		friend interface_type;

	public:
		using base_type::shared_ptr;
		using base_type::operator=;
		using base_type::reset;
		using base_type::get;
		using base_type::operator bool;

		template<class Ty2>
		shared_ptr(SDL2pp::unique_ptr<Ty2> ptr) : base_type{ std::unique_ptr<Ty2, SDL2Destructor<Ty2>>{ ptr.release()} }
		{

		}

		template<class Ty2, class Deleter>
		shared_ptr(Ty2* ptr, Deleter d) = delete;

		template<class Ty2, class Deleter, class Alloc>
		shared_ptr(Ty2* ptr, Deleter d, Alloc alloc) = delete;

		template<class Deleter>
		shared_ptr(std::nullptr_t, Deleter d) = delete;

		template<class Deleter, class Alloc>
		shared_ptr(std::nullptr_t, Deleter d, Alloc alloc) = delete;

		template<class Ty2, class Deleter>
		void reset(Ty2* ptr, Deleter d) = delete;

		template<class Ty2, class Deleter, class Alloc>
		void reset(Ty2* ptr, Deleter d, Alloc alloc) = delete;

		//For a quick and dirty implementation, operator<=> for some reason prioritizes std::shared_ptr's one
		//so I must implement comparison operators the old fashioned way
		template<class Ty1, class Ty2>
		friend bool operator==(const shared_ptr<Ty1>& lh, const shared_ptr<Ty2>& rh) noexcept
		{
			return static_cast<const shared_ptr<Ty1>::base_type&>(lh) == static_cast<const shared_ptr<Ty2>::base_type&>(rh);
		}

		template<class Ty1, class Ty2>
		friend bool operator!=(const shared_ptr<Ty1>& lh, const shared_ptr<Ty2>& rh) noexcept
		{
			return static_cast<const shared_ptr<Ty1>::base_type&>(lh) != static_cast<const shared_ptr<Ty2>::base_type&>(rh);
		}

		template<class Ty1, class Ty2>
		friend bool operator<(const shared_ptr<Ty1>& lh, const shared_ptr<Ty2>& rh) noexcept
		{
			return static_cast<const shared_ptr<Ty1>::base_type&>(lh) < static_cast<const shared_ptr<Ty2>::base_type&>(rh);
		}

		template<class Ty1, class Ty2>
		friend bool operator<= (const shared_ptr<Ty1>& lh, const shared_ptr<Ty2>& rh) noexcept
		{
			return static_cast<const shared_ptr<Ty1>::base_type&>(lh) <= static_cast<const shared_ptr<Ty2>::base_type&>(rh);
		}

		template<class Ty1, class Ty2>
		friend bool operator>(const shared_ptr<Ty1>& lh, const shared_ptr<Ty2>& rh) noexcept
		{
			return static_cast<const shared_ptr<Ty1>::base_type&>(lh) > static_cast<const shared_ptr<Ty2>::base_type&>(rh);
		}

		template<class Ty1, class Ty2>
		friend bool operator<=(const shared_ptr<Ty1>& lh, const shared_ptr<Ty2>& rh) noexcept
		{
			return static_cast<const shared_ptr<Ty1>::base_type&>(lh) <= static_cast<const shared_ptr<Ty2>::base_type&>(rh);
		}

		template<class Ty1>
		friend bool operator==(const shared_ptr<Ty1>& lh, std::nullptr_t) noexcept
		{
			return static_cast<const base_type&>(lh) == nullptr;
		}

		template<class Ty1>
		friend bool operator!=(const shared_ptr<Ty1>& lh, std::nullptr_t) noexcept
		{
			return static_cast<const base_type&>(lh) != nullptr;
		}

		template<class Ty1>
		friend bool operator<(const shared_ptr<Ty1>& lh, std::nullptr_t) noexcept
		{
			return static_cast<const base_type&>(lh) < nullptr;
		}

		template<class Ty1>
		friend bool operator<=(const shared_ptr<Ty1>& lh, std::nullptr_t) noexcept
		{
			return static_cast<const base_type&>(lh) <= nullptr;
		}

		template<class Ty1>
		friend bool operator>(const shared_ptr<Ty1>& lh, std::nullptr_t) noexcept
		{
			return static_cast<const base_type&>(lh) > nullptr;
		}

		template<class Ty1>
		friend bool operator<=(const shared_ptr<Ty1>& lh, std::nullptr_t) noexcept
		{
			return static_cast<const base_type&>(lh) >= nullptr;
		}

		interface_type& operator*() noexcept { return static_cast<interface_type&>(*this); }
		const interface_type& operator*() const noexcept { return static_cast<const interface_type&>(*this); }

		interface_type* operator->() noexcept { return static_cast<interface_type*>(this); }
		const interface_type* operator->() const noexcept { return static_cast<const interface_type*>(this); }
	};

	//export template<class Ty>
	//class shared_ptr : private std::shared_ptr<Ty>, private SDL2Interface<Ty, shared_ptr<Ty>>
	//{

	//};

	//export template<class Ty>
	//class weak_ptr : private std::weak_ptr<Ty>
	//{

	//};
}