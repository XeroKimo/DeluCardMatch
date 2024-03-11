export module xk.Math.Algorithms;

namespace xk::Math
{
	export template<class Ty>
	struct Inclusive;

	export template<class Ty>
	struct Exclusive;

	export template<class Ty>
	struct Inclusive
	{
		Ty value;

		bool operator()(const Ty& compare, const Inclusive<Ty>& max) const noexcept;
		bool operator()(const Ty& compare, const Exclusive<Ty>& max) const noexcept;
	};

	export template<class Ty>
	struct Exclusive
	{
		Ty value;

		bool operator()(const Ty& compare, const Inclusive<Ty>& max) const noexcept;
		bool operator()(const Ty& compare, const Exclusive<Ty>& max) const noexcept;
	};

	template<class Ty>
	bool Inclusive<Ty>::operator()(const Ty& compare, const Inclusive<Ty>& max) const noexcept
	{
		return value <= compare && compare <= max.value;
	}

	template<class Ty>
	bool Inclusive<Ty>::operator()(const Ty& compare, const Exclusive<Ty>& max) const noexcept
	{
		return value <= compare && compare < max.value;
	}

	template<class Ty>
	bool Exclusive<Ty>::operator()(const Ty& compare, const Inclusive<Ty>& max) const noexcept
	{
		return value < compare && compare <= max.value;
	}

	template<class Ty>
	bool Exclusive<Ty>::operator()(const Ty& compare, const Exclusive<Ty>& max) const noexcept
	{
		return value < compare && compare < max.value;
	}

	export template<class Ty>
	bool InRange(const Inclusive<Ty>& min, const Ty& value, const Inclusive<Ty>& max)
	{
		return min(value, max);
	}

	export template<class Ty>
	bool InRange(const Inclusive<Ty>& min, const Ty& value, const Exclusive<Ty>& max)
	{
		return min(value, max);
	}

	export template<class Ty>
	bool InRange(const Exclusive<Ty>& min, const Ty& value, const Inclusive<Ty>& max)
	{
		return min(value, max);
	}

	export template<class Ty>
	bool InRange(const Exclusive<Ty>& min, const Ty& value, const Exclusive<Ty>& max)
	{
		return min(value, max);
	}
}