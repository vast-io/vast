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

#pragma once

#include "vast/fwd.hpp"
#include "vast/meta_index.hpp"
#include "vast/system/accountant.hpp"
#include "vast/system/query_supervisor_master_actor.hpp"

#include <caf/config_value.hpp>
#include <caf/typed_event_based_actor.hpp>

#include <memory>

namespace vast::system {

/// The INDEX actor interface.
using index_actor = caf::typed_actor<
  // FIXME: docs
  caf::replies_to<atom::status, status_verbosity>::with< //
    caf::config_value::dictionary>,
  // FIXME: docs
  caf::reacts_to<atom::done, uuid>,
  // FIXME: docs
  caf::replies_to<caf::stream<table_slice>>::with<
    caf::inbound_stream_slot<table_slice>>,
  // FIXME: docs
  caf::reacts_to<accountant_actor>,
  // FIXME: docs
  caf::reacts_to<atom::subscribe, atom::flush, wrapped_flush_listener>,
  // FIXME: docs
  caf::reacts_to<expression>,
  // FIXME: docs
  caf::reacts_to<uuid, uint32_t>,
  // FIXME: docs
  caf::reacts_to<atom::replace, uuid, std::shared_ptr<partition_synopsis>>,
  // FIXME: docs
  caf::replies_to<atom::erase, uuid>::with<ids>>
  // Conform to the protocol of the query supervisor master actor.
  ::extend_with<query_supervisor_master_actor>;

} // namespace vast::system