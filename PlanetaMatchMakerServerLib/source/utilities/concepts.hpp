// /*
// The MIT License (MIT)
// Copyright (c) 2022 Cdec
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
// */

#pragma once
#include <minimal_serializer/type_traits.hpp>

namespace pgl {
	/**
	 * A concept to constrain the type is serializable by minimal serializer.
	 */
	template <typename T>
	concept serializable = minimal_serializer::serializable<std::remove_cvref_t<T>>;

	/**
	 * A concept to constrain all types are serializable by minimal serializer.
	 */
	template <typename... T>
	concept serializable_all = (serializable<T> && ...);

	/**
	 * A concept to constrain the type is constant.
	 */
	template <typename T>
	concept constant = std::is_const_v<T>;

	/**
	 * A concept to constrain all types are is constant.
	 */
	template <typename... T>
	concept constant_all = (constant<T> && ...);

	/**
	 * A concept to constrain the type is constant.
	 */
	template <typename T>
	concept not_constant = !constant<T>;

	/**
	 * A concept to constrain all types are not constant.
	 */
	template <typename... T>
	concept not_constant_all = (not_constant<T> && ...);
}
