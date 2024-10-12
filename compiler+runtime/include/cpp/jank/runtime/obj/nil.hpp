#pragma once

#include <jank/runtime/object.hpp>

namespace jank::runtime
{
  namespace obj
  {
    using persistent_array_map = static_object<object_type::persistent_array_map>;
    using persistent_array_map_ptr = native_box<persistent_array_map>;
    using cons = static_object<object_type::cons>;
    using cons_ptr = native_box<cons>;
  }

  template <>
  struct static_object<object_type::nil> : gc
  {
    static constexpr native_bool pointer_free{ true };

    static native_box<static_object> nil_const();

    static_object() = default;

    /* behavior::object_like */
    native_bool equal(object const &) const;
    native_persistent_string const &to_string() const;
    native_persistent_string const &to_code_string() const;
    void to_string(fmt::memory_buffer &buff) const;
    native_hash to_hash() const;

    /* behavior::comparable */
    native_integer compare(object const &) const;

    /* behavior::comparable extended */
    native_integer compare(static_object const &) const;

    /* behavior::associatively_readable */
    object_ptr get(object_ptr const key);
    object_ptr get(object_ptr const key, object_ptr const fallback);
    object_ptr get_entry(object_ptr key);
    native_bool contains(object_ptr key) const;

    /* behavior::associatively_writable */
    obj::persistent_array_map_ptr assoc(object_ptr key, object_ptr val) const;
    native_box<static_object> dissoc(object_ptr key) const;

    /* behavior::seqable */
    native_box<static_object> seq();
    native_box<static_object> fresh_seq() const;

    /* behavior::sequenceable */
    native_box<static_object> first() const;
    native_box<static_object> next() const;
    obj::cons_ptr conj(object_ptr head) const;

    /* behavior::sequenceable_in_place */
    native_box<static_object> next_in_place();

    object base{ object_type::nil };
  };

  namespace obj
  {
    using nil = static_object<object_type::nil>;
    using nil_ptr = native_box<nil>;
  }
}
