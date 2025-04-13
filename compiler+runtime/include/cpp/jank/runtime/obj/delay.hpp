#pragma once

#include <jank/runtime/object.hpp>

namespace jank::runtime::obj
{
  using delay_ref = jtl::oref<struct delay>;

  struct delay : gc
  {
    static constexpr object_type obj_type{ object_type::delay };
    static constexpr native_bool pointer_free{ false };

    delay() = default;
    delay(object_ptr fn);

    /* behavior::object_like */
    native_bool equal(object const &) const;
    jtl::immutable_string to_string() const;
    void to_string(util::string_builder &buff) const;
    jtl::immutable_string to_code_string() const;
    native_hash to_hash() const;

    /* behavior::derefable */
    object_ptr deref();

    object base{ obj_type };
    object_ptr val{};
    object_ptr fn{};
    object_ptr error{};
    std::mutex mutex;
  };
}
