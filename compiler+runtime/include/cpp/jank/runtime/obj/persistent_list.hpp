#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/obj/persistent_list_sequence.hpp>
#include <jank/runtime/detail/native_persistent_list.hpp>

namespace jank::runtime::obj
{
  using persistent_list_ptr = native_box<struct persistent_list>;

  struct persistent_list : gc
  {
    using value_type = runtime::detail::native_persistent_list;

    static constexpr object_type obj_type{ object_type::persistent_list };
    static constexpr native_bool pointer_free{ false };
    static constexpr native_bool is_sequential{ true };

    /* Create from a sequence. */
    static persistent_list_ptr create(object_ptr s);
    static persistent_list_ptr create(persistent_list_ptr s);

    persistent_list() = default;
    persistent_list(persistent_list &&) noexcept = default;
    persistent_list(persistent_list const &) = default;
    persistent_list(value_type const &d);
    persistent_list(object_ptr meta, value_type const &d);

    /* TODO: This is broken when `args` is a value_type list we're looking to wrap in another list.
     * It just uses the copy ctor. */
    template <typename... Args>
    persistent_list(std::in_place_t, Args &&...args)
      : data{ std::forward<Args>(args)... }
    {
    }

    template <typename... Args>
    persistent_list(object_ptr const meta, std::in_place_t, Args &&...args)
      : data{ std::forward<Args>(args)... }
      , meta{ meta }
    {
    }

    static persistent_list_ptr empty()
    {
      static auto const ret(make_box<persistent_list>());
      return ret;
    }

    /* behavior::object_like */
    native_bool equal(object const &) const;
    native_persistent_string to_string() const;
    void to_string(fmt::memory_buffer &buff) const;
    native_persistent_string to_code_string() const;
    native_hash to_hash() const;

    /* behavior::metadatable */
    persistent_list_ptr with_meta(object_ptr m) const;

    /* behavior::seqable */
    obj::persistent_list_sequence_ptr seq() const;
    obj::persistent_list_sequence_ptr fresh_seq() const;

    /* behavior::countable */
    size_t count() const;

    /* behavior::conjable */
    persistent_list_ptr conj(object_ptr head) const;

    /* behavior::sequenceable */
    object_ptr first() const;
    obj::persistent_list_sequence_ptr next() const;

    /* behavior::sequenceable_in_place */
    obj::persistent_list_sequence_ptr next_in_place() const;

    /* behavior::stackable */
    object_ptr peek() const;
    persistent_list_ptr pop() const;

    object base{ object_type::persistent_list };
    value_type data;
    option<object_ptr> meta;
  };
}
