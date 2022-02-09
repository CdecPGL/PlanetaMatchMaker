#pragma once

#include <type_traits>

namespace pgl {
	namespace enum_concept {
		/** Example:
		enum class Flag1 : int {
			a = 1,
			b = 2
		};

		namespace enum_concept{
			template<> struct has_bitwise_operators<Flag1> : std::true_type {}; // & &= | |= ^ ^= ~
			template<> struct has_and_or_operators<Flag2> : std::true_type {}; // & &= | |=
		}
		*/
		template<typename T>
		struct has_bitwise_operators : std::false_type {};
		template<typename T>
		struct has_and_or_operators : has_bitwise_operators<T> {};

		template<typename T>
		concept bitwise_operatable = has_bitwise_operators<T>::value;

		template<typename T>
		concept bitwise_and_or_operatable = has_and_or_operators<T>::value;

		template<typename T>
		concept enum_compatible= std::is_enum_v<T>;
	}

	namespace detail {
		template<enum_concept::enum_compatible T>
		constexpr auto underlying_cast(T e) { return static_cast<std::underlying_type_t<T>>(e); }
	}
	
	template<enum_concept::bitwise_and_or_operatable T>
	constexpr T operator&(T l, T r) { return static_cast<T>(detail::underlying_cast(l) & detail::underlying_cast(r)); }
	template<enum_concept::bitwise_and_or_operatable T>
	T & operator&=(T & l, T r) {
		l = static_cast<T>(detail::underlying_cast(l) & detail::underlying_cast(r));
		return l;
	}
	
	template<enum_concept::bitwise_and_or_operatable T>
	constexpr T operator|(T l, T r) { return static_cast<T>(detail::underlying_cast(l) | detail::underlying_cast(r)); }
	template<enum_concept::bitwise_and_or_operatable T>
	T & operator|=(T & l, T r) {
		l = static_cast<T>(detail::underlying_cast(l) | detail::underlying_cast(r));
		return l;
	}
	
	template<enum_concept::bitwise_operatable T>
	constexpr T operator^(T l, T r) { return static_cast<T>(detail::underlying_cast(l) ^ detail::underlying_cast(r)); }
	template<enum_concept::bitwise_operatable T>
	T & operator^=(T & l, T r) {
		l = static_cast<T>(detail::underlying_cast(l) ^ detail::underlying_cast(r));
		return l;
	}
	
	template<enum_concept::bitwise_operatable T>
	constexpr T operator~(T op) { return static_cast<T>(~detail::underlying_cast(op)); }
}