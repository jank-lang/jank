#pragma once

#include <jank/runtime/object.hpp>

namespace uuids
{
  class uuid;
}

namespace jank::runtime::obj
{
  using uuid_ref = oref<struct uuid>;

  struct uuid : object
  {
    static constexpr object_type obj_type{ object_type::uuid };
    static constexpr object_behavior obj_behaviors{ object_behavior::none };
    static constexpr bool pointer_free{ false };

    uuid();
    uuid(jtl::immutable_string const &s);

    /* behavior::object_like */
    bool equal(object const &) const override;
    jtl::immutable_string to_string() const override;
    void to_string(jtl::string_builder &buff) const override;
    jtl::immutable_string to_code_string() const override;
    uhash to_hash() const override;

    /*** XXX: Everything here is immutable after initialization. ***/
    jtl::ref<uuids::uuid> value;
  };
}
