#include <jank/runtime/obj/opaque_box.hpp>
#include <jank/runtime/behavior/metadatable.hpp>
#include <jank/runtime/rtti.hpp>
#include <jank/util/fmt.hpp>

namespace jank::runtime::obj
{
  opaque_box::opaque_box(jtl::ptr<void> const data, jtl::immutable_string const &canonical_type)
    : object{ obj_type }
    , data{ data }
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

  uhash opaque_box::to_hash() const
  {
    return static_cast<uhash>(reinterpret_cast<uintptr_t>(data.data));
  }

  opaque_box_ref opaque_box::with_meta(object_ref const m)
  {
    auto const meta(behavior::detail::validate_meta(m));
    auto ret(make_box<opaque_box>(data, canonical_type));
    ret->meta = meta;
    return ret;
  }
}
