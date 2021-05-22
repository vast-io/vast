//    _   _____   __________
//   | | / / _ | / __/_  __/     Visibility
//   | |/ / __ |_\ \  / /          Across
//   |___/_/ |_/___/ /_/       Space and Time
//
// SPDX-FileCopyrightText: (c) 2021 The VAST Contributors
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "vast/chunk.hpp"
#include "vast/fbs/type.hpp"

#include <caf/detail/apply_args.hpp>
#include <caf/detail/int_list.hpp>
#include <caf/meta/omittable.hpp>
#include <caf/sum_type_access.hpp>
#include <caf/sum_type_token.hpp>

#include <type_traits>

namespace vast::experimental {

// TODO: Move these into fwd.hpp.
class type;
class none_type;
class bool_type;
class integer_type;
class count_type;
class real_type;
class duration_type;
class time_type;
class string_type;
class pattern_type;
class address_type;
class subnet_type;
class enumeration_type;
class list_type;
class map_type;
class alias_type;
class record_type;

/// The top-level type.
class type {
public:
  // -- constructors, destructors, and assignment operators --------------------

  /// Construct a type from a FlatBuffers table.
  explicit type(chunk_ptr&& table) noexcept;

  /// Construct a type from a nested \ref fbs::type::Type FlatBuffers table,
  /// sharing lifetime with a surrounding \ref chunk.
  /// @pre context != nullptr
  /// @pre table.begin() >= context->begin()
  /// @pre table.end() <= context->end()
  type(span<const std::byte> table, const chunk_ptr& context) noexcept;

  /// Construct a nested type from a \ref fbs::type::Type FlatBuffers table.
  /// @pre table.begin() >= as_bytes(context).begin()
  /// @pre table.end() <= as_bytes(context).end()
  type(span<const std::byte> table, const type& context) noexcept;

  /// Default-constructs a type, which is semantically equal to a @ref none_type.
  type() noexcept;

  type(const type&);
  type& operator=(const type&);
  type(type&&) noexcept;
  type& operator=(type&&) noexcept;
  ~type() noexcept;

  // -- helper functions -------------------------------------------------------

  /// Checks if \param x is a *basic* type.
  friend bool is_basic(const type& x) noexcept;

  /// Checks if \param x is a *complex* type.
  friend bool is_complex(const type& x) noexcept;

  /// Checks if \param x is a *recursive* type.
  friend bool is_recursive(const type& x) noexcept;

  /// Checks if \param x is a *container* type.
  friend bool is_container(const type& x) noexcept;

  // -- comparison operators ---------------------------------------------------

  friend bool operator==(const type& lhs, const type& rhs) noexcept;
  friend bool operator!=(const type& lhs, const type& rhs) noexcept;
  friend bool operator<(const type& lhs, const type& rhs) noexcept;
  friend bool operator<=(const type& lhs, const type& rhs) noexcept;
  friend bool operator>(const type& lhs, const type& rhs) noexcept;
  friend bool operator>=(const type& lhs, const type& rhs) noexcept;

  // -- concepts ---------------------------------------------------------------

  /// Returns a view on the underlying representation.
  /// @note The representation of a default-initialized and a none_type is
  /// identical.
  friend span<const std::byte> as_bytes(const class type& type);

  /// Opt-in to CAF's type inspection API.
  template <class Inspector>
  friend typename Inspector::result_type
  inspect(Inspector& f, class type& type) {
    return f(caf::meta::type_name("vast.type"), caf::meta::omittable(),
             type.table_);
  }

  // -- sum type access --------------------------------------------------------

  /// The list of all possible concrete types.
  /// @note The order of this list must be an exact match of the order in the
  /// \ref fbs::type::Type enumeration.
  using concrete_types = caf::detail::type_list<
    none_type, bool_type, integer_type, count_type, real_type, duration_type,
    time_type, string_type, pattern_type, address_type, subnet_type,
    enumeration_type, list_type, map_type, alias_type, record_type>;

  friend struct ::caf::sum_type_access<type>;

  // -- implementation details -------------------------------------------------
protected:
  /// TODO: Docs
  [[nodiscard]] const fbs::Type* table() const noexcept;

private:
  chunk_ptr table_ = nullptr;
};

namespace detail {

// TODO: Consider moving into vast/type/detail/concrete_type.hpp
template <class ConcreteTableType>
class concrete_type : public type {
public:
  using concrete_table_type = ConcreteTableType;

  inline static constexpr auto concrete_type_index
    = fbs::type::TypeTraits<concrete_table_type>::enum_value;

  concrete_type() noexcept = default;
  concrete_type(const concrete_type&) = default;
  concrete_type& operator=(const concrete_type&) = default;
  concrete_type(concrete_type&&) noexcept = default;
  concrete_type& operator=(concrete_type&&) noexcept = default;
  ~concrete_type() noexcept = default;

protected:
  explicit concrete_type(flatbuffers::FlatBufferBuilder& builder,
                         flatbuffers::Offset<concrete_table_type> offset)
    : type{make(builder, offset)} {
    // nop
  }

  [[nodiscard]] const concrete_table_type* concrete_table() const noexcept {
    if constexpr (concrete_type_index == fbs::type::Type::NONE)
      return table();
    else
      return table()->template type_as<concrete_table_type>();
  }

private:
  [[nodiscard]] static chunk_ptr
  make(flatbuffers::FlatBufferBuilder& builder,
       flatbuffers::Offset<concrete_table_type> offset) {
    builder.Finish(
      fbs::CreateType(builder, concrete_type_index, offset.Union()));
    return chunk::make(builder.Release());
  }
};

// TODO: Consider moving into vast/type/detail/concrete_basic_type.hpp
template <class ConcreteTableType>
class concrete_basic_type : public concrete_type<ConcreteTableType> {
public:
  using super = concrete_type<ConcreteTableType>;
  using super::concrete_type_index;
  using typename super::concrete_table_type;

  /// The fix byte size of a basic type.
  inline static constexpr size_t buffer_size = 32;

  explicit concrete_basic_type(flatbuffers::FlatBufferBuilder& builder)
    : super{builder, make(builder)} {
    VAST_ASSERT(as_bytes(*this).size() == buffer_size,
                "underlying byte representation of basic type does not match "
                "the expected size");
  }

  concrete_basic_type() noexcept {
    auto builder = flatbuffers::FlatBufferBuilder{buffer_size};
    *this = concrete_basic_type{builder};
  }

  concrete_basic_type(const concrete_basic_type&) = default;
  concrete_basic_type& operator=(const concrete_basic_type&) = default;
  concrete_basic_type(concrete_basic_type&&) noexcept = default;
  concrete_basic_type& operator=(concrete_basic_type&&) noexcept = default;
  ~concrete_basic_type() noexcept = default;

private:
  static flatbuffers::Offset<concrete_table_type>
  make(flatbuffers::FlatBufferBuilder& builder) {
    auto concrete_builder = typename concrete_table_type::Builder{builder};
    return concrete_builder.Finish();
  }
};

} // namespace detail

} // namespace vast::experimental

namespace std {

// Byte-wise hashing for types. The implementation is from Boost.hash_combine.
template <>
struct hash<vast::experimental::type> {
  auto operator()(const vast::experimental::type& type) const {
    const auto bytes = as_bytes(type);
    auto seed = bytes.size();
    for (const auto i : bytes)
      seed ^= static_cast<uint8_t>(i) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    return seed;
  }
};

template <>
struct equal_to<vast::experimental::type> : hash<vast::span<const std::byte>> {
  using is_transparent = void; // Opt-in to heterogenous lookups.

  template <class Lhs, class Rhs>
  constexpr bool operator()(const Lhs& lhs, const Rhs& rhs) const
    noexcept(noexcept(lhs == rhs)) {
    static_assert(
      std::conjunction_v<
        std::disjunction<std::is_same<Lhs, vast::type>,
                         caf::detail::tl_contains<
                           vast::experimental::type::concrete_types, Lhs>>,
        std::disjunction<std::is_same<Rhs, vast::type>,
                         caf::detail::tl_contains<
                           vast::experimental::type::concrete_types, Rhs>>>);
    return lhs == rhs;
  }
};

} // namespace std

namespace caf {

template <>
struct sum_type_access<vast::experimental::type> {
  using types = vast::experimental::type::concrete_types;
  using type0 = vast::experimental::none_type;

  static constexpr bool specialized = true;

  template <class T, int Pos>
  static bool is(const vast::experimental::type& x, sum_type_token<T, Pos>) {
    if (!x.table())
      return std::is_same_v<T, type0>;
    return x.table()->type_type() == T::concrete_type_index;
  }

  template <class T, int Pos>
  static const T&
  get(const vast::experimental::type& x, sum_type_token<T, Pos>) {
    return static_cast<const T&>(x);
  }

  template <class T, int Pos>
  static const T*
  get_if(const vast::experimental::type* x, sum_type_token<T, Pos> token) {
    if (x && is(*x, token))
      return static_cast<const T*>(x);
    return nullptr;
  }

  template <class Result, class Visitor, class... Ts>
  static Result
  apply(const vast::experimental::type& x, Visitor&& v, Ts&&... xs) {
    auto xs_as_tuple = std::forward_as_tuple(xs...);
    auto indices = caf::detail::get_indices(xs_as_tuple);
    return caf::detail::apply_args_suffxied(v, std::move(indices),
                                            std::move(xs_as_tuple), x);
  }
};

} // namespace caf
