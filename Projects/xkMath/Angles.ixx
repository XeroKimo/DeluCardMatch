module;

export module xk.Math.Angles;
export import <compare>;

namespace xk::Math
{
	export template<class Ty>
	struct Degree
	{
		Ty _value{};

		auto operator<=>(const Degree&) const noexcept = default;

		Degree& operator=(const Degree& value) noexcept
		{
			_value = value._value;
			return *this;
		}

		Degree& operator+=(const Degree& rh) noexcept
		{
			_value += rh._value;
			return *this;
		}

		Degree& operator-=(const Degree& rh) noexcept
		{
			_value -= rh._value;
			return *this;
		}

		friend Degree operator+(Degree lh, const Degree& rh) noexcept
		{
			return lh += rh;
		}

		friend Degree operator-(Degree lh, const Degree& rh) noexcept
		{
			return lh -= rh;
		}
	};
}