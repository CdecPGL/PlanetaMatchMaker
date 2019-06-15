#pragma once

#include <vector>
#include <functional>
#include <boost/endian/conversion.hpp>

#include "utilities/string_utility.hpp"
#include "serialization_error.hpp"
#include "serialize/serializable.hpp"

namespace pgl {
	// Serialize target to send data to a network
	std::vector<uint8_t> serialize(serializable& target);

	// Deserialize data to get data from a network
	void deserialize(const std::vector<uint8_t>& data, serializable& target);

	class serializer final {
	public:
		void operator +=(const serializable& value);

		template <typename T>
		auto operator +=(const T& value) -> std::enable_if_t<std::is_arithmetic_v<T>, void> {
			if (status_ == status::none) {
				throw serialization_error(
					"Cannot add serialization target out of serialization or deserialization process.");
			}

			total_size_ += sizeof(value);
			if (status_ == status::serializing) {
				serializers_.push_back([&value](std::vector<uint8_t>& data, size_t& pos) {
					boost::endian::native_to_big_inplace(value);
					const auto size = sizeof(value);
					std::memcpy(data.data() + pos, &value, size);
					pos += size;
				});
			} else {
				deserializers_.push_back([&value](const std::vector<uint8_t>& data, size_t& pos) {
					const auto size = sizeof(value);
					auto& nc_value = const_cast<T&>(value);
					std::memcpy(&nc_value, data.data() + pos, size);
					boost::endian::big_to_native_inplace(nc_value);
					pos += size;
				});
			}
		}

	private:
		enum class status { none, serializing, deserializing };

		size_t total_size_{0};
		status status_{status::none};
		std::vector<std::function<void(std::vector<uint8_t>&, size_t&)>> serializers_;
		std::vector<std::function<void(const std::vector<uint8_t>&, size_t&)>> deserializers_;

		friend std::vector<uint8_t> serialize(serializable&);
		std::vector<uint8_t> serialize(serializable& target);

		friend void deserialize(const std::vector<uint8_t>&, serializable&);
		void deserialize(const std::vector<uint8_t>& data, serializable& target);
	};
}
