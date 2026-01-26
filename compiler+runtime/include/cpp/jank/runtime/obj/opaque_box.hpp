#pragma once

#include <jtl/option.hpp>

#include <jank/runtime/object.hpp>

namespace jank::runtime::obj
{
  using opaque_box_ref = oref<struct opaque_box>;

  struct opaque_box : object
  {
    static constexpr object_type obj_type{ object_type::opaque_box };
    static constexpr object_behavior obj_behaviors{ object_behavior::none };
    static constexpr bool pointer_free{ false };

    opaque_box(jtl::ptr<void> data, jtl::immutable_string const &canonical_type);

    /* behavior::object_like */
    bool equal(object const &) const override;
    uhash to_hash() const override;

    /* behavior::metadatable */
    opaque_box_ref with_meta(object_ref const m);
    object_ref get_meta() const;

    /*** XXX: Everything here is immutable after initialization. ***/
    jtl::ptr<void> data{};
    jtl::immutable_string canonical_type;
    object_ref meta;
  };
}
