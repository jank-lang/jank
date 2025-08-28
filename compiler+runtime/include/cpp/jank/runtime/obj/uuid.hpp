#pragma once

#include <uuid.h>

#include <jank/runtime/object.hpp>

namespace jank::runtime::obj
{
  using uuid_ref = oref<struct uuid>;

  struct uuid : gc
  {
    static constexpr object_type obj_type{ object_type::uuid };
    static constexpr bool pointer_free{ false };

    uuid();
    uuid(jtl::immutable_string const &s);

    /* behavior::object_like */
    bool equal(object const &) const;
    jtl::immutable_string to_string() const;
    void to_string(jtl::string_builder &buff) const;
    jtl::immutable_string to_code_string() const;
    uhash to_hash() const;

    object base{ obj_type };
    uuids::uuid value{};
    mutable uhash hash{};
  };
}
