#pragma once

#include "nameof.hpp"

#include "string_utility.hpp"

namespace pgl {
	class static_cast_assertion_error final : public std::logic_error {
	public:
		using std::logic_error::logic_error;
	};

	template <class Destination, class Source>
	Destination static_cast_with_range_assertion(const Source& source) {
		Destination destination{static_cast<Destination>(source)};
#if _DEBUG
		Source round_trip{static_cast<Source>(destination)};
		if (round_trip != source) {
			auto error_message = generate_string("Source value (", source, ") is out of range in destination type (",
			                                     nameof::nameof_type<Destination>(), ").");
			throw static_cast_assertion_error(error_message);
		}
#endif
		return destination;
	}
}
