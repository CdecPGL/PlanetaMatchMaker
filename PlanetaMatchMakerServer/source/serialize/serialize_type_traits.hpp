#pragma once

#include <type_traits>

#include <boost/tti/has_member_function.hpp>

namespace pgl {
	class serializer;

	BOOST_TTI_HAS_MEMBER_FUNCTION(on_serialize)

	template <typename T>
	constexpr bool has_member_on_serialize_v = has_member_function_on_serialize<T, void, boost::mpl::vector<serializer&>
		, boost
		::function_types::non_cv>::value;

	struct has_global_on_serialize_impl {
		template <class T, void(*)(T&, serializer&) = &on_serialize>
		static std::true_type check(T&& x);

		template <class T>
		static std::false_type check(...);
	};

	template <class T>
	constexpr bool has_global_on_serialize_v
		= decltype(has_global_on_serialize_impl::check<T>(std::declval<T>()))::value;

	template <class T>
	constexpr bool is_serializable_v = has_global_on_serialize_v<T>;
}
