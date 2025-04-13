#pragma once

#include <jank/runtime/object.hpp>

namespace jank::runtime::obj
{
  using reduced_ref = jtl::oref<struct reduced>;

  struct reduced : gc
  {
    static constexpr object_type obj_type{ object_type::reduced };
    static constexpr native_bool pointer_free{ false };

    reduced() = default;
    reduced(object_ptr o);

    /* behavior::object_like */
    native_bool equal(object const &) const;
    jtl::immutable_string to_string() const;
    void to_string(util::string_builder &buff) const;
    jtl::immutable_string to_code_string() const;
    native_hash to_hash() const;

    /* behavior::derefable */
    object_ptr deref() const;

    object base{ obj_type };
    object_ptr val{};
  };
}
