#pragma once

#include <jank/runtime/object.hpp>

namespace jank::runtime::obj
{
  using delay_ref = oref<struct delay>;

  struct delay : gc
  {
    static constexpr object_type obj_type{ object_type::delay };
    static constexpr bool pointer_free{ false };

    delay() = default;
    delay(object_ref fn);

    /* behavior::object_like */
    bool equal(object const &) const;
    jtl::immutable_string to_string() const;
    void to_string(util::string_builder &buff) const;
    jtl::immutable_string to_code_string() const;
    uhash to_hash() const;

    /* behavior::derefable */
    object_ref deref();

    object base{ obj_type };
    object_ref val{};
    object_ref fn{};
    object_ref error{};
    std::mutex mutex;
  };
}
