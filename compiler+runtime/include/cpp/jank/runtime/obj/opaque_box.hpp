#pragma once

#include <jank/runtime/object.hpp>

namespace jank::runtime::obj
{
  using opaque_box_ref = oref<struct opaque_box>;

  struct opaque_box : gc
  {
    static constexpr object_type obj_type{ object_type::opaque_box };
    static constexpr bool pointer_free{ false };

    opaque_box(jtl::ptr<void> data);

    /* behavior::object_like */
    bool equal(object const &) const;
    jtl::immutable_string to_string() const;
    void to_string(util::string_builder &buff) const;
    jtl::immutable_string to_code_string() const;
    uhash to_hash() const;

    object base{ obj_type };
    jtl::ptr<void> data{};
  };
}
