//    _   _____   __________
//   | | / / _ | / __/_  __/     Visibility
//   | |/ / __ |_\ \  / /          Across
//   |___/_/ |_/___/ /_/       Space and Time
//
// SPDX-FileCopyrightText: (c) 2021 The VAST Contributors
// SPDX-License-Identifier: BSD-3-Clause

#include "vast/type/map_type.hpp"

namespace vast::experimental {

namespace {

flatbuffers::Offset<map_type::concrete_table_type>
make_map_type(flatbuffers::FlatBufferBuilder& builder,
              const class type& key_type, const class type& value_type) {
  auto concrete_builder = map_type::concrete_table_type::Builder{builder};
  auto key_bytes = as_bytes(key_type);
  concrete_builder.add_key_type(builder.CreateVector(
    reinterpret_cast<const uint8_t*>(key_bytes.data()), key_bytes.size()));
  auto value_bytes = as_bytes(value_type);
  concrete_builder.add_value_type(builder.CreateVector(
    reinterpret_cast<const uint8_t*>(value_bytes.data()), value_bytes.size()));
  return concrete_builder.Finish();
}

} // namespace

// -- constructors, destructors, and assignment operators ----------------------

map_type::map_type(flatbuffers::FlatBufferBuilder& builder,
                   const class type& key_type,
                   const class type& value_type) noexcept
  : concrete_type{builder, make_map_type(builder, key_type, value_type)} {
  // nop
}

map_type::map_type(const class type& key_type,
                   const class type& value_type) noexcept {
  auto builder = flatbuffers::FlatBufferBuilder{};
  *this = map_type{builder, key_type, value_type};
}

map_type::map_type(const map_type&) = default;

map_type& map_type::operator=(const map_type&) = default;

map_type::map_type(map_type&&) noexcept = default;

map_type& map_type::operator=(map_type&&) noexcept = default;

map_type::~map_type() noexcept = default;

// -- member functions ---------------------------------------------------------

class type map_type::key_type() const noexcept {
  return {as_bytes(*concrete_table()->key_type()), *this};
}

class type map_type::value_type() const noexcept {
  return {as_bytes(*concrete_table()->value_type()), *this};
}

// -- explicit template instantiations -----------------------------------------

template class detail::concrete_type<fbs::type::map_type::v0>;

} // namespace vast::experimental
