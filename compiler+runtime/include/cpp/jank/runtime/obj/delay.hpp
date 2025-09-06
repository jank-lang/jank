#pragma once

#include <jank/runtime/object.hpp>

namespace jank::runtime::obj
{
  using delay_ref = oref<struct delay>;

  struct delay : object
  {
    static constexpr object_type obj_type{ object_type::delay };
    static constexpr bool pointer_free{ false };

    delay();
    delay(object_ref fn);

    /* behavior::object_like */
    bool equal(object const &) const override;
    jtl::immutable_string to_string() const override;
    void to_string(jtl::string_builder &buff) const override;
    jtl::immutable_string to_code_string() const override;
    uhash to_hash() const override;

    /* behavior::derefable */
    object_ref deref();

    object base{ obj_type };
    object_ref val{};
    object_ref fn{};
    object_ref error{};
    std::mutex mutex;
  };
}
