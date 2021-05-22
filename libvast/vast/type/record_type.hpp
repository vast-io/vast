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

struct record_field final {
  const std::string_view name;
  const class type type;
};

/// A list of fields, each of which have a name and type.
class record_type final
  : public detail::concrete_type<fbs::type::record_type::v0> {
public:
  // -- member constants -------------------------------------------------------

  /// The maximum nesting depth of record types.
  inline static constexpr auto max_depth = 64;

  // -- constructors, destructors, and assignment operators --------------------

  record_type() = delete;

  /// @note The maximum nesting depth of record types must not exceed \ref
  /// record_type::max_depth.
  /// @pre !fields.empty()
  record_type(flatbuffers::FlatBufferBuilder& builder,
              const std::vector<record_field>& fields) noexcept;

  /// @note The maximum nesting depth of record types must not exceed \ref
  /// record_type::max_depth.
  /// @pre !fields.empty()
  explicit record_type(const std::vector<record_field>& fields) noexcept;

  record_type(const record_type&);
  record_type& operator=(const record_type&);
  record_type(record_type&&) noexcept;
  record_type& operator=(record_type&&) noexcept;
  ~record_type() noexcept;

  // -- member functions -------------------------------------------------------

  /// Returns the number of unflattened fields in the record type.
  [[nodiscard]] size_t size() const noexcept;

  /// Computes the nesting depth of the record type.
  [[nodiscard]] size_t depth() const noexcept;

  /// Returns the number of flattened fields in the record type.
  [[nodiscard]] size_t num_leaves() const noexcept;

  /// Access a field at the given \param index.
  [[nodiscard]] record_field field(size_t index) const noexcept;
};

// -- explicit template instantiations -----------------------------------------

extern template class detail::concrete_type<fbs::type::record_type::v0>;

} // namespace vast::experimental
