#pragma once

#include <utility>
#include <tuple>

namespace pgl {
	/**
	 * An function template to implement member_variable_pointer_*_t.
	 * @tparam C A class type.
	 * @tparam V A variable type.
	 * @param p A member variable pointer.
	 * @return std::pair contains the class type and the variable type of passed member variable pointer.
	 */
	template <typename C, typename V>
	auto member_variable_pointer_t_impl(V C::* p)->std::pair<C, V>;

	/**
	 * An alias template to get the type of the class from a member variable pointer.
	 * @tparam P A member function pointer.
	 */
	template <auto P>
	using member_variable_pointer_class_t = typename decltype(pgl::member_variable_pointer_t_impl(P))::first_type;

	/**
	 * An alias template to get the type of the variable from a member variable pointer.
	 * @tparam P A member function pointer.
	 */
	template <auto P>
	using member_variable_pointer_variable_t = typename decltype(pgl::member_variable_pointer_t_impl(P))::second_type;

	/**
	 * An function template to implement member_function_pointer_*_t.
	 * @tparam C A class type.
	 * @tparam R A return value type.
	 * @tparam Ps Parameter types by std::tuple.
	 * @param p A member function pointer.
	 * @return std::tuple contains the class type, the return value type and the parameter types of passed member function pointer.
	 */
	template <typename C, typename R, typename... Ps>
	auto member_function_pointer_t_impl(R(C::* p)(Ps...))->std::tuple<C, R, std::tuple<Ps...>>;

	/**
	 * An alias template to get the type of the class from a member function pointer.
	 * @tparam P A member function pointer.
	 */
	template <auto P>
	using member_function_pointer_class_t = std::tuple_element_t<0, decltype(pgl::member_function_pointer_t_impl(P))>;

	/**
	 * An alias template to get the type of the return value from a member function pointer.
	 * @tparam P A member function pointer.
	 */
	template <auto P>
	using member_function_pointer_return_t = std::tuple_element_t<1, decltype(pgl::member_function_pointer_t_impl(P))>;

	/**
	 * An alias template to get the types of all parameters from a member function pointer by std::tuple.
	 * @tparam P A member function pointer.
	 */
	template <auto P>
	using member_function_pointer_parameters_t = std::tuple_element_t<2, decltype(pgl::member_function_pointer_t_impl(P))>;

	/**
	 * An alias template to get the type of the indicated parameter from a member function pointer.
	 * @tparam P A member function pointer.
	 * @tparam I An index of the parameter you want to get the type of.
	 */
	template <auto P, size_t I>
	using member_function_pointer_parameter_t = std::tuple_element_t<I, member_function_pointer_parameters_t<P>>;
}