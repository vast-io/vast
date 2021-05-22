//    _   _____   __________
//   | | / / _ | / __/_  __/     Visibility
//   | |/ / __ |_\ \  / /          Across
//   |___/_/ |_/___/ /_/       Space and Time
//
// SPDX-FileCopyrightText: (c) 2021 The VAST Contributors
// SPDX-License-Identifier: BSD-3-Clause

#include "vast/type/enumeration_type.hpp"

namespace vast::experimental {

namespace {

flatbuffers::Offset<enumeration_type::concrete_table_type>
make_enumeration_type(flatbuffers::FlatBufferBuilder& builder,
                      const std::vector<std::string>& fields) {
  auto concrete_builder
    = enumeration_type::concrete_table_type::Builder{builder};
  concrete_builder.add_fields(builder.CreateVectorOfStrings(fields));
  return concrete_builder.Finish();
}

} // namespace

// -- constructors, destructors, and assignment operators ----------------------

enumeration_type::enumeration_type(
  flatbuffers::FlatBufferBuilder& builder,
  const std::vector<std::string>& fields) noexcept
  : concrete_type{builder, make_enumeration_type(builder, fields)} {
  // nop
}

enumeration_type::enumeration_type(
  const std::vector<std::string>& fields) noexcept {
  auto builder = flatbuffers::FlatBufferBuilder{};
  *this = enumeration_type{builder, fields};
}

enumeration_type::enumeration_type(const enumeration_type&) = default;

enumeration_type& enumeration_type::operator=(const enumeration_type&)
  = default;

enumeration_type::enumeration_type(enumeration_type&&) noexcept = default;

enumeration_type&
enumeration_type::operator=(enumeration_type&&) noexcept = default;

enumeration_type::~enumeration_type() noexcept = default;

// -- member functions ---------------------------------------------------------

size_t enumeration_type::size() const {
  return concrete_table()->fields()->size();
}

std::string_view enumeration_type::field(size_t index) const {
  return concrete_table()->fields()->Get(index)->string_view();
}

// -- explicit template instantiations -----------------------------------------

template class detail::concrete_type<fbs::type::enumeration_type::v0>;

} // namespace vast::experimental
