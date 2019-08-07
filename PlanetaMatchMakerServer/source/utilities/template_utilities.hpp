#pragma once

#include <memory>
#include <type_traits>

namespace pgl {
	struct is_array_container_impl {
		template <class T>
		static auto check(T&& x) -> decltype(x.operator[](std::declval<size_t>()), x.size(), std::true_type{});

		template <class T>
		static auto check(...) -> std::false_type;
	};

	template <class T>
	struct is_array_container final : decltype(is_array_container_impl::check<T>(std::declval<T>())) {};

	template <class T>
	constexpr bool is_array_container_v = is_array_container<T>::value;

	template <typename T>
	struct is_shared_ptr : std::false_type {};

	template <typename T>
	struct is_shared_ptr<std::shared_ptr<T>> : std::true_type {};

	template <typename T>
	constexpr bool is_shared_ptr_v = is_shared_ptr<T>::value;
}
