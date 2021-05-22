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

/// An enumeration type that can have one specific value.
class enumeration_type final
  : public detail::concrete_type<fbs::type::enumeration_type::v0> {
public:
  // -- constructors, destructors, and assignment operators --------------------

  enumeration_type() = delete;
  enumeration_type(flatbuffers::FlatBufferBuilder& builder,
                   const std::vector<std::string>& fields) noexcept;
  enumeration_type(const std::vector<std::string>& fields) noexcept;

  enumeration_type(const enumeration_type&);
  enumeration_type& operator=(const enumeration_type&);
  enumeration_type(enumeration_type&&) noexcept;
  enumeration_type& operator=(enumeration_type&&) noexcept;
  ~enumeration_type() noexcept;

  // -- member functions -------------------------------------------------------

  [[nodiscard]] size_t size() const;

  [[nodiscard]] std::string_view field(size_t index) const;
};

// -- explicit template instantiations -----------------------------------------

extern template class detail::concrete_type<fbs::type::enumeration_type::v0>;

} // namespace vast::experimental
