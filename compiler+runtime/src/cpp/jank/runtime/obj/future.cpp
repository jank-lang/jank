#include <fmt/format.h>

#include <jank/runtime/obj/future.hpp>
#include <jank/runtime/core/make_box.hpp>
#include <jank/runtime/behavior/callable.hpp>
#include <jank/runtime/core/math.hpp>

namespace jank::runtime::obj
{
  future::future(object_ptr const fn)
  {
    this->fut = std::async(std::launch::async, [&](){ return dynamic_call(fn); });
  }

  native_bool future::equal(object const &o) const
  {
    return &o == &base;
  }

  native_persistent_string future::to_string() const
  {
    util::string_builder buff;
    to_string(buff);
    return buff.release();
  }

  void future::to_string(util::string_builder &buff) const
  {
    fmt::format_to(std::back_inserter(buff), "{}@{}", object_type_str(base.type), fmt::ptr(&base));
  }

  native_persistent_string future::to_code_string() const
  {
    return to_string();
  }

  native_hash future::to_hash() const
  {
    return static_cast<native_hash>(reinterpret_cast<uintptr_t>(this));
  }

  object_ptr future::deref()
  {
    return fut.get();
  }

  object_ptr future::blocking_deref(object_ptr const millis, object_ptr const timeout_value)
  {
    switch (fut.wait_for(std::chrono::milliseconds(to_int(millis))))
    {
      case std::future_status::deferred:
      case std::future_status::timeout:
        return timeout_value;
      case std::future_status::ready:
        return fut.get();
      default:
        std::abort();
    }
  }
}
