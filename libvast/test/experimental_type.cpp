//    _   _____   __________
//   | | / / _ | / __/_  __/     Visibility
//   | |/ / __ |_\ \  / /          Across
//   |___/_/ |_/___/ /_/       Space and Time
//
// SPDX-FileCopyrightText: (c) 2016 The VAST Contributors
// SPDX-License-Identifier: BSD-3-Clause

#define SUITE experimental_type

#include "vast/logger.hpp" // detail::pretty_type_name
#include "vast/test/test.hpp"
#include "vast/type/all.hpp"

namespace vast::experimental {

namespace {

template <class Expected>
void check_holds_alternative(const type& type) {
  CHECK_EQUAL(caf::holds_alternative<none_type>(type),
              (std::is_same_v<Expected, none_type>));
  CHECK_EQUAL(caf::holds_alternative<bool_type>(type),
              (std::is_same_v<Expected, bool_type>));
  CHECK_EQUAL(caf::holds_alternative<integer_type>(type),
              (std::is_same_v<Expected, integer_type>));
  CHECK_EQUAL(caf::holds_alternative<count_type>(type),
              (std::is_same_v<Expected, count_type>));
  CHECK_EQUAL(caf::holds_alternative<real_type>(type),
              (std::is_same_v<Expected, real_type>));
  CHECK_EQUAL(caf::holds_alternative<duration_type>(type),
              (std::is_same_v<Expected, duration_type>));
  CHECK_EQUAL(caf::holds_alternative<time_type>(type),
              (std::is_same_v<Expected, time_type>));
  CHECK_EQUAL(caf::holds_alternative<string_type>(type),
              (std::is_same_v<Expected, string_type>));
  CHECK_EQUAL(caf::holds_alternative<pattern_type>(type),
              (std::is_same_v<Expected, pattern_type>));
  CHECK_EQUAL(caf::holds_alternative<address_type>(type),
              (std::is_same_v<Expected, address_type>));
  CHECK_EQUAL(caf::holds_alternative<subnet_type>(type),
              (std::is_same_v<Expected, subnet_type>));
  CHECK_EQUAL(caf::holds_alternative<enumeration_type>(type),
              (std::is_same_v<Expected, enumeration_type>));
  CHECK_EQUAL(caf::holds_alternative<list_type>(type),
              (std::is_same_v<Expected, list_type>));
  CHECK_EQUAL(caf::holds_alternative<map_type>(type),
              (std::is_same_v<Expected, map_type>));
  CHECK_EQUAL(caf::holds_alternative<alias_type>(type),
              (std::is_same_v<Expected, alias_type>));
  CHECK_EQUAL(caf::holds_alternative<record_type>(type),
              (std::is_same_v<Expected, record_type>));
}

} // namespace

TEST(concrete basic type construction) {
  auto run = [](auto&& concrete_basic_type) {
    MESSAGE(::vast::detail::pretty_type_name(concrete_basic_type));
    const auto t
      = type{std::forward<decltype(concrete_basic_type)>(concrete_basic_type)};
    using expected_type
      = std::remove_reference_t<decltype(concrete_basic_type)>;
    check_holds_alternative<expected_type>(t);
    CHECK_EQUAL(as_bytes(t).size(), expected_type::buffer_size);
  };
  run(bool_type{});
  run(integer_type{});
  run(count_type{});
  run(real_type{});
  run(duration_type{});
  run(time_type{});
  run(string_type{});
  run(pattern_type{});
  run(address_type{});
  run(subnet_type{});
}

TEST(assignment) {
  auto t = type{string_type{}};
  CHECK(caf::holds_alternative<string_type>(t));
  t = real_type{};
  CHECK(caf::holds_alternative<real_type>(t));
  t = {};
  CHECK(!caf::holds_alternative<real_type>(t));
  auto u = type{none_type{}};
  CHECK(caf::holds_alternative<none_type>(u));
}

TEST(copying) {
  auto t = type{string_type{}};
  auto u = t; // NOLINT(performance-unnecessary-copy-initialization)
  CHECK(caf::holds_alternative<string_type>(u));
}

TEST(none_type) {
  const auto t = type{};
  CHECK_EQUAL(t, type{});
  check_holds_alternative<none_type>(t);
  const auto nt = none_type{};
  check_holds_alternative<none_type>(nt);
  CHECK_EQUAL(t, nt);
  CHECK_EQUAL(as_bytes(t), as_bytes(nt));
  CHECK_EQUAL(as_bytes(nt).size(), none_type::buffer_size);
}

// TODO: Add tests for complex types.

TEST_DISABLED(attributes) {
  // auto attrs = std::vector<attribute>{{"key", "value"}};
  // type t;
  // t.attributes(attrs);
  // CHECK(t.attributes().empty());
  // t = string_type{};
  // t.attributes({{"key", "value"}});
  // CHECK_EQUAL(t.attributes(), attrs);
}

TEST(equality comparison) {
  MESSAGE("type-erased comparison");
  CHECK_EQUAL(type{}, type{});
  CHECK_NOT_EQUAL(type{bool_type{}}, type{});
  CHECK_EQUAL(type{bool_type{}}, type{bool_type{}});
  CHECK_NOT_EQUAL(type{bool_type{}}, type{real_type{}});
  auto x = type{string_type{}};
  auto y = type{string_type{}};
  x = alias_type{"foo", x};
  CHECK_NOT_EQUAL(x, y);
  y = alias_type{"foo", y};
  CHECK_EQUAL(x, y);
  MESSAGE("concrete type comparison");
  CHECK_EQUAL(real_type{}, real_type{});
  CHECK_NOT_EQUAL((alias_type{"foo", real_type{}}), real_type{});
  CHECK_EQUAL((alias_type{"foo", real_type{}}),
              (alias_type{"foo", real_type{}}));
  // auto attrs = std::vector<attribute>{{"key", "value"}};
  // CHECK(real_type{}.attributes(attrs) != real_type{});
  // CHECK(real_type{}.attributes(attrs) == real_type{}.attributes(attrs));
}

TEST(less than comparison) {
  CHECK(!(type{} < type{}));
  CHECK(!(real_type{} < real_type{}));
  CHECK_LESS((alias_type{"a", string_type{}}),
             (alias_type{"b", string_type{}}));
}

TEST(strict weak ordering) {
  std::vector<type> xs{string_type{}, address_type{}, pattern_type{}};
  std::vector<type> ys{string_type{}, pattern_type{}, address_type{}};
  std::sort(xs.begin(), xs.end());
  std::sort(ys.begin(), ys.end());
  CHECK(xs == ys);
}

} // namespace vast::experimental
