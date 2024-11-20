#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/detail/type.hpp>
#include <jank/runtime/obj/persistent_vector_sequence.hpp>

namespace jank::runtime
{
  namespace obj
  {
    using transient_vector = static_object<object_type::transient_vector>;
    using transient_vector_ptr = native_box<transient_vector>;
  }

  template <>
  struct static_object<object_type::persistent_vector> : gc
  {
    using transient_type = static_object<object_type::transient_vector>;
    using value_type = runtime::detail::native_persistent_vector;

    static constexpr native_bool pointer_free{ false };
    static constexpr native_bool is_sequential{ true };

    static_object() = default;
    static_object(static_object &&) = default;
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

    static native_box<static_object> create(object_ptr s);

    static native_box<static_object> empty()
    {
      static auto const ret(make_box<static_object>());
      return ret;
    }

    /* behavior::object_like */
    native_bool equal(object const &) const;
    native_persistent_string to_string() const;
    void to_string(fmt::memory_buffer &buff) const;
    native_persistent_string to_code_string() const;
    native_hash to_hash() const;

    /* behavior::comparable */
    native_integer compare(object const &) const;

    /* behavior::comparable extended */
    native_integer compare(static_object const &) const;

    /* behavior::metadatable */
    native_box<static_object> with_meta(object_ptr m) const;

    /* behavior::seqable */
    obj::persistent_vector_sequence_ptr seq() const;
    obj::persistent_vector_sequence_ptr fresh_seq() const;

    /* behavior::countable */
    size_t count() const;

    /* behavior::associatively_readable */
    object_ptr get(object_ptr key) const;
    object_ptr get(object_ptr key, object_ptr fallback) const;
    object_ptr get_entry(object_ptr key) const;
    native_bool contains(object_ptr key) const;

    /* behavior::conjable */
    native_box<static_object> conj(object_ptr head) const;

    /* behavior::stackable */
    object_ptr peek() const;
    native_box<static_object> pop() const;

    /* behavior::indexable */
    object_ptr nth(object_ptr index) const;
    object_ptr nth(object_ptr index, object_ptr fallback) const;

    /* behavior::transientable */
    obj::transient_vector_ptr to_transient() const;

    object base{ object_type::persistent_vector };
    value_type data;
    option<object_ptr> meta;
    mutable native_hash hash{};
  };

  namespace obj
  {
    using persistent_vector = static_object<object_type::persistent_vector>;
    using persistent_vector_ptr = native_box<persistent_vector>;
  }
}
