//    _   _____   __________
//   | | / / _ | / __/_  __/     Visibility
//   | |/ / __ |_\ \  / /          Across
//   |___/_/ |_/___/ /_/       Space and Time
//
// SPDX-FileCopyrightText: (c) 2018 The VAST Contributors
// SPDX-License-Identifier: BSD-3-Clause

#include "vast/system/spawn_meta_index.hpp"

#include "vast/system/meta_index.hpp"
#include "vast/system/node.hpp"

#include <caf/typed_event_based_actor.hpp>

namespace vast::system {

caf::expected<caf::actor>
spawn_meta_index(node_actor::stateful_pointer<node_state> self,
                 spawn_arguments& args) {
  auto handle = self->spawn(meta_index);
  return caf::actor_cast<caf::actor>(handle);
}

} // namespace vast::system
