//    _   _____   __________
//   | | / / _ | / __/_  __/     Visibility
//   | |/ / __ |_\ \  / /          Across
//   |___/_/ |_/___/ /_/       Space and Time
//
// SPDX-FileCopyrightText: (c) 2021 The VAST Contributors
// SPDX-License-Identifier: BSD-3-Clause

#include "vast/type/list_type.hpp"

namespace vast::experimental {

namespace {

flatbuffers::Offset<list_type::concrete_table_type>
make_list_type(flatbuffers::FlatBufferBuilder& builder,
               const class type& type) {
  auto concrete_builder = list_type::concrete_table_type::Builder{builder};
  auto bytes = as_bytes(type);
  concrete_builder.add_type(builder.CreateVector(
    reinterpret_cast<const uint8_t*>(bytes.data()), bytes.size()));
  return concrete_builder.Finish();
}

} // namespace

// -- constructors, destructors, and assignment operators ----------------------

list_type::list_type(flatbuffers::FlatBufferBuilder& builder,
                     const class type& type) noexcept
  : concrete_type{builder, make_list_type(builder, type)} {
  // nop
}

list_type::list_type(const class type& type) noexcept {
  auto builder = flatbuffers::FlatBufferBuilder{};
  *this = list_type{builder, type};
}

list_type::list_type(const list_type&) = default;

list_type& list_type::operator=(const list_type&) = default;

list_type::list_type(list_type&&) noexcept = default;

list_type& list_type::operator=(list_type&&) noexcept = default;

list_type::~list_type() noexcept = default;

// -- member functions ---------------------------------------------------------

class type list_type::type() const noexcept {
  return {as_bytes(*concrete_table()->type()), *this};
}

// -- explicit template instantiations -----------------------------------------

template class detail::concrete_type<fbs::type::list_type::v0>;

} // namespace vast::experimental
