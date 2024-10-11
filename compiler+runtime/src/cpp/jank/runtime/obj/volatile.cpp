#include <jank/runtime/obj/volatile.hpp>

namespace jank::runtime
{
  obj::volatile_::static_object(object_ptr const o)
    : val{ o }
  {
    assert(val);
  }

  native_bool obj::volatile_::equal(object const &o) const
  {
    return &o == &base;
  }

  native_persistent_string obj::volatile_::to_string() const
  {
    fmt::memory_buffer buff;
    to_string(buff);
    return native_persistent_string{ buff.data(), buff.size() };
  }

  void obj::volatile_::to_string(fmt::memory_buffer &buff) const
  {
    fmt::format_to(std::back_inserter(buff),
                   "{}@{}",
                   magic_enum::enum_name(base.type),
                   fmt::ptr(&base));
  }

  native_persistent_string obj::volatile_::to_code_string() const
  {
    return to_string();
  }

  native_hash obj::volatile_::to_hash() const
  {
    return static_cast<native_hash>(reinterpret_cast<uintptr_t>(this));
  }

  object_ptr obj::volatile_::deref() const
  {
    return val;
  }

  object_ptr obj::volatile_::reset(object_ptr const o)
  {
    val = o;
    assert(val);
    return val;
  }
}
