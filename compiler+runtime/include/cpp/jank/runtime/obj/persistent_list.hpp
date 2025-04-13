#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/detail/native_persistent_list.hpp>

namespace jank::runtime::obj
{
  using persistent_list_ref = jtl::oref<struct persistent_list>;
  using persistent_list_sequence_ref = jtl::oref<struct persistent_list_sequence>;

  struct persistent_list : gc
  {
    using value_type = runtime::detail::native_persistent_list;

    static constexpr object_type obj_type{ object_type::persistent_list };
    static constexpr native_bool pointer_free{ false };
    static constexpr native_bool is_sequential{ true };

    /* Create from a sequence. */
    static persistent_list_ref create(object_ptr meta, object_ptr s);
    static persistent_list_ref create(object_ptr s);
    static persistent_list_ref create(persistent_list_ref s);

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

    static persistent_list_ref empty()
    {
      static auto const ret(make_box<persistent_list>());
      return ret;
    }

    /* behavior::object_like */
    native_bool equal(object const &) const;
    jtl::immutable_string to_string() const;
    void to_string(util::string_builder &buff) const;
    jtl::immutable_string to_code_string() const;
    native_hash to_hash() const;

    /* behavior::metadatable */
    persistent_list_ref with_meta(object_ptr m) const;

    /* behavior::seqable */
    obj::persistent_list_sequence_ref seq() const;
    obj::persistent_list_sequence_ref fresh_seq() const;

    /* behavior::countable */
    size_t count() const;

    /* behavior::conjable */
    persistent_list_ref conj(object_ptr head) const;

    /* behavior::sequenceable */
    object_ptr first() const;
    obj::persistent_list_sequence_ref next() const;

    /* behavior::sequenceable_in_place */
    obj::persistent_list_sequence_ref next_in_place() const;

    /* behavior::stackable */
    object_ptr peek() const;
    persistent_list_ref pop() const;

    object base{ obj_type };
    value_type data;
    jtl::option<object_ptr> meta;
  };
}
