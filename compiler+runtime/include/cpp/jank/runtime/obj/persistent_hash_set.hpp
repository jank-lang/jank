#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/obj/persistent_hash_set_sequence.hpp>

namespace jank::runtime::obj
{
  using transient_hash_set_ptr = native_box<struct transient_hash_set>;
  using persistent_hash_set_ptr = native_box<struct persistent_hash_set>;

  struct persistent_hash_set : gc
  {
    static constexpr object_type obj_type{ object_type::persistent_hash_set };
    static constexpr native_bool pointer_free{ false };
    static constexpr native_bool is_set_like{ true };

    using value_type = runtime::detail::native_persistent_hash_set;

    persistent_hash_set() = default;
    persistent_hash_set(persistent_hash_set &&) noexcept = default;
    persistent_hash_set(persistent_hash_set const &) = default;
    persistent_hash_set(value_type &&d);
    persistent_hash_set(value_type const &d);
    persistent_hash_set(object_ptr meta, value_type &&d);

    template <typename... Args>
    persistent_hash_set(std::in_place_t, Args &&...args)
      : data{ std::forward<Args>(args)... }
    {
    }

    template <typename... Args>
    persistent_hash_set(object_ptr const meta, std::in_place_t, Args &&...args)
      : data{ std::forward<Args>(args)... }
      , meta{ meta }
    {
    }

    static persistent_hash_set_ptr empty()
    {
      static auto const ret(make_box<persistent_hash_set>());
      return ret;
    }

    static persistent_hash_set_ptr create_from_seq(object_ptr const seq);

    /* behavior::object_like */
    native_bool equal(object const &) const;
    native_persistent_string to_string() const;
    void to_string(fmt::memory_buffer &buff) const;
    native_persistent_string to_code_string() const;
    native_hash to_hash() const;

    /* behavior::metadatable */
    persistent_hash_set_ptr with_meta(object_ptr m) const;

    /* behavior::seqable */
    obj::persistent_hash_set_sequence_ptr seq() const;
    obj::persistent_hash_set_sequence_ptr fresh_seq() const;

    /* behavior::countable */
    size_t count() const;

    /* behavior::conjable */
    persistent_hash_set_ptr conj(object_ptr head) const;

    /* behavior::callable */
    object_ptr call(object_ptr) const;

    /* behavior::transientable */
    obj::transient_hash_set_ptr to_transient() const;

    native_bool contains(object_ptr o) const;
    persistent_hash_set_ptr disj(object_ptr o) const;

    object base{ object_type::persistent_hash_set };
    value_type data;
    option<object_ptr> meta;
  };
}
