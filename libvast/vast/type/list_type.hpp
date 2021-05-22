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

/// An ordered sequence of values.
class list_type final : public detail::concrete_type<fbs::type::list_type::v0> {
public:
  // -- constructors, destructors, and assignment operators --------------------

  list_type() = delete;
  list_type(flatbuffers::FlatBufferBuilder& builder,
            const class type& type) noexcept;
  explicit list_type(const class type& type) noexcept;

  list_type(const list_type&);
  list_type& operator=(const list_type&);
  list_type(list_type&&) noexcept;
  list_type& operator=(list_type&&) noexcept;
  ~list_type() noexcept;

  // -- member functions -------------------------------------------------------

  [[nodiscard]] class type type() const noexcept;
};

// -- explicit template instantiations -----------------------------------------

extern template class detail::concrete_type<fbs::type::list_type::v0>;

} // namespace vast::experimental
