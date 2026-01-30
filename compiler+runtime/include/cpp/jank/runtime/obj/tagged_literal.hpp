#pragma once

#include <jank/runtime/object.hpp>

namespace jank::runtime::obj
{
  using tagged_literal_ref = oref<struct tagged_literal>;

  struct tagged_literal : object
  {
    static constexpr object_type obj_type{ object_type::tagged_literal };
    static constexpr object_behavior obj_behaviors{ object_behavior::get };
    static constexpr bool pointer_free{ false };

    tagged_literal(object_ref const tag, object_ref const form);

    /* behavior::object_like */
    bool equal(object const &) const override;
    jtl::immutable_string to_string() const override;
    void to_string(jtl::string_builder &buff) const override;
    jtl::immutable_string to_code_string() const override;
    uhash to_hash() const override;

    /* behavior::get */
    object_ref get(object_ref const key) const override;
    object_ref get(object_ref const key, object_ref const fallback) const override;
    bool contains(object_ref const key) const override;

    /*** XXX: Everything here is immutable after initialization. ***/
    object_ref tag{};
    object_ref form{};
  };
}
