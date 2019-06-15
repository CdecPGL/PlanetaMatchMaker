#pragma once
#include <stdexcept>

namespace pgl {
	class serialization_error final : public std::logic_error {
		using logic_error::logic_error;
	};
}
