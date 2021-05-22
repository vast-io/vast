//    _   _____   __________
//   | | / / _ | / __/_  __/     Visibility
//   | |/ / __ |_\ \  / /          Across
//   |___/_/ |_/___/ /_/       Space and Time
//
// SPDX-FileCopyrightText: (c) 2021 The VAST Contributors
// SPDX-License-Identifier: BSD-3-Clause

#include "vast/type/none_type.hpp"

namespace vast::experimental {

namespace {

none_type build() {
  auto builder = flatbuffers::FlatBufferBuilder{none_type::buffer_size};
  return none_type{builder};
}

const auto singleton = build();

} // namespace

// -- constructors, destructors, and assignment operators ----------------------

none_type::none_type(flatbuffers::FlatBufferBuilder& builder) noexcept
  : super{builder, fbs::CreateType(builder)} {
  VAST_ASSERT(as_bytes(*this).size() == buffer_size,
              "underlying byte representation of none_type does not match the "
              "expected size");
}

none_type::none_type() noexcept : none_type{singleton} {
  // nop
}

none_type::none_type(const none_type&) = default;
none_type& none_type::operator=(const none_type&) = default;
none_type::none_type(none_type&&) noexcept = default;
none_type& none_type::operator=(none_type&&) noexcept = default;
none_type::~none_type() noexcept = default;

// -- explicit template instantiations -----------------------------------------

template class detail::concrete_type<fbs::Type>;

} // namespace vast::experimental
