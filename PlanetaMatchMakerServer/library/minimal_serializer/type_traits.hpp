/*
The MIT License (MIT)

Copyright (c) 2019 Cdec

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once

#include <type_traits>
#include <memory>

#include <boost/tti/has_member_function.hpp>

namespace minimal_serializer {
	class serializer;

	// Remove const, volatile and reference
	template <typename T>
	using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;

	BOOST_TTI_HAS_MEMBER_FUNCTION(on_serialize)
	
	template <typename T>
	constexpr bool has_member_on_serialize_v = has_member_function_on_serialize<T, void, boost::mpl::vector<serializer&>
		, boost::function_types::non_cv>::value;

	struct has_global_on_serialize_impl {
		template <class T>
		static auto check(T&& x) -> decltype(on_serialize(std::declval<T&>(), std::declval<serializer&>()), std::true_type());

		template <class T>
		static std::false_type check(...);
	};
	
	template <class T>
	constexpr bool has_global_on_serialize_v
		= decltype(has_global_on_serialize_impl::check<T>(std::declval<T>()))::value;

	template <class T>
	constexpr bool is_serializable_v = has_global_on_serialize_v<T> && std::is_trivial_v<T>;

	struct is_fixed_array_container_impl {
		template <class T>
		static auto check(T&& x) -> decltype(x.operator[](std::declval<size_t>()), x.max_size(), std::true_type{});

		template <class T>
		static auto check(...)->std::false_type;
	};

	template <class T>
	struct is_fixed_array_container final : decltype(is_fixed_array_container_impl::check<T>(std::declval<T>())) {};

	template <class T>
	constexpr bool is_fixed_array_container_v = is_fixed_array_container<T>::value;

	template <typename T>
	struct is_shared_ptr : std::false_type {};

	template <typename T>
	struct is_shared_ptr<std::shared_ptr<T>> : std::true_type {};

	template <typename T>
	constexpr bool is_shared_ptr_v = is_shared_ptr<T>::value;
}
