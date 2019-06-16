#pragma once

#include <vector>
#include <functional>
#include <boost/endian/conversion.hpp>

#include "utilities/template_utilities.hpp"
#include "utilities/string_utility.hpp"
#include "serialization_error.hpp"
#include "serialize/serialize_type_traits.hpp"

namespace pgl {
	class serializer final {
	public:
		// Add a values as a serialization target. The type of the value must be trivial.
		template <typename T>
		auto operator +=(T& value) -> std::enable_if_t<is_serializable_v<T>> {
			static_assert(std::is_trivial_v<T>,
				"T must be a trivial type bcause the serialize size of T must not be changed in runtime.");
			if (status_ == status::none) {
				throw serialization_error(
					"Cannot add serialization target out of serialization or deserialization process.");
			}

			if constexpr (std::is_arithmetic_v<T>) {
				total_size_ += sizeof(value);
				if (status_ == status::serializing) {
					serializers_.push_back([&value](std::vector<uint8_t>& data, size_t& pos) {
						auto e_value = boost::endian::native_to_big(value);
						const auto size = sizeof(e_value);
						std::memcpy(data.data() + pos, &e_value, size);
						pos += size;
					});
				} else {
					deserializers_.push_back([&value](const std::vector<uint8_t>& data, size_t& pos) {
						const auto size = sizeof(value);
						std::memcpy(&value, data.data() + pos, size);
						boost::endian::big_to_native_inplace(value);
						pos += size;
					});
				}
			} else if constexpr (is_array_container_v<T>) {
				for (auto i = 0u; i < value.size(); ++i) {
					*this += value[i];
				}
			} else {
				on_serialize(value, *this);
			}
		}

	private:
		enum class status { none, serializing, deserializing };

		size_t total_size_{0};
		status status_{status::none};
		std::vector<std::function<void(std::vector<uint8_t>&, size_t&)>> serializers_;
		std::vector<std::function<void(const std::vector<uint8_t>&, size_t&)>> deserializers_;

		template <typename T>
		friend auto serialize(const T&) -> std::enable_if_t<is_serializable_v<T>, std::vector<uint8_t>>;

		template <typename T>
		auto serialize(const T& target) -> std::enable_if_t<is_serializable_v<T>, std::vector<uint8_t>> {
			if (status_ != status::none) {
				throw serialization_error("Cannot start serialization in serialization or deserialization progress.");
			}

			total_size_ = 0;

			serializers_.clear();
			status_ = status::serializing;
			T& nc_target = const_cast<T&>(target);
			on_serialize(nc_target, *this);
			status_ = status::none;
			std::vector<uint8_t> data(total_size_);
			size_t pos = 0;
			for (auto&& serializer : serializers_) {
				serializer(data, pos);
			}
			return data;
		}

		template <typename T>
		friend auto deserialize(T&, const std::vector<uint8_t>& data) -> std::enable_if_t<is_serializable_v<T>>;

		template <typename T>
		auto deserialize(T& target, const std::vector<uint8_t>& data) -> std::enable_if_t<is_serializable_v<T>> {
			if (status_ != status::none) {
				throw serialization_error("Cannot start serialization in serialization or deserialization process.");
			}

			total_size_ = 0;
			deserializers_.clear();
			status_ = status::deserializing;
			on_serialize(target, *this);
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
	};

	template <typename T>
	auto on_serialize(T& value, serializer& serializer) -> std::enable_if_t<has_member_on_serialize_v<T>> {
		value.on_serialize(serializer);
	}

	template <typename T>
	auto on_serialize(T& value, serializer& serializer) -> std::enable_if_t<std::is_arithmetic_v<T>> {
		serializer += value;
	}

	template <typename T, size_t V>
	void on_serialize(std::array<T, V>& value, serializer& serializer) {
		serializer += value;
	}

	// Serialize target to send data to a network
	template <typename T>
	auto serialize(const T& target) -> std::enable_if_t<is_serializable_v<T>, std::vector<uint8_t>> {
		serializer serializer;
		return serializer.serialize(target);
	}

	// Deserialize data to get data from a network
	template <typename T>
	auto deserialize(T& target, const std::vector<uint8_t>& data) -> std::enable_if_t<is_serializable_v<T>> {
		serializer serializer;
		serializer.deserialize(target, data);
	}
}
