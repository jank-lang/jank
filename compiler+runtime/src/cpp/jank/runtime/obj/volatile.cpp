#include <magic_enum.hpp>

#include <jank/runtime/obj/volatile.hpp>

namespace jank::runtime::obj
{
  volatile_::volatile_(object_ptr const o)
    : val{ o }
  {
    assert(val);
  }

  native_bool volatile_::equal(object const &o) const
  {
    return &o == &base;
  }

  native_persistent_string volatile_::to_string() const
  {
    fmt::memory_buffer buff;
    to_string(buff);
    return native_persistent_string{ buff.data(), buff.size() };
  }

  void volatile_::to_string(fmt::memory_buffer &buff) const
  {
    fmt::format_to(std::back_inserter(buff),
                   "{}@{}",
                   magic_enum::enum_name(base.type),
                   fmt::ptr(&base));
  }

  native_persistent_string volatile_::to_code_string() const
  {
    return to_string();
  }

  native_hash volatile_::to_hash() const
  {
    return static_cast<native_hash>(reinterpret_cast<uintptr_t>(this));
  }

  object_ptr volatile_::deref() const
  {
    return val;
  }

  object_ptr volatile_::reset(object_ptr const o)
  {
    val = o;
    assert(val);
    return val;
  }
}
