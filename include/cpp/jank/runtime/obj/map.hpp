#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/detail/object_util.hpp>
#include <jank/runtime/detail/map_type.hpp>
#include <jank/runtime/obj/persistent_map_sequence.hpp>

namespace jank::runtime
{
  template <>
  struct static_object<object_type::map> : gc
  {
    using value_type = runtime::detail::persistent_map;

    static constexpr bool pointer_free{ false };

    static_object() = default;
    static_object(static_object &&) = default;
    static_object(static_object const &) = default;
    static_object(native_box<static_object> meta);
    static_object(runtime::detail::persistent_map &&d);
    static_object(runtime::detail::persistent_map const &d);
    template <typename... Args>
    static_object(runtime::detail::in_place_unique, Args &&...args)
      : data{ runtime::detail::in_place_unique{}, std::forward<Args>(args)... }
    { }

    template <typename... Args>
    static native_box<static_object> create_unique(Args &&...args)
    {
      return make_box<static_object>
      (
        runtime::detail::in_place_unique{},
        make_array_box<object_ptr>(std::forward<Args>(args)...),
        sizeof...(args)
      );
    }

    /* behavior::objectable */
    native_bool equal(object const &) const;
    native_string to_string() const;
    void to_string(fmt::memory_buffer &buff) const;
    native_integer to_hash() const;

    /* behavior::metadatable */
    object_ptr with_meta(object_ptr m) const;

    /* behavior::seqable */
    obj::persistent_map_sequence_ptr seq() const;
    obj::persistent_map_sequence_ptr fresh_seq() const;

    /* behavior::countable */
    size_t count() const;

    /* behavior::associatively_readable */
    object_ptr get(object_ptr key) const;
    object_ptr get(object_ptr key, object_ptr fallback) const;

    /* behavior::associatively_writable */
    object_ptr assoc(object_ptr key, object_ptr val) const;

    object base{ object_type::map };
    value_type data{};
    option<obj::map_ptr> meta;
  };

  namespace obj
  {
    using map = static_object<object_type::map>;
    using map_ptr = native_box<map>;
  }
}
