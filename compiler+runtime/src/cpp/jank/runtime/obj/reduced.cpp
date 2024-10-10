#include <jank/runtime/obj/reduced.hpp>

namespace jank::runtime
{
  obj::reduced::static_object(object_ptr const o)
    : val{ o }
  {
    assert(val);
  }

  native_bool obj::reduced::equal(object const &o) const
  {
    return &o == &base;
  }

  native_persistent_string obj::reduced::to_string() const
  {
    fmt::memory_buffer buff;
    to_string(buff);
    return native_persistent_string{ buff.data(), buff.size() };
  }

  void obj::reduced::to_string(fmt::memory_buffer &buff) const
  {
    fmt::format_to(std::back_inserter(buff),
                   "{}@{}",
                   magic_enum::enum_name(base.type),
                   fmt::ptr(&base));
  }

  native_persistent_string obj::reduced::to_code_string() const
  {
    return to_string();
  }

  native_hash obj::reduced::to_hash() const
  {
    return static_cast<native_hash>(reinterpret_cast<uintptr_t>(this));
  }

  object_ptr obj::reduced::deref() const
  {
    return val;
  }
}
