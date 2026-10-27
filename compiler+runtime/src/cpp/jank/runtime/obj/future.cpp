#include <jank/runtime/obj/future.hpp>
#include <jank/runtime/core/make_box.hpp>
#include <jank/util/fmt.hpp>

namespace jank::runtime::obj
{
  future::future()
    : object{ obj_type, obj_behaviors }
  {
  }

  object_ref future::deref()
  {
    /* Calling join on a std::thread object from multiple threads at the same time is undefined
     * behavior. We need to synchronize here. */
    {
      auto const locked_thread{ thread.wlock() };
      if(locked_thread->joinable())
      {
        locked_thread->join();
      }
    }

    auto const locked_state{ state.rlock() };
    jank_debug_assert(locked_state->status != future_status::running);
    if(locked_state->error.is_some())
    {
      throw locked_state->error.unwrap();
    }

    return locked_state->result;
  }

  bool future::is_realized() const
  {
    auto const locked_state{ state.rlock() };
    switch(locked_state->status)
    {
      case future_status::running:
        return false;
      case future_status::done:
      case future_status::cancelled:
        return true;
      default:
        throw std::runtime_error{ util::format("The `future` status `{}` is not recognized.",
                                               static_cast<int>(locked_state->status)) };
    }
  }
}

namespace std
{
  size_t
  hash<jank::runtime::obj::future_ref>::operator()(jank::runtime::obj::future_ref const o) const
  {
    return o.to_hash();
  }

  size_t hash<jank::runtime::obj::future>::operator()(jank::runtime::obj::future const &o) const
  {
    static auto const hasher(std::hash<jank::runtime::obj::future_ref>{});
    return hasher(const_cast<jank::runtime::obj::future *>(&o));
  }

  bool equal_to<jank::runtime::obj::future_ref>::operator()(
    jank::runtime::obj::future_ref const lhs,
    jank::runtime::obj::future_ref const rhs) const noexcept
  {
    return lhs.raw() == rhs.raw();
  }
}
