#pragma once
#include "MathConcepts.h"
#include "Matrix.h"
#include "Trigonometry.h"
#include <array>
#include <algorithm>

namespace InsanityEngine::Math::Quaternion
{
    using Math::Vector::Vector;
    using Math::Matrix::Matrix;
    using Math::Trigonometry::Radians;
    using Math::Trigonometry::Degrees;

    template<Concepts::FloatingPoint T>
    struct Quaternion
    {
    public:
        using value_type = T;
        using data_type = Vector<T, 4>;
        using axis_type = Vector<T, 3>;

    private:
        data_type data;

    public:
        constexpr Quaternion() :
            data({ 0, 0, 0, 1 })
        {

        }
        constexpr Quaternion(value_type x, value_type y, value_type z, value_type w) :
            data({ x, y, z, w })
        {

        }
        constexpr Quaternion(Degrees<value_type> x, Degrees<value_type> y, Degrees<value_type> z) :
            Quaternion(x.ToRadians(), y.ToRadians(), z.ToRadians())
        {
        }

        //Referenced equations https://www.euclideanspace.com/maths/geometry/rotations/conversions/eulerToQuaternion/index.htm
        constexpr Quaternion(Radians<value_type> x, Radians<value_type> y, Radians<value_type> z)
        {
            T cx = cos(x.Data() / 2);
            T cy = cos(y.Data() / 2);
            T cz = cos(z.Data() / 2);
            T sx = sin(x.Data() / 2);
            T sy = sin(y.Data() / 2);
            T sz = sin(z.Data() / 2);

            this->w() = (cx * cy * cz) - (sx * sy * sz);
            this->x() = (cx * sy * sz) + (sx * cy * cz);
            this->y() = (cx * sy * cz) + (sx * cy * sz);
            this->z() = (cx * cy * sz) - (sx * sy * cz);
        }

        constexpr Quaternion(axis_type axis, Degrees<value_type> angle) :
            Quaternion(axis, angle.ToRadians())
        {

        }
        constexpr Quaternion(axis_type axis, Radians<value_type> angle)
        {
            angle /= Radians<value_type>(static_cast<value_type>(2));
            value_type sinAngle = sin(angle.Data());
            w() = cos(angle.Data());
            x() = axis.x() * sinAngle;
            y() = axis.y() * sinAngle;
            z() = axis.z() * sinAngle;
        }

    public:
        constexpr Quaternion& operator+=(const Quaternion& rh)
        {
            std::transform(data.begin(), data.end(), rh.data.begin(), data.begin(), std::plus{});
            return *this;
        }
        constexpr Quaternion& operator-=(const Quaternion& rh)
        {
            std::transform(data.begin(), data.end(), rh.data.begin(), data.begin(), std::minus{});
            return *this;
        }
        constexpr Quaternion& operator*=(const Quaternion& rh)
        {
            data = rh.ToMatrix() * data;

            return *this;
        }
        constexpr Quaternion operator-() const
        {
            Quaternion copy;
            copy.data = data;
            copy.w() *= -1;
            return copy;
        }
        //constexpr Quaternion& operator/=(const Quaternion& rh)
        //{
        //    //std::transform(data.begin(), data.end(), rh.data.begin(), data.begin(), std::minus{});
        //    return *this;
        //}

        friend constexpr Quaternion operator+(Quaternion lh, const Quaternion& rh)
        {
            return (lh += rh);
        }
        friend constexpr Quaternion operator-(Quaternion lh, const Quaternion& rh)
        {
            return (lh -= rh);
        }
        friend constexpr Quaternion operator*(Quaternion lh, const Quaternion& rh)
        {
            return (lh *= rh);
        }
        //friend constexpr Quaternion operator/(Quaternion lh, const Quaternion& rh)
        //{
        //    return (lh /= rh);
        //}

        friend constexpr bool operator==(const Quaternion& lh, const Quaternion& rh) = default;
        friend constexpr bool operator!=(const Quaternion& lh, const Quaternion& rh) = default;

        constexpr value_type& operator[](size_t index) { return data[index]; }
        constexpr const value_type& operator[](size_t index) const { return data[index]; }

    public:
        constexpr value_type& x() { return data[0]; }
        constexpr value_type& y() { return data[1]; }
        constexpr value_type& z() { return data[2]; }
        constexpr value_type& w() { return data[3]; }

        constexpr const value_type& x() const { return data[0]; }
        constexpr const value_type& y() const { return data[1]; }
        constexpr const value_type& z() const { return data[2]; }
        constexpr const value_type& w() const { return data[3]; }

        constexpr auto begin() { return data.begin(); }
        constexpr auto rbegin() { return data.rbegin(); }
        constexpr auto end() { return data.end(); }
        constexpr auto rend() { return data.rend(); }

        constexpr auto begin() const { return data.begin(); }
        constexpr auto rbegin() const { return data.rbegin(); }
        constexpr auto end() const { return data.end(); }
        constexpr auto rend() const { return data.rend(); }

    public:
        void Normalize()
        {
            data.Normalize();
        }

        Vector<value_type, 3> ToEulerDegrees() const
        {
            auto radians = ToEulerRadians();

            return
            {
                 Math::Trigonometry::ToDegrees(radians.x()),
                 Math::Trigonometry::ToDegrees(radians.y()),
                 Math::Trigonometry::ToDegrees(radians.z())
            };
        }

        //Referenced equations https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
        Vector<value_type, 3> ToEulerRadians() const
        {
            value_type yValue = 2 * (w() * y() - z() * x());
            return
            {
                -std::atan2(2 * (w() * x() + y() * z()), 1 - 2 * (x() * x() + y() * y())),
                (std::abs(yValue) >= 1) ?  std::copysign(Math::Constants::pi<value_type> / 2, yValue) : -std::asin(yValue),
                -std::atan2(2 * (w() * z() + y() * x()),  1 - 2 * (z() * z() + y() * y()))
            };      
        }

        //Referenced equations https://www.euclideanspace.com/maths/algebra/realNormedAlgebra/quaternions/code/index.htm
        //The matrix form of the quaternion, if you're looking to make an actually rotation matrix, use ToRotationMatrix()
        Matrix<value_type, 4, 4> ToMatrix() const
        {
            return
            {
                 w(),  z(), -y(), x(),
                -z(),  w(),  x(), y(),
                 y(), -x(),  w(), z(),
                -x(), -y(), -z(), w()
            };
        }


        //Referenced equations https://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToMatrix/index.htm
        //Creates a Rotation Matrix from the quaternion, if you're looking to view the matrix form of the quaternion, use ToMatrix()
        Matrix<value_type, 4, 4> ToRotationMatrix() const
        {
            Matrix<float, 4, 4> lh =
            {
                 w(),  z(), -y(), x(),
                -z(),  w(),  x(), y(),
                 y(), -x(),  w(), z(),
                -x(), -y(), -z(), w()
            };
            Matrix<float, 4, 4> rh =
            {
                 w(),  z(), -y(), -x(),
                -z(),  w(),  x(), -y(),
                 y(), -x(),  w(), -z(),
                 x(),  y(),  z(),  w()
            };

            return lh * rh;
        }
    };

}

namespace InsanityEngine::Math::Types
{
    using Math::Quaternion::Quaternion;
}