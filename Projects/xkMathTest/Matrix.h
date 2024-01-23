#pragma once
#include "MathConcepts.h"
#include "Scalar.h"
#include <algorithm>
#include <array>
#include <numeric>
#include <math.h>
#include <functional>


namespace InsanityEngine::Math
{
    namespace Matrix
    {
        template<Concepts::Arithmetic T, size_t Rows, size_t Columns>
        struct Matrix;


        template<Concepts::Arithmetic T, size_t Rows, size_t Columns>
        constexpr Matrix<T, Rows, Columns> Identity() requires(Rows == Columns);

        template<Concepts::Arithmetic T, size_t Rows, size_t Columns>
        constexpr void Transpose(Matrix<T, Rows, Columns>& mat) requires(Rows == Columns);

        template<Concepts::Arithmetic T, size_t Rows, size_t Columns>
        constexpr Matrix<T, Columns, Rows> TransposeCopy(const Matrix<T, Rows, Columns>& mat);


        template<Concepts::Arithmetic T, size_t Rows, size_t Columns>
        struct Matrix
        {
        public:
            static constexpr size_t row_count = Rows;
            static constexpr size_t column_count = Columns;

            using value_type = T;

            using row_type = std::array<value_type, column_count>;

            using data_type = std::array<row_type, row_count>;

        private:
            //Compatable matrix sizes that can be multiplied
            template<size_t mulColumns>
            using matrix_mul_type = Matrix<value_type, column_count, mulColumns>;

            //Results of a matrix multiplication
            template<size_t mulColumns>
            using matrix_mul_result_type = Matrix<value_type, row_count, mulColumns>;

        public:
            using matrix_type = Matrix;
            using matrix_transpose_type = matrix_mul_type<row_count>;


        public:
            data_type data = { {} };

        public:
            constexpr static matrix_type Identity() requires(column_count == row_count)
            {
                return Math::Matrix::Identity<value_type, row_count, column_count>();
            }

            constexpr matrix_type& Transpose() requires(column_count == row_count)
            {
                return Math::Matrix::Transpose(*this);
            }

            constexpr matrix_transpose_type TransposeCopy() const
            {
                return Math::Matrix::TransposeCopy(*this);
            }

            constexpr value_type& At(size_t row, size_t column)
            {
                return data[row][column];
            }

            constexpr const value_type& At(size_t row, size_t column) const
            {
                return data[row][column];
            }

        public:
            constexpr matrix_type& operator+=(const matrix_type& rh)
            {
                for(size_t y = 0; y < row_count; y++)
                {
                    for(size_t x = 0; x < column_count; x++)
                    {
                        At(y, x) += rh(y, x);
                    }
                }

                return *this;
            }
            constexpr matrix_type& operator-=(const matrix_type& rh)
            {
                for(size_t y = 0; y < row_count; y++)
                {
                    for(size_t x = 0; x < column_count; x++)
                    {
                        At(y, x) -= rh(y, x);
                    }
                }

                return *this;
            }

            template<size_t mulColumns>
            friend constexpr matrix_mul_result_type<mulColumns> operator*(const matrix_type& lh, const matrix_mul_type<mulColumns>& rh)
            {
                matrix_mul_result_type<mulColumns> mat;
                for(size_t y = 0; y < mat.row_count; y++)
                {
                    for(size_t x = 0; x < mat.column_count; x++)
                    {
                        for(size_t column_select = 0; column_select < lh.column_count; column_select++)
                        {
                            mat(y, x) += lh(y, column_select) * rh(column_select, x);
                        }
                    }
                }

                return mat;
            }

            constexpr matrix_type& operator*=(const matrix_type& rh) requires(column_count == row_count)
            {
                *this = *this * rh;
                return (*this);
            }

            constexpr matrix_type& operator*=(float scale)
            {
                for(size_t y = 0; y < row_count; y++)
                {
                    for(size_t x = 0; x < column_count; x++)
                    {
                        At(y, x) *= scale;
                    }
                }

                return *this;
            }

            constexpr matrix_type& operator/=(float scale)
            {
                for(size_t y = 0; y < row_count; y++)
                {
                    for(size_t x = 0; x < column_count; x++)
                    {
                        At(y, x) /= scale;
                    }
                }

                return *this;
            }

            friend constexpr matrix_type operator+(matrix_type lh, const matrix_type& rh)
            {
                return lh += rh;
            }

            friend constexpr matrix_type operator-(matrix_type lh, const matrix_type& rh)
            {
                return lh -= rh;
            }

            friend constexpr matrix_type operator*(matrix_type lh, float scale)
            {
                return lh *= scale;
            }

            friend constexpr matrix_type operator/(matrix_type lh, float scale)
            {
                return lh /= scale;
            }

            friend constexpr bool operator==(const matrix_type& lh, const matrix_type& rh)
            {
                return lh.data == rh.data;
            }

            friend constexpr bool operator!=(const matrix_type& lh, const matrix_type& rh)
            {
                return !(lh == rh);
            }

            constexpr value_type& operator()(size_t row, size_t column)
            {
                return At(row, column);
            }

            constexpr const value_type& operator()(size_t row, size_t column) const
            {
                return At(row, column);
            }
        };



        template<Concepts::Arithmetic T, size_t Rows, size_t Columns>
        constexpr Matrix<T, Rows, Columns> Identity() requires(Rows == Columns)
        {
            Matrix<T, Rows, Columns> mat;

            for(size_t x = 0, y = 0; x < Rows; x++, y++)
            {
                mat(y, x) = 1;
            }

            return mat;
        }

        template<Concepts::Arithmetic T, size_t Rows, size_t Columns>
        constexpr void Transpose(Matrix<T, Rows, Columns>& mat) requires(Rows == Columns)
        {
            return (mat = TransposeCopy(mat));
        }

        template<Concepts::Arithmetic T, size_t Rows, size_t Columns>
        constexpr Matrix<T, Columns, Rows> TransposeCopy(const Matrix<T, Rows, Columns>& mat)
        {
            Matrix<T, Columns, Rows> copy;

            for(size_t y = 0; y < Rows; y++)
            {
                for(size_t x = 0; x < Columns; x++)
                {
                    copy(x, y) = mat(y, x);
                }
            }

            return copy;
        }
    }


    namespace Vector
    {
        template<Concepts::Arithmetic T, size_t Size>
        struct Vector;

        using Math::Scalar::Scalar;

        template<Concepts::Arithmetic T, size_t Size>
        constexpr T MagnitudeSquared(const Vector<T, Size>& v);

        template<Concepts::Arithmetic T, size_t Size>
        T Magnitude(const Vector<T, Size>& v);

        template<Concepts::Arithmetic T, size_t Size>
        Vector<T, Size>& Normalize(Vector<T, Size>& v);

        template<Concepts::Arithmetic T, size_t Size>
        Vector<T, Size> NormalizeCopy(Vector<T, Size> lh);

        template<Concepts::Arithmetic T, size_t Size>
        constexpr T Dot(const Vector<T, Size>& lh, const Vector<T, Size>& rh);

        template<Concepts::Arithmetic T, size_t Size>
        constexpr Vector<T, Size> Cross(const Vector<T, Size>& lh, const Vector<T, Size>& rh) requires (Size == 3);

        template<Concepts::Arithmetic T, size_t Size>
        struct Vector
        {
        public:
            using value_type = T;
            static constexpr size_t size = Size;
            using data_t = std::array<value_type, size>;

        public:
            data_t data = {};

        public:
            constexpr Vector() = default;
            constexpr Vector(const Vector& other) = default;
            constexpr Vector(Vector&& other) noexcept = default;

            constexpr Vector(const data_t& data) : data(data) {}

            template<Concepts::Arithmetic ConversionT>
            constexpr Vector(Scalar<ConversionT> scalar) { std::fill(data.begin(), data.end(), static_cast<value_type>(scalar.value)); }

            template<Concepts::Arithmetic ConversionT, size_t SizeConversion>
            constexpr Vector(const Vector<ConversionT, SizeConversion>& other)
            {
                if constexpr(SizeConversion <= Size)
                {
                    for(size_t i = 0; i < SizeConversion; i++)
                    {
                        data[i] = static_cast<value_type>(other[i]);
                    }
                }
                else
                {
                    for(size_t i = 0; i < size; i++)
                    {
                        data[i] = static_cast<value_type>(other[i]);
                    }
                }
            }

            template<Concepts::Arithmetic... ConversionT>
            constexpr Vector(ConversionT... values) requires (sizeof...(ConversionT) == size) : data({ static_cast<value_type>(values)... }) {}

            constexpr Vector(Vector<value_type, 2> v, value_type z) requires (size == 3) : data({ v.x(), v.y(), z }) {}
            constexpr Vector(Vector<value_type, 2> v, value_type z, value_type w) requires (size == 4) : data({ v.x(), v.y(), z, w }) {}
            constexpr Vector(Vector<value_type, 3> v, value_type w) requires (size == 4) : data({ v.x(), v.y(), v.z(), w }) {}

        public:
            constexpr value_type& x() requires (size >= 1) { return data[0]; }
            constexpr value_type& y() requires (size >= 2) { return data[1]; }
            constexpr value_type& z() requires (size >= 3) { return data[2]; }
            constexpr value_type& w() requires (size >= 4) { return data[3]; }

            constexpr const value_type& x() const requires (size >= 1) { return data[0]; }
            constexpr const value_type& y() const requires (size >= 2) { return data[1]; }
            constexpr const value_type& z() const requires (size >= 3) { return data[2]; }
            constexpr const value_type& w() const requires (size >= 4) { return data[3]; }

            constexpr auto begin() { return data.begin(); }
            constexpr auto rbegin() { return data.rbegin(); }
            constexpr auto end() { return data.end(); }
            constexpr auto rend() { return data.rend(); }

            constexpr auto begin() const { return data.begin(); }
            constexpr auto rbegin() const { return data.rbegin(); }
            constexpr auto end() const { return data.end(); }
            constexpr auto rend() const { return data.rend(); }

        public:
            value_type Magnitude() const { return Math::Vector::Magnitude(*this); }
            value_type MagnitudeSquared() const { return Math::Vector::MagnitudeSquared(*this); }

            Vector& Normalize() { return Math::Vector::Normalize(*this); }
            Vector NormalizeCopy() const { return Math::Vector::NormalizeCopy(*this); }

        public:

            friend constexpr Vector<value_type, size>& operator+=(Vector<value_type, size>& lh, const Vector<value_type, size>& rh)
            {
                std::transform(lh.begin(), lh.end(), rh.begin(), lh.begin(), std::plus{});
                return lh;
            }
            friend constexpr Vector<value_type, size>& operator-=(Vector<value_type, size>& lh, const Vector<value_type, size>& rh)
            {
                std::transform(lh.begin(), lh.end(), rh.begin(), lh.begin(), std::minus{});
                return lh;
            }
            friend constexpr Vector<value_type, size>& operator*=(Vector<value_type, size>& lh, const Vector<value_type, size>& rh)
            {
                std::transform(lh.begin(), lh.end(), rh.begin(), lh.begin(), std::multiplies{});
                return lh;
            }
            friend constexpr Vector<value_type, size>& operator/=(Vector<value_type, size>& lh, const Vector<value_type, size>& rh)
            {
                std::transform(lh.begin(), lh.end(), rh.begin(), lh.begin(), std::divides{});
                return lh;
            }

            friend constexpr Vector<value_type, size>& operator*=(Vector<value_type, size>& vector, const value_type& scalar)
            {
                for(value_type& value : vector)
                    value *= scalar;
                return vector;
            }
            friend constexpr Vector<value_type, size>& operator/=(Vector<value_type, size>& vector, const value_type& scalar)
            {
                for(value_type& value : vector)
                    value /= scalar;
                return vector;
            }

            friend constexpr Vector<value_type, size> operator+(Vector<value_type, size> lh, const Vector<value_type, size>& rh) { return lh += rh; }
            friend constexpr Vector<value_type, size> operator-(Vector<value_type, size> lh, const Vector<value_type, size>& rh) { return lh -= rh; }
            friend constexpr Vector<value_type, size> operator*(Vector<value_type, size> lh, const Vector<value_type, size>& rh) { return lh *= rh; }
            friend constexpr Vector<value_type, size> operator/(Vector<value_type, size> lh, const Vector<value_type, size>& rh) { return lh /= rh; }
            friend constexpr Vector<value_type, size> operator-(Vector<value_type, size> lh) { return lh *= -1; }

            friend constexpr Vector<value_type, size> operator*(Vector<value_type, size> vector, const value_type& scalar) { return vector *= scalar; }
            friend constexpr Vector<value_type, size> operator/(Vector<value_type, size> vector, const value_type& scalar) { return vector /= scalar; }

            friend constexpr bool operator==(const Vector<value_type, size>& lh, const Vector<T, size>& rh)
            {
                return std::equal(lh.begin(), lh.end(), rh.begin());
            }
            friend constexpr bool operator!=(const Vector<value_type, size>& lh, const Vector<T, size>& rh)
            {
                return !(lh == rh);
            }

            constexpr Vector& operator=(const Vector& other) = default;
            constexpr Vector& operator=(Vector&& other) = default;

            template<Concepts::Arithmetic ConversionT>
            constexpr Vector& operator=(const Scalar<ConversionT> scalar)
            {
                std::fill(data.begin(), data.end(), static_cast<value_type>(scalar.value));
                return *this;
            }

            constexpr value_type& operator[](size_t index) { return data[index]; }
            constexpr const value_type& operator[](size_t index) const { return data[index]; }

            template<Concepts::Arithmetic ConversionT, size_t Size_Conversion>
            constexpr explicit operator Vector<ConversionT, Size_Conversion>() const { return Vector<ConversionT, Size_Conversion>(*this); }
        };


        template<Concepts::Arithmetic T, size_t Rows, size_t Columns>
        Vector<T, Rows> operator*(const Matrix::Matrix<T, Rows, Columns>& lh, const Vector<T, Rows>& rh)
        {
            Vector<T, Rows> vec;

            for(size_t x = 0; x < Columns; x++)
            {
                for(size_t y = 0; y < Rows; y++)
                {
                    vec[x] += lh.At(x, y) * rh[y];
                }
            }

            return vec;
        }


        template<Concepts::Arithmetic T, size_t Size>
        constexpr T MagnitudeSquared(const Vector<T, Size>& v)
        {
            return std::inner_product(v.begin(), v.end(), v.begin(), static_cast<T>(0), std::plus{}, std::multiplies{});
        }

        template<Concepts::Arithmetic T, size_t Size>
        T Magnitude(const Vector<T, Size>& v)
        {
            return std::sqrt(MagnitudeSquared(v));
        }

        template<Concepts::Arithmetic T, size_t Size>
        Vector<T, Size>& Normalize(Vector<T, Size>& v)
        {
            return v /= Magnitude(v);
        }

        template<Concepts::Arithmetic T, size_t Size>
        Vector<T, Size> NormalizeCopy(Vector<T, Size> lh)
        {
            return Normalize(lh);
        }

        template<Concepts::Arithmetic T, size_t Size>
        constexpr T Dot(const Vector<T, Size>& lh, const Vector<T, Size>& rh)
        {
            return std::inner_product(lh.begin(), lh.end(), rh.begin(), static_cast<T>(0), std::plus{}, std::multiplies{});
        }

        template<Concepts::Arithmetic T, size_t Size>
        constexpr Vector<T, Size> Cross(const Vector<T, Size>& lh, const Vector<T, Size>& rh) requires (Size == 3)
        {
            Vector<T, Size> v;

            constexpr size_t x = 0;
            constexpr size_t y = 1;
            constexpr size_t z = 2;

            v[x] = lh[y] * rh[z] - lh[z] * rh[y];
            v[y] = lh[z] * rh[x] - lh[x] * rh[z];
            v[z] = lh[x] * rh[y] - lh[y] * rh[x];

            return v;
        }
    }


    namespace Types
    {
        using Math::Matrix::Matrix;
        using Matrix4x4f = Matrix<float, 4, 4>;



        using Math::Vector::Vector;
        using Vector2f = Vector<float, 2>;
        using Vector3f = Vector<float, 3>;
        using Vector4f = Vector<float, 4>;

        using Vector2d = Vector<double, 2>;
        using Vector3d = Vector<double, 3>;
        using Vector4d = Vector<double, 4>;

        using Vector2ld = Vector<long double, 2>;
        using Vector3ld = Vector<long double, 3>;
        using Vector4ld = Vector<long double, 4>;

        using Vector2i = Vector<int, 2>;
        using Vector3i = Vector<int, 3>;
        using Vector4i = Vector<int, 4>;

        using Vector2ui = Vector<unsigned int, 2>;
        using Vector3ui = Vector<unsigned int, 3>;
        using Vector4ui = Vector<unsigned int, 4>;

        using Vector2l = Vector<long long, 2>;
        using Vector3l = Vector<long long, 3>;
        using Vector4l = Vector<long long, 4>;

        using Vector2ul = Vector<unsigned long long, 2>;
        using Vector3ul = Vector<unsigned long long, 3>;
        using Vector4ul = Vector<unsigned long long, 4>;
    }

}