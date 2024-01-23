#pragma once
#include <concepts>


namespace InsanityEngine::Math::Concepts
{
    template<class T>
    concept Arithmetic = std::is_arithmetic_v<T>;

    template<class T>
    concept FloatingPoint = std::is_floating_point_v<T>;

}