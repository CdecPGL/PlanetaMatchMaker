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
	}

	namespace type_traits {
		template<bool Con> using concept_t = typename std::enable_if<Con, std::nullptr_t>::type;
	}

	namespace detail {
		using namespace type_traits;
		template<typename T, concept_t<std::is_enum<T>::value> = nullptr>
		constexpr std::underlying_type_t<T> underlying_cast(T e) { return static_cast<std::underlying_type_t<T>>(e); }
	}
	
	template<typename T, type_traits::concept_t<enum_concept::has_and_or_operators<T>::value> = nullptr>
	constexpr T operator&(T l, T r) { return static_cast<T>(detail::underlying_cast(l) & detail::underlying_cast(r)); }
	template<typename T, type_traits::concept_t<enum_concept::has_and_or_operators<T>::value> = nullptr>
	T & operator&=(T & l, T r) {
		l = static_cast<T>(detail::underlying_cast(l) & detail::underlying_cast(r));
		return l;
	}
	
	template<typename T, type_traits::concept_t<enum_concept::has_and_or_operators<T>::value> = nullptr>
	constexpr T operator|(T l, T r) { return static_cast<T>(detail::underlying_cast(l) | detail::underlying_cast(r)); }
	template<typename T, type_traits::concept_t<enum_concept::has_and_or_operators<T>::value> = nullptr>
	T & operator|=(T & l, T r) {
		l = static_cast<T>(detail::underlying_cast(l) | detail::underlying_cast(r));
		return l;
	}
	
	template<typename T, type_traits::concept_t<enum_concept::has_bitwise_operators<T>::value> = nullptr>
	constexpr T operator^(T l, T r) { return static_cast<T>(detail::underlying_cast(l) ^ detail::underlying_cast(r)); }
	template<typename T, type_traits::concept_t<enum_concept::has_bitwise_operators<T>::value> = nullptr>
	T & operator^=(T & l, T r) {
		l = static_cast<T>(detail::underlying_cast(l) ^ detail::underlying_cast(r));
		return l;
	}
	
	template<typename T, type_traits::concept_t<enum_concept::has_bitwise_operators<T>::value> = nullptr>
	constexpr T operator~(T op) { return static_cast<T>(~detail::underlying_cast(op)); }
}