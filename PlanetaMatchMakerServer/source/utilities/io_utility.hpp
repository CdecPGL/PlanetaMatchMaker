#pragma once

#include <iostream>

#include "minimal_serializer/string_utility.hpp"

namespace pgl {
	// print parameters to stdout. This is thread safe but outputs from multiple threads may mixed.
	template <typename ... Params>
	void print(Params&& ... params) {
		std::cout << generate_string(params...);
	}

	// print parameters to stdout and force a line break. This is thread safe but outputs from multiple threads may mixed.
	template <typename ... Params>
	void print_line(Params&& ... params) {
		std::cout << generate_string(params...) << std::endl;
	}

	// print parameters to stderr. This is thread safe but outputs from multiple threads may mixed.
	template <typename ... Params>
	void print_error(Params&& ... params) {
		std::cerr << generate_string(params...);
	}

	// print parameters to stderr and force a line break. This is thread safe but outputs from multiple threads may mixed.
	template <typename ... Params>
	void print_error_line(Params&& ... params) {
		std::cerr << generate_string(params...) << std::endl;
	}
}
