#include <jank/runtime/obj/opaque_box.hpp>
#include <jank/runtime/rtti.hpp>
#include <jank/util/fmt.hpp>

namespace jank::runtime::obj
{
  opaque_box::opaque_box(jtl::ptr<void> const data)
    : data{ data }
  {
  }

  bool opaque_box::equal(object const &o) const
  {
    if(o.type != object_type::tagged_literal)
    {
      return false;
    }

    auto const b{ expect_object<opaque_box>(&o) };
    return b->data == data;
  }

  void opaque_box::to_string(util::string_builder &buff) const
  {
    util::format_to(buff, "{}@{}", object_type_str(base.type), &base);
  }

  jtl::immutable_string opaque_box::to_string() const
  {
    util::string_builder buff;
    to_string(buff);
    return buff.release();
  }

  jtl::immutable_string opaque_box::to_code_string() const
  {
    return to_string();
  }

  uhash opaque_box::to_hash() const
  {
    return static_cast<uhash>(reinterpret_cast<uintptr_t>(data.data));
  }
}
