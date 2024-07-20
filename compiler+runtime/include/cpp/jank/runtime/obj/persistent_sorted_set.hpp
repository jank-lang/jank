#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/obj/persistent_sorted_set_sequence.hpp>

namespace jank::runtime
{
  namespace obj
  {
    using transient_sorted_set = static_object<object_type::transient_sorted_set>;
    using transient_sorted_set_ptr = native_box<transient_sorted_set>;
  }

  template <>
  struct static_object<object_type::persistent_sorted_set> : gc
  {
    using value_type = runtime::detail::native_persistent_sorted_set;

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

    static native_box<static_object> create_from_seq(object_ptr const seq);

    /* behavior::object_like */
    native_bool equal(object const &) const;
    native_persistent_string to_string() const;
    void to_string(fmt::memory_buffer &buff) const;
    native_hash to_hash() const;

    /* behavior::metadatable */
    native_box<static_object> with_meta(object_ptr m) const;

    /* behavior::seqable */
    obj::persistent_sorted_set_sequence_ptr seq() const;
    obj::persistent_sorted_set_sequence_ptr fresh_seq() const;

    /* behavior::countable */
    size_t count() const;

    /* behavior::conjable */
    native_box<static_object> conj(object_ptr head) const;

    /* behavior::callable */
    object_ptr call(object_ptr);

    /* behavior::transientable */
    obj::transient_sorted_set_ptr to_transient() const;

    native_bool contains(object_ptr o) const;
    native_box<static_object> disj(object_ptr o) const;

    object base{ object_type::persistent_sorted_set };
    value_type data;
    option<object_ptr> meta;
  };

  using persistent_sorted_set = static_object<object_type::persistent_sorted_set>;
  using persistent_sorted_set_ptr = native_box<persistent_sorted_set>;
}
