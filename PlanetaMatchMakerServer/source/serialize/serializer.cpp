#include "serialize/serializer.hpp"

namespace pgl {

	std::vector<uint8_t> serializer::serialize(serializable& target) {
		if (status_ != status::none) {
			throw serialization_error("Cannot start serialization in serialization or deserialization progress.");
		}

		total_size_ = 0;
		serializers_.clear();
		status_ = status::serializing;
		target.on_serialize(*this);
		status_ = status::none;
		std::vector<uint8_t> data(total_size_);
		size_t pos = 0;
		for (auto&& serializer : serializers_) {
			serializer(data, pos);
		}
		return data;
	}

	void serializer::deserialize(const std::vector<uint8_t>& data, serializable& target) {
		if (status_ != status::none) {
			throw serialization_error("Cannot start serialization in serialization or deserialization process.");
		}

		total_size_ = 0;
		deserializers_.clear();
		status_ = status::deserializing;
		target.on_serialize(*this);
		status_ = status::none;
		if (total_size_ != data.size()) {
			const auto message = generate_string("The data size (", data.size(),
				" bytes) does not match to the passed size (", total_size_,
				" bytes) for deserialization of the type (", typeid(target), ").");
			throw serialization_error(message);
		}
		size_t pos = 0;
		for (auto&& deserializer : deserializers_) {
			deserializer(data, pos);
		}
	}

	void serializer::operator+=(const serializable& value) {
		auto& nc_value = const_cast<serializable&>(value);
		nc_value.on_serialize(*this);
	}

	std::vector<uint8_t> serialize(serializable& target) {
		serializer serializer;
		return serializer.serialize(target);
	}

	void deserialize(const std::vector<uint8_t>& data, serializable& target) {
		serializer serializer;
		serializer.deserialize(data, target);
	}
}
