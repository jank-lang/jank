#include <jank/runtime/obj/promise.hpp>

namespace jank::runtime::obj
{
  promise::promise()
    : object{ obj_type, obj_behaviors }
  {
  }

  object_ref promise::deref()
  {
    auto locked_state{ state.wlock() };
    sync.wait(locked_state.as_lock(),
              [&] { return locked_state->status == promise_status::ready; });
    return locked_state->val;
  }

  bool promise::is_realized() const
  {
    auto const locked_state{ state.rlock() };
    return locked_state->status == promise_status::ready;
  }

  object_ref promise::call(object_ref const o) const
  {
    auto const locked_state{ state.wlock() };
    if(locked_state->status == promise_status::pending)
    {
      locked_state->val = o;
      locked_state->status = promise_status::ready;
      sync.notify_all();
    }
    return this;
  }

}
