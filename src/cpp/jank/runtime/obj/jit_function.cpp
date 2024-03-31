#include <jank/runtime/obj/jit_function.hpp>

namespace jank::runtime
{
  native_bool obj::jit_function::equal(object const &rhs) const
  {
    return &base == &rhs;
  }

  native_persistent_string obj::jit_function::to_string()
  {
    fmt::memory_buffer buff;
    to_string(buff);
    return native_persistent_string{ buff.data(), buff.size() };
  }

  void obj::jit_function::to_string(fmt::memory_buffer &buff)
  {
    fmt::format_to(std::back_inserter(buff),
                   "{}@{}",
                   magic_enum::enum_name(base.type),
                   fmt::ptr(&base));
  }

  native_hash obj::jit_function::to_hash() const
  {
    return static_cast<native_hash>(reinterpret_cast<uintptr_t>(this));
  }

  object_ptr obj::jit_function::with_meta(object_ptr const m)
  {
    auto const new_meta(behavior::detail::validate_meta(m));
    meta = new_meta;
    return this;
  }
}
