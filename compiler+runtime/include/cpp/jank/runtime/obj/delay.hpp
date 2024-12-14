#pragma once

#include <jank/runtime/object.hpp>

namespace jank::runtime::obj
{
  using delay_ptr = native_box<struct delay>;

  struct delay : gc
  {
    static constexpr object_type obj_type{ object_type::delay };
    static constexpr native_bool pointer_free{ false };

    delay() = default;
    delay(object_ptr fn);

    /* behavior::object_like */
    native_bool equal(object const &) const;
    native_persistent_string to_string() const;
    void to_string(fmt::memory_buffer &buff) const;
    native_persistent_string to_code_string() const;
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
