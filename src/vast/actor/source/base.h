#ifndef VAST_ACTOR_SOURCE_BASE_H
#define VAST_ACTOR_SOURCE_BASE_H

#include "vast/event.h"
#include "vast/result.h"
#include "vast/actor/accountant.h"
#include "vast/actor/atoms.h"
#include "vast/actor/basic_state.h"
#include "vast/concept/printable/vast/error.h"
#include "vast/concept/printable/vast/time.h"
#include "vast/util/assert.h"

namespace vast {
namespace source {

/// The base class for derived states which extract events.
struct base_state : basic_state {
  base_state(local_actor* self, char const* name)
    : basic_state{self, name} {
  }

  ~base_state() {
    if (events_.empty())
      return;
    VAST_DEBUG_AT(self, "sends", events_.size(), "last events");
    self->send(sinks_[next_sink_++ % sinks_.size()], std::move(events_));
  }

  virtual vast::schema schema() = 0;
  virtual void schema(vast::schema const& sch) = 0;
  virtual result<event> extract() = 0;

  static constexpr size_t max_batch_size = 1 << 20;

  bool done_ = false;
  bool paused_ = false;
  accountant::actor_type accountant_;
  std::vector<actor> sinks_;
  size_t next_sink_ = 0;
  uint64_t batch_size_ = 65536;
  std::vector<event> events_;
  time::moment start_;
};

/// The base actor for sources.
/// @param self The actor handle.
template <typename State>
behavior base(stateful_actor<State>* self) {
  using caf::actor;
  return {
    [=](down_msg const& msg) {
      // Handle sink termination.
      auto sink = std::find_if(
        self->state.sinks_.begin(),
        self->state.sinks_.end(),
        [&](auto& x) { return msg.source == x; }
      );
      if (sink != self->state.sinks_.end())
        self->state.sinks_.erase(sink);
      if (self->state.sinks_.empty()) {
        VAST_WARN_AT(self, "has no more sinks");
        self->quit(exit::done);
      }
    },
    [=](overload_atom, actor const& victim) {
      VAST_DEBUG_AT(self, "got OVERLOAD from", victim);
      self->state.paused_ = true; // Stop after the next batch.
    },
    [=](underload_atom, actor const& victim) {
      VAST_DEBUG_AT(self, "got UNDERLOAD from", victim);
      self->state.paused_ = false;
      if (!self->state.done_)
        self->send(self, run_atom::value);
    },
    [=](batch_atom, uint64_t batch_size) {
      if (batch_size > base_state::max_batch_size) {
        VAST_ERROR_AT(self, "got too large batch size:", batch_size);
        self->quit(exit::error);
        return;
      }
      VAST_DEBUG_AT(self, "sets batch size to", batch_size);
      self->state.batch_size_ = batch_size;
      self->state.events_.reserve(batch_size);
    },
    [=](get_atom, schema_atom) {
      return self->state.schema();
    },
    [=](put_atom, schema const& sch) {
      self->state.schema(sch);
    },
    [=](put_atom, sink_atom, actor const& sink) {
      VAST_DEBUG_AT(self, "adds sink to", sink);
      self->monitor(sink);
      self->state.sinks_.push_back(sink);
    },
    [=](accountant::actor_type acc) {
      VAST_DEBUG_AT(self, "registers accountant#" << acc->id());
      self->state.accountant_ = acc;
      auto label = self->state.name + '#' + to_string(self->id()) + "-events";
      self->send(self->state.accountant_, std::move(label), time::now());
    },
    [=](get_atom, sink_atom) { return self->state.sinks_; },
    [=](run_atom) {
      if (self->current_sender() != self)
        self->state.start_ = time::snapshot();
      if (self->state.sinks_.empty()) {
        VAST_ERROR_AT(self, "cannot run without sinks");
        self->quit(exit::error);
        return;
      }
      while (self->state.events_.size() < self->state.batch_size_
             && !self->state.done_) {
        auto r = self->state.extract();
        if (r) {
          self->state.events_.push_back(std::move(*r));
        } else if (r.failed()) {
          VAST_ERROR_AT(self, r.error());
          self->state.done_ = true;
          break;
        }
      }
      if (!self->state.events_.empty()) {
        auto stop = time::snapshot();
        VAST_VERBOSE_AT(self, "produced", self->state.events_.size(),
                        "events in", stop - self->state.start_);
        if (self->state.accountant_)
          self->send(self->state.accountant_,
                     uint64_t{self->state.events_.size()}, stop);
        auto next = self->state.next_sink_++ % self->state.sinks_.size();
        self->send(self->state.sinks_[next], std::move(self->state.events_));
        self->state.events_ = {};
        self->state.events_.reserve(self->state.batch_size_);
        self->state.start_ = time::snapshot();
        // FIXME: if we do not give the stdlib implementation a hint to yield
        // here, this actor can monopolize all available resources. In
        // particular, we encountered a scenario where it prevented the BASP
        // broker from a getting a chance to operate, thereby queuing up
        // all event batches locally and running out of memory, as opposed to
        // sending them out as soon as possible. This yield fix temporarily
        // works around a deeper issue in CAF, which needs to be addressed in
        // the future.
        std::this_thread::yield();
      }
      if (self->state.done_)
        self->quit(exit::done);
      else if (!self->state.paused_)
        self->send(self, self->current_message());
    },
    quit_on_others(self),
  };
}

} // namespace source
} // namespace vast

#endif
