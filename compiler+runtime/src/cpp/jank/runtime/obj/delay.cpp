#include <jank/runtime/obj/delay.hpp>

namespace jank::runtime
{
  obj::delay::static_object(object_ptr const fn)
    : fn{ fn }
  {
  }

  native_bool obj::delay::equal(object const &o) const
  {
    return &o == &base;
  }

  native_persistent_string obj::delay::to_string() const
  {
    fmt::memory_buffer buff;
    to_string(buff);
    return native_persistent_string{ reinterpret_cast<native_persistent_string::size_type>(
                                       buff.data()),
                                     static_cast<char>(buff.size()) };
  }

  object_ptr obj::delay::force(object_ptr const &d)
  {
    return visit_object(
  [d](auto const typed_d) -> object_ptr {
    using T = typename decltype(typed_d)::value_type;

    if constexpr(behavior::derefable<T>)
    {
      return typed_d->deref();
    }
    else
    {
      return d;
    }
  },
  d);
  }

  void obj::delay::to_string(fmt::memory_buffer &buff) const
  {
    fmt::format_to(std::back_inserter(buff),
                   "{}@{}",
                   magic_enum::enum_name(base.type),
                   fmt::ptr(&base));
  }

  native_persistent_string obj::delay::to_code_string() const
  {
    return to_string();
  }

  native_hash obj::delay::to_hash() const
  {
    return static_cast<native_hash>(reinterpret_cast<uintptr_t>(this));
  }

  object_ptr obj::delay::deref() const
  {
    if(val != nullptr)
    {
      return val;
    }

    if(delay_exception_ptr != nullptr)
    {
      throw *delay_exception_ptr;
    }

    return visit_object(
      [&]<typename T>(T const typed_s) -> object_ptr {
        if constexpr(behavior::function_like<T> && !std::same_as<T, obj::nil>)
        {
          try
          {
            val = typed_s->call();
            return val;
          }
          catch(...)
          {
            *delay_exception_ptr = std::current_exception();
            throw *delay_exception_ptr;
          }
        }
        else
        {
          try
          {
            throw std::runtime_error{ fmt::format("Invalid expression provided: {}",
                                                  typed_s->to_string()) };
          }
          catch(...)
          {
            *delay_exception_ptr = std::current_exception();
            std::rethrow_exception(*delay_exception_ptr);
          }
        }
      },
      fn);
  }
}
