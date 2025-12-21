#pragma once

#include <jank/runtime/object.hpp>

namespace jank::runtime::obj
{
  using tagged_literal_ref = oref<struct tagged_literal>;

  struct tagged_literal
  {
    static constexpr object_type obj_type{ object_type::tagged_literal };
    static constexpr bool pointer_free{ false };

    tagged_literal(object_ref const tag, object_ref const form);

    /* behavior::object_like */
    bool equal(object const &) const;
    jtl::immutable_string to_string() const;
    void to_string(jtl::string_builder &buff) const;
    jtl::immutable_string to_code_string() const;
    uhash to_hash() const;

    /* behavior::associatively_readable */
    object_ref get(object_ref const key) const;
    object_ref get(object_ref const key, object_ref const fallback) const;
    object_ref get_entry(object_ref const key) const;
    bool contains(object_ref const key) const;

    object base{ obj_type };

    object_ref tag{};
    object_ref form{};

    mutable uhash hash{};
  };
}
