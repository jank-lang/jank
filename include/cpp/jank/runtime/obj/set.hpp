#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/obj/persistent_set_sequence.hpp>

namespace jank::runtime
{
  template <>
  struct static_object<object_type::set> : gc
  {
    using value_type = runtime::detail::native_persistent_set;

    static constexpr bool pointer_free{ false };

    static_object() = default;
    static_object(static_object &&) = default;
    static_object(static_object const &) = default;
    static_object(native_box<static_object> meta);
    static_object(value_type &&d);
    static_object(value_type const &d);
    template <typename... Args>
    static_object(Args &&...args)
      : data{ std::forward<Args>(args)... }
    { }

    /* behavior::objectable */
    native_bool equal(object const &) const;
    native_string to_string() const;
    void to_string(fmt::memory_buffer &buff) const;
    native_integer to_hash() const;

    /* behavior::metadatable */
    object_ptr with_meta(object_ptr m) const;

    /* behavior::seqable */
    obj::persistent_set_sequence_ptr seq() const;
    obj::persistent_set_sequence_ptr fresh_seq() const;

    /* behavior::countable */
    size_t count() const;

    /* behavior::consable */
    native_box<static_object> cons(object_ptr head) const;

    /* behavior::callable */
    object_ptr call(object_ptr) const;

    native_bool contains(object_ptr o) const;

    object base{ object_type::set };
    value_type data;
    option<object_ptr> meta;
  };
}
