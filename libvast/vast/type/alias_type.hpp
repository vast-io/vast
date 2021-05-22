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

/// A type with a name.
class alias_type final
  : public detail::concrete_type<fbs::type::alias_type::v0> {
public:
  // -- constructors, destructors, and assignment operators --------------------

  alias_type() = delete;
  alias_type(flatbuffers::FlatBufferBuilder& builder, std::string_view name,
             const class type& type) noexcept;
  alias_type(std::string_view name, const class type& type) noexcept;

  alias_type(const alias_type&);
  alias_type& operator=(const alias_type&);
  alias_type(alias_type&&) noexcept;
  alias_type& operator=(alias_type&&) noexcept;
  ~alias_type() noexcept;

  // -- member functions -------------------------------------------------------

  [[nodiscard]] std::string_view name() const noexcept;
  [[nodiscard]] class type type() const noexcept;
};

// -- explicit template instantiations -----------------------------------------

extern template class detail::concrete_type<fbs::type::alias_type::v0>;

} // namespace vast::experimental
