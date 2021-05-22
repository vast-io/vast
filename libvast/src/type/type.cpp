//    _   _____   __________
//   | | / / _ | / __/_  __/     Visibility
//   | |/ / __ |_\ \  / /          Across
//   |___/_/ |_/___/ /_/       Space and Time
//
// SPDX-FileCopyrightText: (c) 2021 The VAST Contributors
// SPDX-License-Identifier: BSD-3-Clause

#include "vast/type/type.hpp"

#include "vast/chunk.hpp"
#include "vast/die.hpp"
#include "vast/fbs/type.hpp"
#include "vast/type/none_type.hpp"

namespace vast::experimental {

namespace {

// We must special-case a default-initialized type, as is is semantically
// equivalent to a none_type. This is cheaper than having the default
// constructor of type create an actual none_type.
const auto default_init_representation = none_type{};

} // namespace

// -- constructors, destructors, and assignment operators ----------------------

type::type(chunk_ptr&& table) noexcept : table_{std::move(table)} {
  // nop
}

type::type(span<const std::byte> table, const chunk_ptr& context) noexcept
  : table_{context->slice(table)} {
  // nop
}

type::type(span<const std::byte> table, const type& context) noexcept
  : type{table, context.table_} {
  // nop
}

type::type() noexcept = default;
type::type(const type&) = default;
type& type::operator=(const type&) = default;
type::type(type&&) noexcept = default;
type& type::operator=(type&&) noexcept = default;
type::~type() noexcept = default;

// -- helper functions ---------------------------------------------------------

bool is_basic(const type& x) noexcept {
  switch (x.table()->type_type()) {
    case fbs::type::Type::NONE:
      return false;
    case fbs::type::Type::bool_type_v0:
    case fbs::type::Type::integer_type_v0:
    case fbs::type::Type::count_type_v0:
    case fbs::type::Type::real_type_v0:
    case fbs::type::Type::duration_type_v0:
    case fbs::type::Type::time_type_v0:
    case fbs::type::Type::string_type_v0:
    case fbs::type::Type::pattern_type_v0:
    case fbs::type::Type::address_type_v0:
    case fbs::type::Type::subnet_type_v0:
      return true;
    case fbs::type::Type::enumeration_type_v0:
    case fbs::type::Type::list_type_v0:
    case fbs::type::Type::map_type_v0:
    case fbs::type::Type::alias_type_v0:
    case fbs::type::Type::record_type_v0:
      return false;
  }
  die("unreachable");
}

bool is_complex(const type& x) noexcept {
  switch (x.table()->type_type()) {
    case fbs::type::Type::NONE:
    case fbs::type::Type::bool_type_v0:
    case fbs::type::Type::integer_type_v0:
    case fbs::type::Type::count_type_v0:
    case fbs::type::Type::real_type_v0:
    case fbs::type::Type::duration_type_v0:
    case fbs::type::Type::time_type_v0:
    case fbs::type::Type::string_type_v0:
    case fbs::type::Type::pattern_type_v0:
    case fbs::type::Type::address_type_v0:
    case fbs::type::Type::subnet_type_v0:
      return false;
    case fbs::type::Type::enumeration_type_v0:
    case fbs::type::Type::list_type_v0:
    case fbs::type::Type::map_type_v0:
    case fbs::type::Type::alias_type_v0:
    case fbs::type::Type::record_type_v0:
      return true;
  }
  die("unreachable");
}

bool is_recursive(const type& x) noexcept {
  switch (x.table()->type_type()) {
    case fbs::type::Type::NONE:
    case fbs::type::Type::bool_type_v0:
    case fbs::type::Type::integer_type_v0:
    case fbs::type::Type::count_type_v0:
    case fbs::type::Type::real_type_v0:
    case fbs::type::Type::duration_type_v0:
    case fbs::type::Type::time_type_v0:
    case fbs::type::Type::string_type_v0:
    case fbs::type::Type::pattern_type_v0:
    case fbs::type::Type::address_type_v0:
    case fbs::type::Type::subnet_type_v0:
    case fbs::type::Type::enumeration_type_v0:
      return false;
    case fbs::type::Type::list_type_v0:
    case fbs::type::Type::map_type_v0:
    case fbs::type::Type::alias_type_v0:
    case fbs::type::Type::record_type_v0:
      return true;
  }
  die("unreachable");
}

bool is_container(const type& x) noexcept {
  switch (x.table()->type_type()) {
    case fbs::type::Type::NONE:
    case fbs::type::Type::bool_type_v0:
    case fbs::type::Type::integer_type_v0:
    case fbs::type::Type::count_type_v0:
    case fbs::type::Type::real_type_v0:
    case fbs::type::Type::duration_type_v0:
    case fbs::type::Type::time_type_v0:
    case fbs::type::Type::string_type_v0:
    case fbs::type::Type::pattern_type_v0:
    case fbs::type::Type::address_type_v0:
    case fbs::type::Type::subnet_type_v0:
    case fbs::type::Type::enumeration_type_v0:
      return false;
    case fbs::type::Type::list_type_v0:
    case fbs::type::Type::map_type_v0:
      return true;
    case fbs::type::Type::alias_type_v0:
      return false;
    case fbs::type::Type::record_type_v0:
      return true;
  }
  die("unreachable");
}

// -- comparison operators -----------------------------------------------------

bool operator==(const type& lhs, const type& rhs) noexcept {
  return as_bytes(lhs) == as_bytes(rhs);
}

bool operator!=(const type& lhs, const type& rhs) noexcept {
  return as_bytes(lhs) != as_bytes(rhs);
}

bool operator<(const type& lhs, const type& rhs) noexcept {
  return as_bytes(lhs) < as_bytes(rhs);
}

bool operator<=(const type& lhs, const type& rhs) noexcept {
  return as_bytes(lhs) <= as_bytes(rhs);
}

bool operator>(const type& lhs, const type& rhs) noexcept {
  return as_bytes(lhs) > as_bytes(rhs);
}

bool operator>=(const type& lhs, const type& rhs) noexcept {
  return as_bytes(lhs) >= as_bytes(rhs);
}

// -- sum type access ----------------------------------------------------------

static_assert(caf::detail::tl_size<type::concrete_types>::value
                == static_cast<std::underlying_type_t<fbs::type::Type>>(
                     fbs::type::Type::MAX)
                     + 1,
              "number of concrete types does not match number of types in "
              "FlatBuffers schema");

// -- concepts -----------------------------------------------------------------

/// Returns a view on the underlying representation.
/// @note The representation of a default-initialized and a none_type is
/// identical.
span<const std::byte> as_bytes(const class type& type) {
  return type.table_ ? as_bytes(type.table_)
                     : as_bytes(default_init_representation);
}

// -- implementation details ---------------------------------------------------

[[nodiscard]] const fbs::Type* type::table() const noexcept {
  return table_ ? fbs::GetType(table_->data())
                : default_init_representation.table();
}

} // namespace vast::experimental
