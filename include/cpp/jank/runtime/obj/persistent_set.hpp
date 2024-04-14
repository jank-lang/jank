#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/obj/persistent_set_sequence.hpp>

namespace jank::runtime
{
  namespace obj
  {
    using transient_set = static_object<object_type::transient_set>;
    using transient_set_ptr = native_box<transient_set>;
  }

  template <>
  struct static_object<object_type::persistent_set> : gc
  {
    using value_type = runtime::detail::native_persistent_set;

    static constexpr native_bool pointer_free{ false };

    static_object() = default;
    static_object(static_object &&) noexcept = default;
    static_object(static_object const &) = default;
    static_object(value_type &&d);
    static_object(value_type const &d);
    static_object(object_ptr meta, value_type &&d);

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
    obj::persistent_set_sequence_ptr seq() const;
    obj::persistent_set_sequence_ptr fresh_seq() const;

    /* behavior::countable */
    size_t count() const;

    /* behavior::consable */
    native_box<static_object> cons(object_ptr head) const;

    /* behavior::callable */
    object_ptr call(object_ptr) const;

    /* behavior::transientable */
    obj::transient_set_ptr to_transient() const;

    native_bool contains(object_ptr o) const;

    object base{ object_type::persistent_set };
    value_type data;
    option<object_ptr> meta;
  };

  using persistent_set = static_object<object_type::persistent_set>;
  using persistent_set_ptr = native_box<persistent_set>;
}
