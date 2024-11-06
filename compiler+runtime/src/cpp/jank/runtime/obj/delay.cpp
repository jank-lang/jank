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
    return native_persistent_string{ buff.data(), buff.size() };
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

  object_ptr obj::delay::deref()
  {
    std::lock_guard<std::mutex> const lock{ mutex };
    if(val != nullptr)
    {
      return val;
    }

    if(error != nullptr)
    {
      throw error;
    }

    try
    {
      val = dynamic_call(fn);
    }
    catch(std::exception const &e)
    {
      error = make_box(e.what());
      throw;
    }
    catch(object_ptr const e)
    {
      error = e;
      throw;
    }
    return val;
  }
}
