#include <jank/runtime/obj/future.hpp>
#include <jank/runtime/behavior/callable.hpp>
#include <jank/runtime/core/make_box.hpp>
#include <jank/util/fmt.hpp>

namespace jank::runtime::obj
{
  bool future::equal(object const &o) const
  {
    return &o == &base;
  }

  jtl::immutable_string future::to_string() const
  {
    jtl::string_builder buff;
    to_string(buff);
    return buff.release();
  }

  void future::to_string(jtl::string_builder &buff) const
  {
    util::format_to(buff, "#object [{} {}]", object_type_str(base.type), &base);
  }

  jtl::immutable_string future::to_code_string() const
  {
    return to_string();
  }

  uhash future::to_hash() const
  {
    return static_cast<uhash>(reinterpret_cast<uintptr_t>(this));
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
        throw std::runtime_error{ util::format("Invalid future status: {}",
                                               static_cast<int>(locked_state->status)) };
    }
  }
}
