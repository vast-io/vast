//    _   _____   __________
//   | | / / _ | / __/_  __/     Visibility
//   | |/ / __ |_\ \  / /          Across
//   |___/_/ |_/___/ /_/       Space and Time
//
// SPDX-FileCopyrightText: (c) 2021 The VAST Contributors
// SPDX-License-Identifier: BSD-3-Clause

#include "vast/type/alias_type.hpp"

namespace vast::experimental {

namespace {

flatbuffers::Offset<alias_type::concrete_table_type>
make_alias_type(flatbuffers::FlatBufferBuilder& builder, std::string_view name,
                const class type& type) {
  auto concrete_builder = alias_type::concrete_table_type::Builder{builder};
  concrete_builder.add_name(builder.CreateString(name));
  auto bytes = as_bytes(type);
  concrete_builder.add_type(builder.CreateVector(
    reinterpret_cast<const uint8_t*>(bytes.data()), bytes.size()));
  return concrete_builder.Finish();
}

} // namespace

// -- constructors, destructors, and assignment operators ----------------------

alias_type::alias_type(flatbuffers::FlatBufferBuilder& builder,
                       std::string_view name, const class type& type) noexcept
  : concrete_type{builder, make_alias_type(builder, name, type)} {
  // nop
}

alias_type::alias_type(std::string_view name, const class type& type) noexcept {
  auto builder = flatbuffers::FlatBufferBuilder{};
  *this = alias_type{builder, name, type};
}

alias_type::alias_type(const alias_type&) = default;

alias_type& alias_type::operator=(const alias_type&) = default;

alias_type::alias_type(alias_type&&) noexcept = default;

alias_type& alias_type::operator=(alias_type&&) noexcept = default;

alias_type::~alias_type() noexcept = default;

// -- member functions ---------------------------------------------------------

std::string_view alias_type::name() const noexcept {
  return concrete_table()->name()->string_view();
}

class type alias_type::type() const noexcept {
  return {as_bytes(*concrete_table()->type()), *this};
}

// -- explicit template instantiations -----------------------------------------

template class detail::concrete_type<fbs::type::alias_type::v0>;

} // namespace vast::experimental
