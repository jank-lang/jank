#include <magic_enum.hpp>

#include <jank/runtime/obj/reduced.hpp>

namespace jank::runtime::obj
{
  reduced::reduced(object_ptr const o)
    : val{ o }
  {
    assert(val);
  }

  native_bool reduced::equal(object const &o) const
  {
    return &o == &base;
  }

  native_persistent_string reduced::to_string() const
  {
    fmt::memory_buffer buff;
    to_string(buff);
    return native_persistent_string{ buff.data(), buff.size() };
  }

  void reduced::to_string(fmt::memory_buffer &buff) const
  {
    fmt::format_to(std::back_inserter(buff),
                   "{}@{}",
                   magic_enum::enum_name(base.type),
                   fmt::ptr(&base));
  }

  native_persistent_string reduced::to_code_string() const
  {
    return to_string();
  }

  native_hash reduced::to_hash() const
  {
    return static_cast<native_hash>(reinterpret_cast<uintptr_t>(this));
  }

  object_ptr reduced::deref() const
  {
    return val;
  }
}
