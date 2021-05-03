//    _   _____   __________
//   | | / / _ | / __/_  __/     Visibility
//   | |/ / __ |_\ \  / /          Across
//   |___/_/ |_/___/ /_/       Space and Time
//
// SPDX-FileCopyrightText: (c) 2021 The VAST Contributors
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "vast/fwd.hpp"

#include "vast/data.hpp"
#include "vast/detail/concepts.hpp"
#include "vast/detail/narrow.hpp"
#include "vast/detail/type_traits.hpp"
#include "vast/error.hpp"
#include "vast/logger.hpp"
#include "vast/type.hpp"

#include <caf/error.hpp>

#include <concepts>

/// Assigns fields from `src` to `dst`.
/// The source must have a structure that matches the destination.
/// For example:
/// auto xs = record{               | struct foo {
///   {"a", "foo"},                 |   std::string a;
///   {"b", record{                 |   struct {
///     {"c", -42},                 |     integer c;
///     {"d", list{1, 2, 3}}        |     std::vector<count> d;
///   }},                           |   } b;
///   {"e", record{                 |   struct {
///     {"f", caf::none},           |     integer f;
///     {"g", caf::none},           |     std::optional<count> g;
///   }},                           |   } e;
///   {"h", true}                   |   bool h;
/// };                              | };
///
/// If a member of `out` is missing in `in`, the value does not get overwritten,
/// Similarly, data in `in` that does not match `out` is ignored.
/// NOTE: The overload for `data` is defined last for reasons explained there.

namespace vast {

/// Checks if `from` can be converted to `to`, i.e. whether a viable overload
/// of / `convert(from, to)` exists.
// NOTE: You might wonder why this is defined as a macro instead of a concept.
// The issue here is that we're looking for overloads of a non-member function,
// and the rules for name lookup for free functions mandate that regular
// qualified or unqualified lookup is done when the definitoin of the template
// is parsed, and only dependent lookup is done in the instantiation phase in
// case the call is unqualified. That means a concept would only be able to
// detect overloads that are declard later iff they happen to be in the same
// namespace as their arguments, but it won't pick up overloads like
// `convert(std::string, std::string)` or `convert(uint64_t, uint64_t)`.
// https://timsong-cpp.github.io/cppwp/n4868/temp.res#temp.dep.candidate
// It would be preferable to forward declare `is_concrete_convertible` but that
// is not allowed.
// We don't do this for the is_convertible trait yet because clang 11 doesn't
// support ad-hoc requires constraints, and we only need this detection for
// `if constexpr` predicates here.
#define IS_TYPED_CONVERTIBLE(type, from, to)                                   \
  requires {                                                                   \
    { vast::convert(type, from, to) } -> std::same_as<caf::error>;             \
  }

template <class T>
concept has_layout = requires {
  std::same_as<decltype(T::layout), record_type>;
};

// Overload for records.
template <detail::is_inspectable T>
caf::error convert(const record_type& layout, const record& src, T& dst);

template <has_layout T>
caf::error convert(const record& src, T& dst);

// Dispatch to standard conversion.
// clang-format off
template <class Type, class From, class To>
  requires (!std::same_as<From, To>) && std::convertible_to<From, To>
caf::error convert(const Type&, const From& src, To& dst) {
  dst = src;
  return caf::none;
}
// clang-format on

// Generic overload when `src` and `dst` are of the same type.
template <class Type, class T>
caf::error convert(const Type&, const T& src, T& dst) {
  dst = src;
  return caf::none;
}

// Overload for enums.
// clang-format off
template <class To>
  requires std::is_enum_v<To>
caf::error convert(const enumeration_type& t, const std::string& src, To& dst) {
  auto i = std::find(t.fields.begin(), t.fields.end(), src);
  if (i == t.fields.end())
    return caf::make_error(ec::convert_error, "{} is not a value of {}", src,
        detail::pretty_type_name(dst));
  dst = detail::narrow_cast<To>(std::distance(t.fields.begin(), i));
  return caf::none;
}
// clang-format on

// Overload for lists.
template <detail::has_push_back To>
caf::error convert(const list_type& t, const list& src, To& dst) {
  for (const auto& element : src) {
    typename To::value_type v{};
    if (auto err = convert(t.value_type, element, v))
      return err;
    dst.push_back(std::move(v));
  }
  return caf::none;
}

// Overload for maps.
// clang-format off
template <detail::has_insert To>
  requires requires {
    typename To::key_type;
    typename To::mapped_type;
  }
caf::error convert(const map_type& t, const map& src, To& dst) {
  for (const auto& [key, value] : src) {
    typename To::value_type v{};
    if (auto err = convert(t.key_type, key, v.first))
      return err;
    if (auto err = convert(t.value_type, value, v.second))
      return err;
    dst.insert(std::move(v));
  }
  return caf::none;
}
// clang-format on

// Overload for maps.
// clang-format off
template <detail::has_insert To>
  requires requires {
    typename To::key_type;
    typename To::mapped_type;
  }
caf::error convert(const map_type& t, const record& src, To& dst) {
  for (const auto& [key, value] : src) {
    typename To::value_type v{};
    if (auto err = convert(t.key_type, key, v.first))
      return err;
    if (auto err = convert(t.value_type, value, v.second))
      return err;
    dst.insert(std::move(v));
  }
  return caf::none;
}
// clang-format on

// Overload for integers.
template <std::signed_integral To>
caf::error convert(const integer_type&, const integer& i, To& dst) {
  if constexpr (sizeof(To) >= sizeof(integer::value)) {
    dst = i.value;
  } else {
    if (i.value < std::numeric_limits<To>::min()
        || i.value > std::numeric_limits<To>::max())
      return caf::make_error(ec::convert_error,
                             fmt::format("{} can not be represented by the "
                                         "target variable [{}, {}]",
                                         i, std::numeric_limits<To>::min(),
                                         std::numeric_limits<To>::max()));
    dst = detail::narrow_cast<To>(i.value);
  }
  return caf::none;
}

class record_inspector {
public:
  template <class To>
  caf::error apply(const record_type::each::range_state& f, To& dst) {
    // Find the value from the record
    const auto* src_section = &src;
    auto it = src.end();
    size_t depth = 0;
    for (; depth < f.depth() - 1; depth++) {
      const auto* node = f.trace[depth];
      it = src_section->find(node->name);
      if (it == src_section->end())
        return caf::none;
      src_section = caf::get_if<record>(&it->second);
      if (!src_section)
        return caf::make_error(ec::convert_error, "{} is of unexpected type {}",
                               node->name, it->second);
    }
    it = src_section->find(f.trace[depth]->name);
    if (it == src_section->end())
      return caf::none;
    return caf::visit(
      [&](const auto& t) -> caf::error {
        using concrete_type = std::decay_t<decltype(t)>;
        using concrete_data
          = std::conditional_t<std::is_same_v<concrete_type, enumeration_type>,
                               std::string, type_to_data<concrete_type>>;
        if constexpr (detail::is_any_v<concrete_type, alias_type, none_type>) {
          // Data conversion of none or alias type does not make sense.
          return caf::make_error(ec::convert_error, "can't convert alias or "
                                                    "none types");
        } else {
          const auto* d = caf::get_if<concrete_data>(&it->second);
          if (!d) {
            if (caf::holds_alternative<caf::none_t>(it->second)) {
              // TODO: Consider in-place default construction instead.
              if constexpr (std::is_default_constructible_v<To> //
                            && std::is_move_assignable_v<To>)
                dst = To{};
              return caf::none;
            }
            return caf::make_error(
              ec::convert_error, fmt::format("unexpected data format at {}: {}",
                                             f.key(), it->second));
          }
          if constexpr (IS_TYPED_CONVERTIBLE(t, *d, dst))
            return convert(t, *d, dst);
          else
            return caf::make_error(
              ec::convert_error, fmt::format("R can't convert from {} to {} "
                                             "with type {}",
                                             detail::pretty_type_name(*d),
                                             detail::pretty_type_name(dst), t));
        }
        return caf::none;
      },
      f.type());
  }

  template <class... Ts>
  caf::error operator()(Ts&... xs) {
    return apply_all(std::index_sequence_for<Ts...>{}, xs...);
  }

  template <class... Ts, size_t... Is>
  caf::error apply_all(std::index_sequence<Is...>, Ts&... xs) {
    const auto& rng = record_type::each(layout);
    auto it = rng.begin();
    return caf::error::eval([&] {
      return apply(*it++, xs);
    }...);
  }

  const record_type& layout;
  const record& src;
};

// Overload for records.
template <detail::is_inspectable To>
caf::error convert(const record_type& layout, const record& src, To& dst) {
  if (layout.fields.empty()) {
    if constexpr (has_layout<To>)
      return convert(src, dst);
    else
      return caf::make_error(ec::convert_error,
                             "destination types must have a "
                             "static layout definition: {}",
                             detail::pretty_type_name(dst));
  }
  auto ri = record_inspector{layout, src};
  return inspect(ri, dst);
}

template <has_layout To>
caf::error convert(const record& src, To& dst) {
  return convert(dst.layout, src, dst);
}

// A concept to detect whether any previously declared overloads of
// `convert` can be used for a combination of `Type`, `From`, and `To`.
template <class Type, class From, class To>
concept is_concrete_convertible
  = requires(const Type& type, const From& src, To& dst) {
  { vast::convert(type, src, dst) } -> std::same_as<caf::error>;
};

// NOTE: This overload has to be last because we need to be able to detect
// all other overloads with `is_concrete_convertible`. At the same time,
// it must not be declared before to prevent recursing into itself because
// of the non-explicit constructor of `data`.
template <class To>
caf::error convert(const type& t, const data& src, To& dst) {
  return caf::visit(
    [&](const auto& t, const auto& x) {
      if constexpr (is_concrete_convertible<decltype(t), decltype(x), To>)
        return convert(t, x, dst);
      else
        return caf::make_error(ec::convert_error,
                               fmt::format("can't convert from {} to {} with "
                                           "type {}",
                                           detail::pretty_type_name(x),
                                           detail::pretty_type_name(dst), t));
    },
    t, src);
}

} // namespace vast
