#pragma once

#include <type_traits>

#include "nameof.hpp"

#include "minimal_serializer/string_utility.hpp"

namespace pgl {
	class static_cast_range_error final : public std::logic_error {
	public:
		using std::logic_error::logic_error;
	};

	template <class Destination, class Source>
	Destination range_checked_static_cast(const Source& source) {
		Destination destination{static_cast<Destination>(source)};
		Source round_trip{static_cast<Source>(destination)};
		if (round_trip != source || std::is_unsigned_v<Destination> && source < 0) {
			auto error_message = minimal_serializer::generate_string("Source value (", source, ") of type (",
				nameof::nameof_type<Source>(), ") is out of range in destination type (",
				nameof::nameof_type<Destination>(), ").");
			throw static_cast_range_error(error_message);
		}
		return destination;
	}
}
