#pragma once

#include <jtl/option.hpp>

#include <jank/runtime/object.hpp>

namespace jank::runtime::obj
{
  using opaque_box_ref = oref<struct opaque_box>;

  struct opaque_box
  {
    static constexpr object_type obj_type{ object_type::opaque_box };
    static constexpr bool pointer_free{ false };

    opaque_box(jtl::ptr<void> data, jtl::immutable_string const &canonical_type);

    /* behavior::object_like */
    bool equal(object const &) const;
    jtl::immutable_string to_string() const;
    void to_string(jtl::string_builder &buff) const;
    jtl::immutable_string to_code_string() const;
    uhash to_hash() const;

    /* behavior::metadatable */
    opaque_box_ref with_meta(object_ref const m);

    object base{ obj_type };
    jtl::ptr<void> data{};
    jtl::immutable_string canonical_type;
    jtl::option<object_ref> meta;
  };
}
