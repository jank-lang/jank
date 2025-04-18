#include <jank/runtime/obj/native_pointer_wrapper.hpp>
#include <jank/runtime/rtti.hpp>
#include <jank/util/fmt.hpp>

namespace jank::runtime::obj
{
  native_pointer_wrapper::native_pointer_wrapper(void * const d)
    : data{ d }
  {
  }

  bool native_pointer_wrapper::equal(object const &o) const
  {
    if(o.type != object_type::native_pointer_wrapper)
    {
      return false;
    }

    auto const c(expect_object<native_pointer_wrapper>(&o));
    return data == c->data;
  }

  void native_pointer_wrapper::to_string(util::string_builder &buff) const
  {
    util::format_to(buff, "{}@{}", object_type_str(base.type), &base);
  }

  jtl::immutable_string native_pointer_wrapper::to_string() const
  {
    util::string_builder buff;
    to_string(buff);
    return buff.release();
  }

  jtl::immutable_string native_pointer_wrapper::to_code_string() const
  {
    return to_string();
  }

  native_hash native_pointer_wrapper::to_hash() const
  {
    return static_cast<native_hash>(reinterpret_cast<uintptr_t>(data));
  }
}
