#include <magic_enum.hpp>

#include <jank/runtime/obj/native_pointer_wrapper.hpp>
#include <jank/runtime/rtti.hpp>

namespace jank::runtime
{
  obj::native_pointer_wrapper::static_object(void * const d)
    : data{ d }
  {
  }

  native_bool obj::native_pointer_wrapper::equal(object const &o) const
  {
    if(o.type != object_type::native_pointer_wrapper)
    {
      return false;
    }

    auto const c(expect_object<obj::native_pointer_wrapper>(&o));
    return data == c->data;
  }

  void obj::native_pointer_wrapper::to_string(fmt::memory_buffer &buff) const
  {
    fmt::format_to(std::back_inserter(buff),
                   "{}@{}",
                   magic_enum::enum_name(base.type),
                   fmt::ptr(&base));
  }

  native_persistent_string obj::native_pointer_wrapper::to_string() const
  {
    fmt::memory_buffer buff;
    to_string(buff);
    return native_persistent_string{ buff.data(), buff.size() };
  }

  native_persistent_string obj::native_pointer_wrapper::to_code_string() const
  {
    return to_string();
  }

  native_hash obj::native_pointer_wrapper::to_hash() const
  {
    return static_cast<native_hash>(reinterpret_cast<uintptr_t>(data));
  }
}
