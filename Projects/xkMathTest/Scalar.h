#pragma once

#include "MathConcepts.h"

namespace InsanityEngine::Math::Scalar
{
    template<Concepts::Arithmetic T>
    struct Scalar
    {
    public:
        T value = 0;

    public:
        constexpr Scalar() = default;
        explicit constexpr Scalar(T value) : value(value) {}


    };

}

namespace InsanityEngine::Math::Types
{
    using Math::Scalar::Scalar;
}