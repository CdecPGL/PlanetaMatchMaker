/*
The MIT License (MIT)

Copyright (c) 2019 Cdec

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once

#include <type_traits>
#include <tuple>

#include <boost/static_string/static_string.hpp>

namespace minimal_serializer {
	/**
	 * @brief Remove const, volatile and reference
	 */
	template <typename T>
	using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;

	struct is_tuple_like_impl final {
		template <typename T>
		static auto check(T&& x) -> decltype(std::tuple_size<T>::value, std::true_type{});

		template <typename T>
		static auto check(...) -> std::false_type;
	};

	/**
	 * @brief Whether the type can be treated as tuple.
	 */
	template <typename T>
	constexpr bool is_tuple_like_v = decltype(is_tuple_like_impl::check<T>(std::declval<T>()))::value;

	template <typename C, typename V>
	auto member_variable_pointer_t_impl(V C::* p) -> std::pair<C, V>;

	/**
	 * @brief A type of class indicated by member variable pointer.
	 */
	template <auto P>
	using member_variable_pointer_class_t = typename decltype(minimal_serializer::member_variable_pointer_t_impl(P)
	)::first_type;

	/**
	 * @brief A type of variable indicated by member variable pointer.
	 */
	template <auto P>
	using member_variable_pointer_variable_t = typename decltype(minimal_serializer::member_variable_pointer_t_impl(P)
	)::second_type;

	/**
	 * @brief A container to hold member variable pointer which are serialize target.
	 * @tparam FirstPtr A member function pointer which is serialize target.
	 * @tparam RestPtrs Member function pointers which are serialize target.
	 */
	template <auto FirstPtr, auto... RestPtrs>
	class serialize_target_container final {
		static_assert(std::is_member_object_pointer_v<decltype(FirstPtr)> && (std::is_member_object_pointer_v<decltype(
						RestPtrs)> && ...), "FirstPtr and RestPtrs must be member function pointer.");
		static_assert(
			(std::is_same_v<
				member_variable_pointer_class_t<FirstPtr>,
				member_variable_pointer_class_t<RestPtrs>
			> && ...),
			"All pointers must be in same class."
		);

	public:
		using ptr_types = std::tuple<decltype(FirstPtr), decltype(RestPtrs)...>;
		const ptr_types ptrs = ptr_types(FirstPtr, RestPtrs...);
		using types = std::tuple<member_variable_pointer_variable_t<FirstPtr>, member_variable_pointer_variable_t<
									RestPtrs>...>;
		using class_type = member_variable_pointer_class_t<FirstPtr>;
		using const_reference_types = std::tuple<
			std::add_lvalue_reference_t<std::add_const_t<member_variable_pointer_variable_t<FirstPtr>>>,
			std::add_lvalue_reference_t<std::add_const_t<member_variable_pointer_variable_t<RestPtrs>>>...>;
		using reference_types = std::tuple<std::add_lvalue_reference_t<member_variable_pointer_variable_t<FirstPtr>>,
											std::add_lvalue_reference_t<member_variable_pointer_variable_t<RestPtrs>>...
		>;

		static const_reference_types get_const_reference_tuple(const class_type& obj) {
			return std::tie(obj.*FirstPtr, obj.*RestPtrs...);
		}

		static reference_types get_reference_tuple(class_type& obj) {
			return std::tie(obj.*FirstPtr, obj.*RestPtrs...);
		}
	};

	/**
	 * @brief A template meta function to get serialize targets type alias defined in the type.
	 * This returns T::serialize_targets if it is defined, or returns void if it is not.
	 */
	struct get_serialize_targets_impl final {
		template <typename T>
		static auto check(T&& x) -> typename T::serialize_targets;

		template <typename T>
		static auto check(...) -> void;
	};

	/**
	 * @brief A serialize target definition. The type will be void if T does not have serialize_targets type alias. This struct may be specialized for custom class.
	 * @tparam T A target type.
	 */
	template <typename T>
	struct serialize_targets {
		using type = decltype(get_serialize_targets_impl::check<T>(std::declval<T>()));
	};

	/**
	 * @brief A serialize target definition type alias. The type will be void if T does not have serialize_targets type alias and does not have specialized serialize_targets struct template.
	 * @tparam T A target type.
	 * @todo Check the type of serialize_targets<T>::type is serialize_target_container or void.
	 */
	template <typename T>
	using serialize_targets_t = typename serialize_targets<T>::type;

	/**
	 * @brief Whether the type has serialize target definition.
	 * @tparam T A target type.
	 */
	template <typename T>
	constexpr bool has_serialize_targets_definition_v = !std::is_same_v<serialize_targets_t<T>, void>;

	struct is_serializable_boost_static_string_impl final {
		template <typename T>
#ifdef BOOST_STATIC_STRING_CPP20
		[[deprecated("Use boost::static_strings::static_u8string instead.")]]
#endif
		static auto check(T&& x) -> std::enable_if_t<
			std::is_same_v<T, boost::static_strings::static_string<T::static_capacity>>,
			std::true_type
		>;

#ifdef BOOST_STATIC_STRING_CPP20
		template <typename T>
		static auto check(T&& x) -> std::enable_if_t<
			std::is_same_v<T, boost::static_strings::static_u8string<T::static_capacity>>,
			std::true_type
		>;
#endif

		template <typename T>
		static auto check(...) -> std::false_type;
	};

	/**
	 * @brief A template alias to check whether the type is boost static string which is serializable with minimal serializer.
	 * @tparam T A target type.
	 */
	template <typename T>
	constexpr bool is_serializable_boost_static_string_v =
		decltype(is_serializable_boost_static_string_impl::check<T>(std::declval<T>()))::value;

	// float is available in boost.endian if Boost Library version >= 1.74.0
#if BOOST_VERSION >= 107400
	template <typename T>
	constexpr bool is_serializable_builtin_type_v =
		std::is_same_v<T, int8_t> ||
		std::is_same_v<T, uint8_t> ||
		std::is_same_v<T, int16_t> ||
		std::is_same_v<T, uint16_t> ||
		std::is_same_v<T, int32_t> ||
		std::is_same_v<T, uint32_t> ||
		std::is_same_v<T, int64_t> ||
		std::is_same_v<T, uint64_t> ||
		std::is_same_v<T, bool> ||
		(std::is_same_v<T, float> && sizeof(float) == 4) ||
		(std::is_same_v<T, double> && sizeof(double) == 8);
#else
	template <typename T>
	constexpr bool is_serializable_builtin_type_v =
		std::is_same_v<T, int8_t> ||
		std::is_same_v<T, uint8_t> ||
		std::is_same_v<T, int16_t> ||
		std::is_same_v<T, uint16_t> ||
		std::is_same_v<T, int32_t> ||
		std::is_same_v<T, uint32_t> ||
		std::is_same_v<T, int64_t> ||
		std::is_same_v<T, uint64_t> ||
		std::is_same_v<T, bool>;
#endif

	template <typename T>
	constexpr bool is_serializable_enum_v = std::is_enum_v<T>;

	template <typename T>
	constexpr bool is_serializable_tuple_v = is_tuple_like_v<T>;

	template <typename T>
	constexpr bool is_serializable_custom_type_v = std::is_trivial_v<T> && has_serialize_targets_definition_v<T>;

	/**
	 * @brief Whether a type is serializable.
	 */
	template <typename T>
	constexpr bool is_serializable_v =
		is_serializable_builtin_type_v<T> ||
		is_serializable_enum_v<T> ||
		is_serializable_tuple_v<T> ||
		is_serializable_boost_static_string_v<T> ||
		is_serializable_custom_type_v<T>;

#if __cpp_concepts
	/**
	 * @brief A concept to constrain types to serializable.
	 */
	template <typename T>
	concept serializable = is_serializable_v<T>;
#endif
}
