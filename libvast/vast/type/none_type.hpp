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

/// TODO: document
class none_type : public detail::concrete_type<fbs::Type> {
public:
  // -- member types -----------------------------------------------------------

  using super = detail::concrete_type<fbs::Type>;
  using super::concrete_type_index;
  using typename super::concrete_table_type;

  /// The fix byte size of a none type.
  inline static constexpr size_t buffer_size = 28;

  // -- constructors, destructors, and assignment operators --------------------

  explicit none_type(flatbuffers::FlatBufferBuilder& builder) noexcept;
  none_type() noexcept;
  none_type(const none_type&);
  none_type& operator=(const none_type&);
  none_type(none_type&&) noexcept;
  none_type& operator=(none_type&&) noexcept;
  ~none_type() noexcept;
};

// -- explicit template instantiations -----------------------------------------

extern template class detail::concrete_type<fbs::Type>;

} // namespace vast::experimental
