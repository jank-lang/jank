#pragma once

#include <jank/runtime/object.hpp>

namespace jank::runtime::obj
{
  using tagged_literal_ref = oref<struct tagged_literal>;

  struct tagged_literal : gc
  {
    static constexpr object_type obj_type{ object_type::tagged_literal };
    static constexpr native_bool pointer_free{ false };

    tagged_literal(object_ref tag, object_ref form);

    /* behavior::object_like */
    native_bool equal(object const &) const;
    jtl::immutable_string to_string() const;
    void to_string(util::string_builder &buff) const;
    jtl::immutable_string to_code_string() const;
    native_hash to_hash() const;

    /* behavior::associatively_readable */
    object_ref get(object_ref const key) const;
    object_ref get(object_ref const key, object_ref const fallback) const;
    object_ref get_entry(object_ref key) const;
    native_bool contains(object_ref key) const;

    object base{ obj_type };

    object_ref tag{};
    object_ref form{};

    mutable native_hash hash{};
  };
}
