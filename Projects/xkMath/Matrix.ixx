module;

#include <array>
#include <algorithm>
#include <numeric>
#include <concepts>
#include <cstdint>
#include <cmath>
#pragma warning(disable:4244)
export module xk.Math.Matrix;

namespace xk::Math
{
	export template<class Ty, size_t M, size_t N>
	struct Matrix;

	export template<class Ty, size_t M, size_t N>
	constexpr Matrix<Ty, N, M> Transpose(const Matrix<Ty, M, N>& mat) noexcept;

	template<class Ty, bool IsConst>
	struct BackingMatrixRef
	{
		using matrix = std::remove_const_t<Ty>;
		using matrix_pointer = std::remove_const_t<Ty>*;
		using matrix_reference = std::remove_const_t<Ty>&;
		using reference = matrix::reference;

		matrix_pointer m_matrix;
	};

	template<class Ty>
	struct BackingMatrixRef<Ty, true>
	{
		using matrix = const std::remove_const_t<Ty>;
		using matrix_pointer = const std::remove_const_t<Ty>*;
		using matrix_reference = const std::remove_const_t<Ty>&;
		using reference = matrix::const_reference;

		matrix_pointer m_matrix;
	};

	export template<class Ty, size_t M, size_t N, bool IsConst>
	class ColumnRef : private BackingMatrixRef<Matrix<Ty, M,  N>, IsConst>
	{
		using base_type = BackingMatrixRef<Matrix<Ty, M, N>, IsConst>;
		using matrix = base_type::matrix;
		using matrix_pointer = base_type::matrix_pointer;
		using matrix_reference = base_type::matrix_reference;
		using reference = base_type::reference;

		using base_type::m_matrix;
		size_t m_selectedColumn;

	public:
		constexpr ColumnRef(matrix_reference matrix, size_t column) :
			base_type(&matrix),
			m_selectedColumn(column)
		{

		}

	public:
		constexpr reference operator[](size_t index) const;
	};

	export template<class Ty, size_t M, size_t N, bool IsConst>
	class RowRef : private BackingMatrixRef<Matrix<Ty, M, N>, IsConst>
	{
		using base_type = BackingMatrixRef<Matrix<Ty, M, N>, IsConst>;
		using matrix = base_type::matrix;
		using matrix_pointer = base_type::matrix_pointer;
		using matrix_reference = base_type::matrix_reference;

		using reference = base_type::reference;
		using base_type::m_matrix;
		size_t m_selectedRow;

	public:
		constexpr RowRef(matrix_reference matrix, size_t row) :
			base_type(&matrix),
			m_selectedRow(row)
		{

		}

	public:
		constexpr reference operator[](size_t index) const;
	};

	template<class Ty, size_t M, size_t N>
	ColumnRef(Matrix<Ty, M, N>&, size_t) -> ColumnRef<Ty, M, N, false>;

	template<class Ty, size_t M, size_t N>
	ColumnRef(const Matrix<Ty, M, N>&, size_t) -> ColumnRef<Ty, M, N, true>;

	template<class Ty, size_t M, size_t N>
	RowRef(Matrix<Ty, M, N>&, size_t) -> RowRef<Ty, M, N, false>;

	template<class Ty, size_t M, size_t N>
	RowRef(const Matrix<Ty, M, N>&, size_t) -> RowRef<Ty, M, N, true>;

	template<class Ty1, class Ty2, size_t M, size_t N, size_t M2, bool IsConst1, bool IsConst2>
	constexpr auto operator*(RowRef<Ty1, M, N, IsConst1> lh, ColumnRef<Ty2, N, M2, IsConst2> rh);

	template<class Ty, size_t M, size_t N>
	struct Matrix
	{
		static constexpr size_t row_count = M;
		static constexpr size_t column_count = N;
		static constexpr size_t element_count = row_count * column_count;
		static constexpr bool is_square_matrix = row_count == column_count;
		using value = Ty;
		using reference = Ty&;
		using const_reference = const Ty&;

		std::array<value, element_count> _values{};

	public:
		constexpr Matrix() = default;	

		template<std::convertible_to<Ty> Ty2>
		constexpr Matrix(const Matrix<Ty2, M, N>& other)
		{
			std::copy(other._values.begin(), other._values.end(), _values.begin());
		}

		template<std::convertible_to<Ty>... Ty2>
			requires (sizeof...(Ty2) <= M * N) //Some stupid reason consumers of the library can't see element_count when trying to instantiate this constructor
		constexpr Matrix(Ty2... values) :
			_values({ static_cast<Ty>(values)... })
		{
			auto copy = _values;
			for (size_t i = 0; i < row_count; i++)
			{
				for (size_t j = 0; j < column_count; j++)
				{
					_values[ColumnMajorIndex(i, j)] = copy[RowMajorIndex(i, j)];
				}
			}
		}

		static constexpr Matrix Identity() requires (is_square_matrix)
		{
			Matrix mat;
			for (size_t i = 0; i < column_count; i++)
			{
				mat.At(i, i) = 1;
			}
			return mat;
		}

	public:
		constexpr reference At(size_t row, size_t column) { return _values[ColumnMajorIndex(row, column)]; }
		constexpr const_reference At(size_t row, size_t column) const { return _values[ColumnMajorIndex(row, column)]; }

	public:
		template<class Ty2>
		constexpr Matrix& operator+=(const Matrix<Ty2, row_count, column_count>& rh)
		{
			for (size_t i = 0; i < element_count; i++)
			{
				_values[i] += rh._values[i];
			}

			return *this;
		}

		template<class Ty2>
		constexpr Matrix& operator-=(const Matrix<Ty2, row_count, column_count>& rh)
		{
			for (size_t i = 0; i < element_count; i++)
			{
				_values[i] -= rh._values[i];
			}

			return *this;
		}

		constexpr Matrix operator-() const
		{
			Matrix temp = *this;
			temp *= -1;
			return temp;
		}

		template<class Ty2, size_t M2>
		friend constexpr Matrix<decltype(std::declval<Ty>() * std::declval<Ty2>()), M, M2> operator*(const Matrix<Ty, M, N>& lh, const Matrix<Ty2, N, M2>& rh)
		{
			Matrix<decltype(std::declval<Ty>()* std::declval<Ty2>()), M, M2> result;
			for (size_t row = 0; row < result.row_count; row++)
			{
				for (size_t column = 0; column < result.column_count; column++)
				{
					result.At(row, column) = RowRef(lh, row) * ColumnRef(rh, column);
				}
			}
			return result;
		}

		template<class Ty2>
		friend constexpr Matrix operator+(Matrix<Ty2, row_count, column_count> lh, const Matrix<Ty2, row_count, column_count>& rh)
		{
			return lh += rh;
		}

		template<class Ty2>
		friend constexpr Matrix operator-(Matrix<Ty2, row_count, column_count> lh, const Matrix<Ty2, row_count, column_count>& rh)
		{
			return lh -= rh;
		}

		template<class Ty2, size_t M2, size_t N2>
		constexpr Matrix& operator*=(const Matrix<Ty2, M2, N2>& rh) requires (M2 == N) && (N2 == N)
		{
			*this = (*this) * rh;
			return *this;
		}

		template<class Ty2>
			requires std::is_arithmetic_v<Ty2>
		constexpr Matrix& operator*=(Ty2 scalar)
		{
			for (reference value : _values)
			{
				value *= scalar;
			}
			return *this;
		}

		template<class Ty2>
			requires std::is_arithmetic_v<Ty2>
		constexpr Matrix& operator/=(Ty2 scalar)
		{
			for (reference value : _values)
			{
				value /= scalar;
			}
			return *this;
		}

		template<class Ty2>
			requires std::is_arithmetic_v<Ty2>
		friend constexpr Matrix operator*(Matrix lh, Ty2 scalar)
		{
			return lh *= scalar;
		}

		template<class Ty2>
			requires std::is_arithmetic_v<Ty2>
		friend constexpr Matrix operator*(Ty2 scalar, Matrix lh)
		{
			return lh *= scalar;
		}

		template<class Ty2>
			requires std::is_arithmetic_v<Ty2>
		friend constexpr Matrix operator/(Matrix lh, Ty2 scalar)
		{
			return lh /= scalar;
		}

		template<class Ty2>
		friend constexpr bool operator==(const Matrix& lh, const Matrix<Ty2, M, N>& rh)
		{
			return std::equal(lh._values.begin(), lh._values.end(), rh._values.begin());
		}

		operator Ty() const requires (element_count == 1) { return _values[0]; }

		constexpr size_t ColumnMajorIndex(size_t row, size_t column) const noexcept { return column * row_count + row; }
		constexpr size_t RowMajorIndex(size_t row, size_t column) const noexcept { return row * column_count + column; }
	};

	export template<class Ty, size_t ElementCount>
	struct Vector : public Matrix<Ty, ElementCount, 1>
	{
		using base_type = Matrix<Ty, ElementCount, 1>;
		using base_type::_values;
		using base_type::Matrix;
		using base_type::element_count;

		constexpr Vector(const base_type& value) : base_type(value)
		{

		}

		constexpr base_type::reference operator[](size_t index) { return _values[index]; }
		constexpr base_type::const_reference operator[](size_t index) const { return _values[index]; }

		constexpr base_type::reference X() requires (element_count >= 1) { return _values[0]; }
		constexpr base_type::const_reference X() const requires (element_count >= 1) { return _values[0]; }

		constexpr base_type::reference Y() requires (element_count >= 2) { return _values[1]; }
		constexpr base_type::const_reference Y() const requires (element_count >= 2) { return _values[1]; }

		constexpr base_type::reference Z() requires (element_count >= 3) { return _values[2]; }
		constexpr base_type::const_reference Z() const requires (element_count >= 3) { return _values[2]; }

		constexpr base_type::reference W() requires (element_count >= 4) { return _values[3]; }
		constexpr base_type::const_reference W() const requires (element_count >= 4) { return _values[3]; }

		constexpr auto begin() { return _values.begin(); }
		constexpr auto end() { return _values.end(); }

		constexpr auto begin() const { return _values.begin(); }
		constexpr auto end() const { return _values.end(); }

		template<class Ty2, size_t ElementCount2>
		constexpr Vector& operator+=(const Matrix<Ty2, ElementCount2, 1>& rh)
		{
			static_cast<base_type&>(*this) += rh;
			return *this;
		}

		template<class Ty2, size_t ElementCount2>
		friend constexpr Vector operator+(Vector lh, const Matrix<Ty2, ElementCount2, 1>& rh)
		{
			return lh += rh;
		}

		template<class Ty2, size_t ElementCount>
		constexpr Vector& operator-=(const Matrix<Ty2, ElementCount, 1>& rh)
		{
			static_cast<base_type&>(*this) -= rh;
			return *this;
		}

		template<class Ty2, size_t ElementCount2>
		friend constexpr Vector operator-(Vector lh, const Matrix<Ty2, ElementCount2, 1>& rh)
		{
			return lh -= rh;
		}

		constexpr Vector operator-() const
		{
			return base_type::operator-();
		}

		template<class Ty2>
			requires std::is_arithmetic_v<Ty2>
		constexpr Vector& operator*=(Ty2 scalar)
		{
			static_cast<base_type&>(*this) *= scalar;
			return *this;
		}

		template<class Ty2>
			requires std::is_arithmetic_v<Ty2>
		friend constexpr Vector operator*(Vector lh, Ty2 scalar)
		{
			return lh *= scalar;
		}

		template<class Ty2>
			requires std::is_arithmetic_v<Ty2>
		friend constexpr Vector operator*(Ty2 scalar, Vector lh)
		{
			return lh *= scalar;
		}

		template<class Ty2>
			requires std::is_arithmetic_v<Ty2>
		constexpr Vector& operator/=(Ty2 scalar)
		{
			static_cast<base_type&>(*this) /= scalar;
			return *this;
		}

		template<class Ty2>
			requires std::is_arithmetic_v<Ty2>
		friend constexpr Vector operator/(Vector lh, Ty scalar)
		{
			return lh /= scalar;
		}

	private:
		using base_type::At;
	};

	template<class... Ty>
	Vector(Ty...) -> Vector<std::tuple_element_t<0, std::tuple<Ty...>>, sizeof...(Ty)>;

	template<class Ty1, class Ty2, size_t M, size_t N, size_t M2, bool IsConst1, bool IsConst2>
	constexpr auto operator*(RowRef<Ty1, M, N, IsConst1> lh, ColumnRef<Ty2, N, M2, IsConst2> rh)
	{
		using value_type = decltype(std::declval<Ty1>()* std::declval<Ty2>());
		value_type value = 0;
		for (size_t i = 0; i < N; i++)
		{
			value += lh[i] * rh[i];
		}
		return value;
	}

	export template<class Ty, size_t M, size_t N, bool IsConst>
	constexpr ColumnRef<Ty, M, N, IsConst>::reference ColumnRef<Ty, M, N, IsConst>::operator[](size_t index) const
	{
		return (m_matrix->At(index, m_selectedColumn));
	}

	export template<class Ty, size_t M, size_t N, bool IsConst>
	constexpr RowRef<Ty, M, N, IsConst>::reference RowRef<Ty, M, N, IsConst>::operator[](size_t index) const
	{
		return (m_matrix->At(m_selectedRow, index));
	}

	template<class Ty, size_t M, size_t N>
	constexpr Matrix<Ty, N, M> Transpose(const Matrix<Ty, M, N>& mat) noexcept
	{
		Matrix<Ty, N, M> result;
		for (size_t row = 0; row < mat.row_count; row++)
		{
			for (size_t column = 0; column < mat.column_count; column++)
			{
				result.At(column, row) = mat.At(row, column);
			}
		}
		return result;
	}

	export template<class Ty, size_t ElementCount>
	constexpr Ty Dot(const Vector<Ty, ElementCount>& lh, const Vector<Ty, ElementCount>& rh)
	{
		return std::transform_reduce(lh.begin(), lh.end(), rh.begin(), Ty{});
	}

	export template<class Ty, size_t ElementCount>
	constexpr Ty MagnitudeSquared(const Vector<Ty, ElementCount>& v)
	{
		return std::accumulate(v.begin(), v.end(), static_cast<Ty>(0), [](Ty total, Ty val)
			{
				return total + val * val;
			});
	}
	
	export template<class Ty, size_t ElementCount>
	constexpr Ty Magnitude(const Vector<Ty, ElementCount>& v)
	{
		return std::sqrt(std::accumulate(v.begin(), v.end(), static_cast<Ty>(0), [](Ty total, Ty val)
			{
				return total + val * val;
			}));
	}

	export template<class Ty, size_t ElementCount>
	constexpr Vector<Ty, ElementCount> Normalize(Vector<Ty, ElementCount> v)
	{
		std::transform(v.begin(), v.end(), v.begin(), [dividor = Magnitude(v)](auto& e)
		{
			return e /= dividor;
		});
		return v;
	}

	export template<class Ty>
	constexpr Matrix<Ty, 4, 4> TransformMatrix(const Vector<Ty, 3>& vector)
	{
		return
		{
			1, 0, 0, vector.X(),
			0, 1, 0, vector.Y(),
			0, 0, 1, vector.Z(),
			0, 0, 0, 1
		};
	}
	export namespace Aliases
	{
		using Vector2 = Vector<float, 2>;
		using Vector3 = Vector<float, 3>;
		using Vector4 = Vector<float, 4>;

		using iVector2 = Vector<int, 2>;
		using iVector3 = Vector<int, 3>;
		using iVector4 = Vector<int, 4>;

		using uVector2 = Vector<unsigned int, 2>;
		using uVector3 = Vector<unsigned int, 3>;
		using uVector4 = Vector<unsigned int, 4>;

		using dVector2 = Vector<double, 2>;
		using dVector3 = Vector<double, 3>;
		using dVector4 = Vector<double, 4>;

		using i8Vector2 = Vector<std::int8_t, 2>;
		using i8Vector3 = Vector<std::int8_t, 3>;
		using i8Vector4 = Vector<std::int8_t, 4>;

		using u8Vector2 = Vector<std::uint8_t, 2>;
		using u8Vector3 = Vector<std::uint8_t, 3>;
		using u8Vector4 = Vector<std::uint8_t, 4>;

		using i16Vector2 = Vector<std::int16_t, 2>;
		using i16Vector3 = Vector<std::int16_t, 3>;
		using i16Vector4 = Vector<std::int16_t, 4>;

		using u16Vector2 = Vector<std::uint16_t, 2>;
		using u16Vector3 = Vector<std::uint16_t, 3>;
		using u16Vector4 = Vector<std::uint16_t, 4>;

		using Matrix2x2 = Matrix<float, 2, 2>;
		using Matrix3x3 = Matrix<float, 3, 3>;
		using Matrix3x4 = Matrix<float, 3, 4>;
		using Matrix4x4 = Matrix<float, 4, 4>;
	}
}

#pragma warning(default:4244)