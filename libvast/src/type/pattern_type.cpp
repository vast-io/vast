//    _   _____   __________
//   | | / / _ | / __/_  __/     Visibility
//   | |/ / __ |_\ \  / /          Across
//   |___/_/ |_/___/ /_/       Space and Time
//
// SPDX-FileCopyrightText: (c) 2021 The VAST Contributors
// SPDX-License-Identifier: BSD-3-Clause

#include "vast/type/pattern_type.hpp"

namespace vast::experimental {

// -- explicit template instantiations -----------------------------------------

template class detail::concrete_type<fbs::type::pattern_type::v0>;
template class detail::concrete_basic_type<fbs::type::pattern_type::v0>;

} // namespace vast::experimental
