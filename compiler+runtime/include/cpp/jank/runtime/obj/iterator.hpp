#pragma once

#include <jank/runtime/behavior/seqable.hpp>

namespace jank::runtime
{
  namespace obj
  {
    using cons = static_object<object_type::cons>;
    using cons_ptr = native_box<cons>;
  }

  template <>
  struct static_object<object_type::iterator> : gc
  {
    static constexpr native_bool pointer_free{ false };

    static_object() = default;
    static_object(static_object &&) = default;
    static_object(static_object const &) = default;
    static_object(object_ptr const fn, object_ptr const start);

    /* behavior::object_like */
    native_bool equal(object const &) const;
    native_persistent_string to_string();
    void to_string(fmt::memory_buffer &buff);
    native_hash to_hash() const;

    /* behavior::seqable */
    native_box<static_object> seq();
    native_box<static_object> fresh_seq() const;

    /* behavior::sequenceable */
    object_ptr first() const;
    native_box<static_object> next() const;
    obj::cons_ptr conj(object_ptr head) const;

    /* behavior::sequenceable_in_place */
    native_box<static_object> next_in_place();

    object base{ object_type::iterator };
    /* TODO: Support chunking. */
    object_ptr fn{};
    object_ptr current{};
    object_ptr previous{};
    mutable native_box<static_object> cached_next{};
  };

  namespace obj
  {
    using iterator = static_object<object_type::iterator>;
    using iterator_ptr = native_box<iterator>;
  }
}
