//    _   _____   __________
//   | | / / _ | / __/_  __/     Visibility
//   | |/ / __ |_\ \  / /          Across
//   |___/_/ |_/___/ /_/       Space and Time
//
// SPDX-FileCopyrightText: (c) 2021 The VAST Contributors
// SPDX-License-Identifier: BSD-3-Clause

#include "vast/type/record_type.hpp"

namespace vast::experimental {

namespace {

flatbuffers::Offset<record_type::concrete_table_type>
make_record_type(flatbuffers::FlatBufferBuilder& builder,
                 const std::vector<record_field>& fields) {
  VAST_ASSERT(!fields.empty(), "record type must contain fields");
  auto field_offsets
    = std::vector<flatbuffers::Offset<fbs::type::alias_type::v0>>{};
  field_offsets.reserve(fields.size());
  for (const auto& field : fields) {
    const auto bytes = as_bytes(field.name);
    const auto field_offset = fbs::type::alias_type::Createv0(
      builder, builder.CreateString(field.name),
      builder.CreateVector(reinterpret_cast<const uint8_t*>(bytes.data()),
                           bytes.size()));
    field_offsets.push_back(field_offset);
  }
  auto concrete_builder = record_type::concrete_table_type::Builder{builder};
  concrete_builder.add_fields(builder.CreateVector(field_offsets));
  return concrete_builder.Finish();
}

} // namespace

// -- constructors, destructors, and assignment operators ----------------------

record_type::record_type(flatbuffers::FlatBufferBuilder& builder,
                         const std::vector<record_field>& fields) noexcept
  : concrete_type{builder, make_record_type(builder, fields)} {
  VAST_ASSERT(depth() <= max_depth, "record type depth exceeded maximum "
                                    "nesting depth");
}

record_type::record_type(const std::vector<record_field>& fields) noexcept {
  auto builder = flatbuffers::FlatBufferBuilder{};
  *this = record_type{builder, fields};
}

record_type::record_type(const record_type&) = default;

record_type& record_type::operator=(const record_type&) = default;

record_type::record_type(record_type&&) noexcept = default;

record_type& record_type::operator=(record_type&&) noexcept = default;

record_type::~record_type() noexcept = default;

// -- member functions ---------------------------------------------------------

size_t record_type::size() const noexcept {
  return concrete_table()->fields()->size();
}

size_t record_type::depth() const noexcept {
  const auto impl
    = [](const auto& impl,
         const fbs::type::record_type::v0* record_type) -> size_t {
    size_t result = 1;
    for (const auto& field : *record_type->fields()) {
      const auto* current = field->type_nested_root();
      while (const auto* alias = current->type_as_alias_type_v0())
        current = alias->type_nested_root();
      if (const auto* record = current->type_as_record_type_v0())
        result = std::max(result, 1 + impl(impl, record));
    }
    return result;
  };
  return impl(impl, concrete_table());
}

size_t record_type::num_leaves() const noexcept {
  const auto impl
    = [](const auto& impl,
         const fbs::type::record_type::v0* record_type) -> size_t {
    size_t result = 0;
    for (const auto& field : *record_type->fields()) {
      const auto* current = field->type_nested_root();
      while (const auto* alias = current->type_as_alias_type_v0())
        current = alias->type_nested_root();
      if (const auto* record = current->type_as_record_type_v0())
        result += impl(impl, record);
      else
        result += 1;
    }
    return result;
  };
  return impl(impl, concrete_table());
}

record_field record_type::field(size_t index) const noexcept {
  const auto* field = concrete_table()->fields()->Get(index);
  VAST_ASSERT(field->type_nested_root()->type_type()
              == fbs::type::Type::alias_type_v0);
  return {
    field->name()->string_view(),
    type{as_bytes(*field->type()), *this},
  };
};

// -- explicit template instantiations -----------------------------------------

template class detail::concrete_type<fbs::type::record_type::v0>;

} // namespace vast::experimental
