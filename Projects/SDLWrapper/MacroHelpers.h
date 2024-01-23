#pragma once
#include <type_traits>


#define DECLARE_ENUM_BIT_FLAGS(type) \
export constexpr type operator|(type lh, type rh) noexcept\
{\
	return type{ static_cast<std::underlying_type_t<type>>(lh) | static_cast<std::underlying_type_t<type>>(rh) };\
}