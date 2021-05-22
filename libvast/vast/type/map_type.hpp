//    _   _____   __________
//   | | / / _ | / __/_  __/     Visibility
//   | |/ / __ |_\ \  / /          Across
//   |___/_/ |_/___/ /_/       Space and Time
//
// SPDX-FileCopyrightText: (c) 2021 The VAST Contributors
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "vast/type/type.hpp"

namespace vast::experimental {

/// An associative mapping from keys to values.
class map_type final : public detail::concrete_type<fbs::type::map_type::v0> {
public:
  // -- constructors, destructors, and assignment operators --------------------

  map_type() = delete;
  map_type(flatbuffers::FlatBufferBuilder& builder, const class type& key_type,
           const class type& value_type) noexcept;
  map_type(const class type& key_type, const class type& value_type) noexcept;

  map_type(const map_type&);
  map_type& operator=(const map_type&);
  map_type(map_type&&) noexcept;
  map_type& operator=(map_type&&) noexcept;
  ~map_type() noexcept;

  // -- member functions -------------------------------------------------------

  [[nodiscard]] class type key_type() const noexcept;
  [[nodiscard]] class type value_type() const noexcept;
};

// -- explicit template instantiations -----------------------------------------

extern template class detail::concrete_type<fbs::type::map_type::v0>;

} // namespace vast::experimental
