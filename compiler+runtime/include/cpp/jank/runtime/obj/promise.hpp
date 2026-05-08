#pragma once

#include <condition_variable>

#include <folly/Synchronized.h>

#include <jank/runtime/object.hpp>

namespace jank::runtime::obj
{
  using promise_ref = oref<struct promise>;

  enum class promise_status : u8
  {
    pending,
    ready
  };

  struct promise : object
  {
    static constexpr object_type obj_type{ object_type::promise };
    static constexpr object_behavior obj_behaviors{ object_behavior::call };
    static constexpr bool pointer_free{ false };

    promise();

    /* behavior::derefable */
    object_ref deref();

    /* behavior::realizable */
    bool is_realized() const;

    /* behavior::callable */
    object_ref call(object_ref const) const override;

    struct mutable_state
    {
      object_ref val;
      promise_status status{ promise_status::pending };
    };

    mutable folly::Synchronized<mutable_state> state;
    mutable std::condition_variable_any sync;
  };
}
