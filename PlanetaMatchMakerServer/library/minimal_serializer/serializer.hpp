/*
The MIT License (MIT)

Copyright (c) 2019-2022 Cdec

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once

#include <boost/endian/conversion.hpp>

#include "string_utility.hpp"
#include "type_traits.hpp"

namespace minimal_serializer {
	class serialization_error final : public std::logic_error {
		using logic_error::logic_error;
	};

	template <typename T>
	void convert_endian_native_to_big_inplace(T& value) {
		boost::endian::native_to_big_inplace(value);
	}

	// bool type is endian independent. In addition, bool type is not supported in boost endian conversion from boost library 1.71.0.
	template <>
	inline void convert_endian_native_to_big_inplace(bool&) { }

	template <typename T>
	void convert_endian_big_to_native_inplace(T& value) {
		boost::endian::big_to_native_inplace(value);
	}

	// bool type is endian independent. In addition, bool type is not supported in boost endian conversion from boost library 1.71.0.
	template <>
	inline void convert_endian_big_to_native_inplace(bool&) { }

	template <typename T>
	constexpr auto static_assertion_for_not_serializable_type() -> std::enable_if_t<!is_serializable_v<T>, void> {
		static_assert(std::is_trivial_v<T>,
			"T must be a trivial type because the serialize size of T must not be changed in runtime.");
		static_assert(has_serialize_targets_definition_v<T>,
			"There must be a member alias 'using serialize_targets = minimal_serializer::serialize_target_container<...>' or a template specification for minimal_serializer::serialize_target class template."
		);
	}

	template <typename T, typename Return = void>
	auto raise_error_for_not_serializable_type() -> std::enable_if_t<!is_serializable_v<T>, Return> {
		static_assertion_for_not_serializable_type<T>();
		throw serialization_error("Not serializable type.");
	}

	template <typename T>
	constexpr size_t get_serialized_size_impl();

	template <typename T, size_t... I>
	constexpr size_t get_serialized_size_tuple_impl(std::index_sequence<I...>) {
		return (get_serialized_size_impl<remove_cvref_t<std::tuple_element_t<I, T>>>() + ...);
	}

	template <typename T>
	constexpr size_t get_serialized_size_tuple() {
		return get_serialized_size_tuple_impl<T>(std::make_index_sequence<std::tuple_size_v<T>>{});
	}

	template <typename T>
	constexpr size_t get_serialized_size_impl() {
		using raw_t = remove_cvref_t<T>;
		if constexpr (is_serializable_builtin_type_v<raw_t>) {
			return sizeof(raw_t);
		}
		else if constexpr (is_serializable_enum_v<raw_t>) {
			return sizeof(std::underlying_type_t<raw_t>);
		}
		else if constexpr (is_serializable_tuple_v<raw_t>) {
			return get_serialized_size_tuple<raw_t>();
		}
		else if constexpr (is_serializable_custom_type_v<raw_t>) {
			using target_types = typename serialize_targets_t<raw_t>::types;
			return get_serialized_size_tuple<target_types>();
		}
		else {
			static_assertion_for_not_serializable_type<raw_t>();
			return 0;
		}
	}

	/**
	 * The serialized size of T. If T has const, volatile and/or reference, they will be removed.
	 */
	template <typename T>
	constexpr size_t serialized_size_v = get_serialized_size_impl<T>();

	/**
	 * A fixed size byte array of serialized data for T.
	 */
	template <typename T>
	using serialized_data = std::array<uint8_t, serialized_size_v<T>>;

	template <typename T>
	void serialize_impl(const T& obj, uint8_t* buffer_top, const size_t buffer_size, size_t& offset);

	template <typename T, size_t I>
	void serialize_tuple_impl(const T& obj, uint8_t* buffer_top, const size_t buffer_size, size_t& offset) {
		serialize_impl<remove_cvref_t<std::tuple_element_t<I, T>>>(std::get<I>(obj), buffer_top, buffer_size, offset);
	}

	template <typename T, size_t... Is>
	void serialize_tuple_impl(const T& obj, uint8_t* buffer_top, const size_t buffer_size, size_t& offset,
							std::index_sequence<Is...>) {
		(serialize_tuple_impl<T, Is>(obj, buffer_top, buffer_size, offset), ...);
	}

	template <typename T>
	void serialize_tuple(const T& obj, uint8_t* buffer_top, const size_t buffer_size, size_t& offset) {
		serialize_tuple_impl<T>(obj, buffer_top, buffer_size, offset, std::make_index_sequence<std::tuple_size_v<T>>{});
	}

	template <typename T>
	void serialize_impl(const T& obj, uint8_t* buffer_top, const size_t buffer_size, size_t& offset) {
		if constexpr (is_serializable_builtin_type_v<T>) {
			constexpr auto size = sizeof(T);
			if (offset + size > buffer_size) {
				throw serialization_error("Serialization source is out of range.");
			}

			auto* data_ptr = buffer_top + offset;
			std::memcpy(data_ptr, &obj, size);
			auto& e_value = *reinterpret_cast<T*>(data_ptr);
			convert_endian_native_to_big_inplace(e_value);
			offset += size;
		}
		else if constexpr (is_serializable_enum_v<T>) {
			using underlying_type = std::underlying_type_t<T>;
			serialize_impl<underlying_type>(static_cast<underlying_type>(obj), buffer_top, buffer_size, offset);
		}
		else if constexpr (is_serializable_tuple_v<T>) {
			serialize_tuple<T>(obj, buffer_top, buffer_size, offset);
		}
		else if constexpr (is_serializable_custom_type_v<T>) {
			using target_types = typename serialize_targets_t<T>::const_reference_types;
			const auto target_references = serialize_targets_t<T>::get_const_reference_tuple(obj);
			serialize_tuple<target_types>(target_references, buffer_top, buffer_size, offset);
		}
		else {
			raise_error_for_not_serializable_type<T>();
		}
	}

	/**
	 * Serialize data to size fixed byte array.
	 * 
	 * @param obj A object to serialize.
	 * @tparam T The type of data to serialize.
	 * @return A serialized byte array.
	 * @throw serialization_error Serialization is failed.
	 */
	template <typename T>
	serialized_data<T> serialize(const T& obj) {
		size_t offset = 0;
		serialized_data<T> data;
		serialize_impl(obj, data.data(), data.size(), offset);
		return data;
	}

	/**
	 * Serialize data to buffer.
	 * 
	 * @param obj A object to serialize.
	 * @param buffer A destination buffer which has data() and size() member function.
	 * @param offset offset A reference start position of source byte array.
	 * @tparam T The type of data to serialize.
	 * @throw serialization_error Serialization is failed.
	 */
	template <typename T, typename Buffer>
	auto serialize(const T& obj, Buffer& buffer,
					size_t offset) -> decltype(std::declval<Buffer>().data(), std::declval<Buffer>().size(), void()) {
		serialize_impl(obj, buffer.data(), buffer.size(), offset);
	}

	/**
	 * Serialize data to buffer.
	 *
	 * @param obj A object to serialize to output stream.
	 * @param stream A destination stream.
	 * @tparam T The type of data to serialize.
	 * @throw serialization_error Serialization is failed.
	 */
	template <typename T>
	void serialize(const T& obj, std::ostream& stream) {
		const auto buffer = serialize(obj);
		for (const auto& data : buffer) {
			stream << data;
		}
	}

	template <typename T>
	void deserialize_impl(T& obj, const uint8_t* buffer_top, const size_t buffer_size, size_t& offset);

	template <typename T, size_t I>
	void deserialize_tuple_impl(T& obj, const uint8_t* buffer_top, const size_t buffer_size, size_t& offset) {
		deserialize_impl<remove_cvref_t<std::tuple_element_t<I, T>>>(std::get<I>(obj), buffer_top, buffer_size, offset);
	}

	template <typename T, size_t... Is>
	void deserialize_tuple_impl(T& obj, const uint8_t* buffer_top, const size_t buffer_size, size_t& offset,
								std::index_sequence<Is...>) {
		(deserialize_tuple_impl<T, Is>(obj, buffer_top, buffer_size, offset), ...);
	}

	template <typename T>
	void deserialize_tuple(T& obj, const uint8_t* buffer_top, const size_t buffer_size, size_t& offset) {
		deserialize_tuple_impl<T>(obj, buffer_top, buffer_size, offset,
								std::make_index_sequence<std::tuple_size_v<T>>{});
	}

	template <class T>
	void deserialize_impl(T& obj, const uint8_t* buffer_top, const size_t buffer_size, size_t& offset) {
		if constexpr (is_serializable_builtin_type_v<T>) {
			const auto size = sizeof(T);
			if (offset + size > buffer_size) {
				throw serialization_error("Deserialization destination is out of range.");
			}

			std::memcpy(&obj, buffer_top + offset, size);
			convert_endian_big_to_native_inplace(obj);
			offset += size;
		}
		else if constexpr (is_serializable_enum_v<T>) {
			using underlying_type = std::underlying_type_t<T>;
			// In order to cast with referencing same value, cast via pointer.
			deserialize_impl<underlying_type>(*reinterpret_cast<underlying_type*>(&obj), buffer_top, buffer_size,
											offset);
		}
		else if constexpr (is_serializable_tuple_v<T>) {
			deserialize_tuple<T>(obj, buffer_top, buffer_size, offset);
		}
		else if constexpr (is_serializable_custom_type_v<T>) {
			using target_types = typename serialize_targets_t<T>::reference_types;
			auto target_references = serialize_targets_t<T>::get_reference_tuple(obj);
			deserialize_tuple<target_types>(target_references, buffer_top, buffer_size, offset);
		}
		else {
			raise_error_for_not_serializable_type<T>();
		}
	}

	/**
	 * Deserialize data from buffer.
	 * 
	 * @param obj A object to deserialize.
	 * @param buffer A source buffer which has data() and size() member function.
	 * @param offset A reference start position of source byte array.
	 * @tparam T The type of data to deserialize.
	 * @throw serialization_error Deserialization is failed.
	 */
	template <typename T, typename Buffer>
	auto deserialize(T& obj, const Buffer& buffer,
					size_t offset = 0) -> decltype(std::declval<Buffer>().data(), std::declval<Buffer>().size(),
		std::enable_if_t<!std::is_const_v<T>, void>()) {
		deserialize_impl(obj, buffer.data(), buffer.size(), offset);
	}

	/**
	 * Deserialize data from input stream.
	 *
	 * @param obj A object to deserialize.
	 * @param stream A source input stream.
	 * @tparam T The type of data to deserialize.
	 * @throw serialization_error Deserialization is failed.
	 */
	template <typename T>
	auto deserialize(T& obj, std::istream& stream) -> std::enable_if_t<!std::is_const_v<T>, void> {
		serialized_data<T> buffer;
		for (auto&& data : buffer) {
			stream >> data;
		}
		deserialize(obj, buffer);
	}
}
