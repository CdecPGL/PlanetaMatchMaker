#pragma once

#include <iostream>

#include "string_utility.hpp"

namespace pgl {
	template <typename ... Params>
	void print(Params&& ... params) {
		std::cout << generate_string(params...);
	}

	template <typename ... Params>
	void print_line(Params&& ... params) {
		std::cout << generate_string(params...) << std::endl;
	}

	template <typename ... Params>
	void print_error(Params&& ... params) {
		std::cerr << generate_string(params...);
	}

	template <typename ... Params>
	void print_error_line(Params&& ... params) {
		std::cerr << generate_string(params...) << std::endl;
	}
}
