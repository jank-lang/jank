#pragma once

#include <jank/runtime/behavior/seqable.hpp>

namespace jank::runtime
{
  template <>
  struct static_object<object_type::cons> : gc
  {
    static constexpr native_bool pointer_free{ false };

    static_object() = default;
    static_object(static_object &&) = default;
    static_object(static_object const &) = default;
    static_object(object_ptr const head, object_ptr const tail);

    /* behavior::objectable */
    native_bool equal(object const &) const;
    native_persistent_string to_string();
    void to_string(fmt::memory_buffer &buff);
    native_hash to_hash() const;

    /* behavior::metadatable */
    object_ptr with_meta(object_ptr m) const;

    /* behavior::seqable */
    native_box<static_object> seq();
    native_box<static_object> fresh_seq() const;

    /* behavior::sequenceable */
    object_ptr first() const;
    object_ptr next() const;
    native_box<static_object> next_in_place();
    object_ptr next_in_place_first();

    /* behavior::consable */
    native_box<static_object> cons(object_ptr head) const;

    object base{ object_type::cons };
    object_ptr head{};
    object_ptr tail{};
    mutable native_hash hash{};
  };

  namespace obj
  {
    using cons = static_object<object_type::cons>;
    using cons_ptr = native_box<cons>;
  }
}
