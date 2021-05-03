//    _   _____   __________
//   | | / / _ | / __/_  __/     Visibility
//   | |/ / __ |_\ \  / /          Across
//   |___/_/ |_/___/ /_/       Space and Time
//
// SPDX-FileCopyrightText: (c) 2021 The VAST Contributors
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "vast/detail/type_traits.hpp"

#include <iterator>
#include <type_traits>

namespace vast::detail {

template <class T>
concept transparent = requires {
  typename T::is_transparent;
};

/// Types that work with std::data and std::size (= containers)
template <class T>
concept container = requires(T t) {
  std::data(t);
  std::size(t);
};

/// Contiguous byte buffers
template <class T>
concept byte_container = requires(T t) {
  std::data(t);
  std::size(t);
  sizeof(decltype(*std::data(t))) == 1;
};

struct any_callable {
  template <class... Ts>
  void operator()(Ts&&...);
};

/// Inspectables
template <class T>
concept is_inspectable = requires(detail::any_callable& i, T& x) {
  {inspect(i, x)};
};

/// push_back
template <class C>
concept has_insert = requires(C xs, typename C::value_type x) {
  xs.insert(x);
};

template <class C>
concept has_push_back = requires(C xs, typename C::value_type x) {
  xs.push_back(x);
};

template <class T>
concept monoid = requires(T x, T y) {
  { mappend(x, y) } -> std::same_as<T>;
};

} // namespace vast::detail
