/******************************************************************************
 *                    _   _____   __________                                  *
 *                   | | / / _ | / __/_  __/     Visibility                   *
 *                   | |/ / __ |_\ \  / /          Across                     *
 *                   |___/_/ |_/___/ /_/       Space and Time                 *
 *                                                                            *
 * This file is part of VAST. It is subject to the license terms in the       *
 * LICENSE file found in the top-level directory of this distribution and at  *
 * http://vast.io/license. No part of VAST, including this file, may be       *
 * copied, modified, propagated, or distributed except according to the terms *
 * contained in the LICENSE file.                                             *
 ******************************************************************************/

// -- v1 includes --------------------------------------------------------------

#include "vast/msgpack_table_slice_builder.hpp"

#include "vast/fbs/table_slice.hpp"
#include "vast/fbs/utils.hpp"

// -- v0 includes --------------------------------------------------------------

#include "vast/detail/overload.hpp"
#include "vast/logger.hpp"
#include "vast/msgpack_table_slice.hpp"
#include "vast/msgpack_table_slice_builder.hpp"

#include <caf/make_copy_on_write.hpp>
#include <caf/make_counted.hpp>

#include <memory>

using namespace vast;

namespace vast {

namespace msgpack {

// We activate ADL for data_view here so that we can rely on existing recursive
// put functions defined in msgpack_builder.hpp.
template <class Builder>
[[nodiscard]] size_t put(Builder& builder, data_view v);

} // namespace msgpack

namespace v1 {

// -- constructors, destructors, and assignment operators ----------------------

msgpack_table_slice_builder::msgpack_table_slice_builder(
  record_type layout) noexcept
  : table_slice_builder(std::move(layout)), msgpack_builder_{buffer_} {
  // nop
}

msgpack_table_slice_builder::~msgpack_table_slice_builder() noexcept = default;

// -- properties ---------------------------------------------------------------

/// @returns the current number of rows in the table slice.
table_slice::size_type msgpack_table_slice_builder::rows() const noexcept {
  return detail::narrow_cast<table_slice::size_type>(offset_table_.size());
}

table_slice_encoding msgpack_table_slice_builder::encoding() const noexcept {
  return implementation_id;
}

void msgpack_table_slice_builder::reserve(table_slice::size_type num_rows) {
  offset_table_.reserve(num_rows);
}

// -- implementation details -------------------------------------------------

bool msgpack_table_slice_builder::add_impl(data_view x) {
  if (!type_check(layout().fields[column_].type, x))
    return false;
  if (column_ == 0)
    offset_table_.push_back(buffer_.size());
  column_ = (column_ + 1) % columns();
  auto n = put(msgpack_builder_, x);
  return n > 0;
};

caf::expected<chunk_ptr> msgpack_table_slice_builder::finish_impl() {
  // Create a `vast.fbs.table_slice.msgpack.v0` table slice.
  auto builder = fbs::table_slice::msgpack::v0Builder{fbb_};
  auto serialized_layout = fbs::serialize_bytes(fbb_, layout());
  if (!serialized_layout)
    return std::move(serialized_layout.error());
  builder.add_layout(*serialized_layout);
  builder.add_offset_table(fbb_.CreateVector(offset_table_));
  builder.add_data(fbb_.CreateVector(
    reinterpret_cast<const uint8_t*>(buffer_.data()), buffer_.size()));
  auto encoded_slice = builder.Finish();
  // Create the table slice and put it into a chunk.
  fbs::CreateTableSlice(fbb_, fbs::table_slice::TableSlice::msgpack_v0,
                        encoded_slice.Union());
  return fbs::release(fbb_);
}

void msgpack_table_slice_builder::reset_impl() {
  offset_table_ = {};
  buffer_ = {};
}

} // namespace v1

inline namespace v0 {

caf::atom_value msgpack_table_slice_builder::get_implementation_id() noexcept {
  return msgpack_table_slice::class_id;
}

table_slice_builder_ptr
msgpack_table_slice_builder::make(record_type layout,
                                  size_t initial_buffer_size) {
  return caf::make_counted<msgpack_table_slice_builder>(std::move(layout),
                                                        initial_buffer_size);
}

msgpack_table_slice_builder::msgpack_table_slice_builder(
  record_type layout, size_t initial_buffer_size)
  : super{std::move(layout)}, col_{0}, builder_{buffer_} {
  buffer_.reserve(initial_buffer_size);
}

msgpack_table_slice_builder::~msgpack_table_slice_builder() {
  // nop
}

namespace {

template <class Builder, class View>
size_t encode(Builder& builder, View v) {
  using namespace msgpack;
  auto f = detail::overload{
    [&](auto x) { return put(builder, x); },
    [&](view<duration> x) { return put(builder, x.count()); },
    [&](view<time> x) { return put(builder, x.time_since_epoch().count()); },
    [&](view<pattern> x) { return put(builder, x.string()); },
    [&](view<address> x) {
      auto ptr = reinterpret_cast<const char*>(x.data().data());
      if (x.is_v4()) {
        auto str = std::string_view{ptr + 12, 4};
        return builder.template add<fixstr>(str);
      } else {
        auto str = std::string_view{ptr, 16};
        return builder.template add<fixstr>(str);
      }
    },
    [&](view<subnet> x) {
      auto proxy = builder.template build<fixarray>();
      auto n = encode(proxy, make_view(x.network()));
      if (n == 0) {
        builder.reset();
        return n;
      }
      proxy.template add<uint8>(x.length());
      return builder.add(std::move(proxy));
    },
    [&](view<port> x) {
      auto proxy = builder.template build<fixarray>();
      proxy.template add<uint16>(x.number());
      proxy.template add<uint8>(static_cast<uint8_t>(x.type()));
      return builder.add(std::move(proxy));
    },
    [&](view<enumeration> x) {
      // The noop cast exists only to communicate the MsgPack type.
      return put(builder, static_cast<uint8_t>(x));
    },
    [&](view<list> xs) { return put_array(builder, *xs); },
    [&](view<map> xs) { return put_map(builder, *xs); },
    [&](view<record> xs) -> size_t {
      // We store records flattened, so we just append all values sequentially.
      size_t result = 0;
      for ([[maybe_unused]] auto [_, x] : xs) {
        auto n = put(builder, x);
        if (n == 0) {
          builder.reset();
          return 0;
        }
        result += n;
      }
      return result;
    },
  };
  return f(v);
}

} // namespace

bool msgpack_table_slice_builder::add_impl(data_view x) {
  // Check whether input is valid.
  if (!type_check(layout().fields[col_].type, x))
    return false;
  if (col_ == 0)
    offset_table_.push_back(buffer_.size());
  col_ = (col_ + 1) % columns();
  auto n = put(builder_, x);
  VAST_ASSERT(n > 0);
  return true;
}

table_slice_ptr msgpack_table_slice_builder::finish() {
  // Sanity check.
  if (col_ != 0)
    return nullptr;
  table_slice_header header;
  header.layout = layout();
  header.rows = offset_table_.size();
  auto ptr = new msgpack_table_slice{std::move(header)};
  ptr->offset_table_ = std::move(offset_table_);
  ptr->chunk_ = chunk::make(std::move(buffer_));
  ptr->buffer_ = as_bytes(span{ptr->chunk_->data(), ptr->chunk_->size()});
  offset_table_ = {};
  buffer_ = {};
  return table_slice_ptr{ptr, false};
}

size_t msgpack_table_slice_builder::rows() const noexcept {
  return offset_table_.size();
}

caf::atom_value
msgpack_table_slice_builder::implementation_id() const noexcept {
  return get_implementation_id();
}

} // namespace v0

namespace msgpack {

// We activate ADL for data_view here so that we can rely on existing recursive
// put functions defined in msgpack_builder.hpp.
template <class Builder>
[[nodiscard]] size_t put(Builder& builder, data_view v) {
  return caf::visit([&](auto&& x) { return encode(builder, x); }, v);
}

} // namespace msgpack

} // namespace vast
