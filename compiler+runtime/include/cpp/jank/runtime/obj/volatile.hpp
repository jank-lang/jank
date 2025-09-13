#pragma once

#include <jank/runtime/object.hpp>

namespace jank::runtime::obj
{
  using volatile_ref = oref<struct volatile_>;

  struct volatile_ : object
  {
    static constexpr object_type obj_type{ object_type::volatile_ };
    static constexpr bool pointer_free{ false };

    volatile_();
    volatile_(object_ref o);

    /* behavior::object_like */
    bool equal(object const &) const override;
    jtl::immutable_string to_string() const override;
    void to_string(jtl::string_builder &buff) const override;
    jtl::immutable_string to_code_string() const override;
    uhash to_hash() const override;

    /* behavior::derefable */
    object_ref deref() const;

    object_ref reset(object_ref o);

    object base{ obj_type };
    object_ref val{};
  };
}
