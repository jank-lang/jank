#pragma once

#include <folly/Synchronized.h>

#include <jtl/option.hpp>

#include <jank/runtime/object.hpp>

namespace jank::runtime::obj
{
  using future_ref = oref<struct future>;

  enum class future_status : u8
  {
    running,
    done,
    cancelled
  };

  struct future
  {
    static constexpr object_type obj_type{ object_type::future };
    static constexpr bool pointer_free{ false };

    future() = default;

    /* behavior::object_like */
    bool equal(object const &) const;
    jtl::immutable_string to_string() const;
    void to_string(jtl::string_builder &buff) const;
    jtl::immutable_string to_code_string() const;
    uhash to_hash() const;

    /* behavior::derefable */
    object_ref deref();

    /* behavior::realizable */
    bool is_realized() const;

    /*** XXX: Everything here is immutable after initialization. ***/
    object base{ obj_type };

    /*** XXX: Everything here is thread-safe. ***/

    /* It's undefined behavior to have multiple threads join on one thread object at
     * the same time, so we need to synchronize this. But we also have synchronized
     * state, which is separate, since the thread itself needs to set that state and it
     * can't use the same mutex that other threads are using to join. That'd cause
     * deadlocks. */
    folly::Synchronized<std::thread> thread;

    struct mutable_state
    {
      object_ref result;
      /* If the thread threw an exception, we'll hang onto it. When we're dereferenced,
       * the exception will be re-thrown. */
      jtl::option<object_ref> error;
      future_status status{ future_status::running };
    };

    folly::Synchronized<mutable_state> state;
  };
}
