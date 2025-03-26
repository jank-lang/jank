#pragma once

#include <jank/runtime/object.hpp>

namespace jank::runtime::obj
{
  using volatile_ptr = native_box<struct volatile_>;

  struct volatile_ : gc
  {
    static constexpr object_type obj_type{ object_type::volatile_ };
    static constexpr native_bool pointer_free{ false };

    volatile_() = default;
    volatile_(object_ptr o);

    /* behavior::object_like */
    native_bool equal(object const &) const;
    jtl::immutable_string to_string() const;
    void to_string(util::string_builder &buff) const;
    jtl::immutable_string to_code_string() const;
    native_hash to_hash() const;

    /* behavior::derefable */
    object_ptr deref() const;

    object_ptr reset(object_ptr o);

    object base{ obj_type };
    object_ptr val{};
  };
}
