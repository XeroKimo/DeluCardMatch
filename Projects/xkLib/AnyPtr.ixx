module;

#include <typeinfo>

export module xk.AnyPtr;

namespace xk
{
	export class AnyPtr
	{
	private:
		void* m_ptr = nullptr;
		const std::type_info* m_type = nullptr;

	public:
		AnyPtr() noexcept = default;
		template<class Ty>
		AnyPtr(Ty* ptr) noexcept :
			m_ptr{ ptr },
			m_type{ ptr ? &typeid(Ty) : nullptr }
		{

		}

		template<class Ty>
		AnyPtr& operator=(Ty* ptr) noexcept
		{
			m_ptr = ptr;
			m_type = ptr ? &typeid(Ty) : nullptr;
			return *this;
		}

		template<class Ty>
		bool Is() const noexcept
		{
			return m_type && typeid(Ty) == *m_type;
		}

		template<class Ty>
		Ty* As() const noexcept
		{
			if (Is<Ty>())
			{
				return reinterpret_cast<Ty*>(m_ptr);
			}
			return nullptr;
		}
	};

	export class AnyRef
	{
	private:
		void* m_ptr = nullptr;
		const std::type_info* m_type = nullptr;

	public:
		AnyRef() noexcept = delete;

		template<class Ty>
		AnyRef(Ty& ptr) noexcept :
			m_ptr{ &ptr },
			m_type{ &typeid(Ty) }
		{

		}

		template<class Ty>
		bool Is() const noexcept
		{
			return m_type && typeid(Ty) == *m_type;
		}

		template<class Ty>
		Ty& As() const
		{
			if (Is<Ty>())
			{
				return *reinterpret_cast<Ty*>(m_ptr);
			}

			throw std::bad_cast();
		}
	};


}