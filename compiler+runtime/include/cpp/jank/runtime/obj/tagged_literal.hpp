#pragma once

#include <jank/runtime/object.hpp>

namespace jank::runtime::obj
{
  struct tagged_literal : gc
  {
    static constexpr object_type obj_type{ object_type::tagged_literal };
    static constexpr native_bool pointer_free{ false };

    tagged_literal(object_ptr tag, object_ptr form);

    /* behavior::object_like */
    native_bool equal(object const &) const;
    native_persistent_string to_string() const;
    void to_string(util::string_builder &buff) const;
    native_persistent_string to_code_string() const;
    native_hash to_hash() const;

    /* behavior::associatively_readable */
    object_ptr get(object_ptr const key) const;
    object_ptr get(object_ptr const key, object_ptr const fallback) const;
    object_ptr get_entry(object_ptr key) const;
    native_bool contains(object_ptr key) const;

    object base{ obj_type };

    object_ptr tag{};
    object_ptr form{};

    mutable native_hash hash{};
  };
}
