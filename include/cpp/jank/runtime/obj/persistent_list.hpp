#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/obj/persistent_list_sequence.hpp>
#include <jank/runtime/detail/native_persistent_list.hpp>

namespace jank::runtime
{
  object_ptr seq(object_ptr s);

  template <>
  struct static_object<object_type::persistent_list> : gc
  {
    using value_type = runtime::detail::native_persistent_list;

    static constexpr native_bool pointer_free{ false };

    static native_box<static_object> create(object_ptr s);

    static_object() = default;
    static_object(static_object &&) = default;
    static_object(static_object const &) = default;
    static_object(value_type &&d);
    static_object(value_type const &d);

    template <typename... Args>
    static_object(std::in_place_t, Args &&...args)
      : data{ std::forward<Args>(args)... }
    {
    }

    template <typename... Args>
    static_object(object_ptr const meta, std::in_place_t, Args &&...args)
      : data{ std::forward<Args>(args)... }
      , meta{ meta }
    {
    }

    static native_box<static_object> empty()
    {
      static auto const ret(make_box<static_object>());
      return ret;
    }

    /* behavior::objectable */
    native_bool equal(object const &) const;
    native_persistent_string to_string() const;
    void to_string(fmt::memory_buffer &buff) const;
    native_hash to_hash() const;

    /* behavior::metadatable */
    object_ptr with_meta(object_ptr m) const;

    /* behavior::seqable */
    obj::persistent_list_sequence_ptr seq() const;
    obj::persistent_list_sequence_ptr fresh_seq() const;

    /* behavior::countable */
    size_t count() const;

    /* behavior::consable */
    native_box<static_object> cons(object_ptr head) const;

    object base{ object_type::persistent_list };
    value_type data;
    option<object_ptr> meta;
  };

  namespace obj
  {
    using persistent_list = static_object<object_type::persistent_list>;
    using persistent_list_ptr = native_box<persistent_list>;
  }
}
