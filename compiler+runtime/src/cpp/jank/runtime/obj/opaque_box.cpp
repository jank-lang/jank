#include <jank/runtime/obj/opaque_box.hpp>
#include <jank/runtime/behavior/metadatable.hpp>
#include <jank/runtime/rtti.hpp>
#include <jank/util/fmt.hpp>

namespace jank::runtime::obj
{
  opaque_box::opaque_box(jtl::ptr<void> const data, jtl::immutable_string const &canonical_type)
    : data{ data }
    , canonical_type{ canonical_type }
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

  void opaque_box::to_string(jtl::string_builder &buff) const
  {
    util::format_to(buff, "#object [{} {}]", object_type_str(base.type), &base);
  }

  jtl::immutable_string opaque_box::to_string() const
  {
    jtl::string_builder buff;
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

  opaque_box_ref opaque_box::with_meta(object_ref const m)
  {
    meta = behavior::detail::validate_meta(m);
    return this;
  }
}
