#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/detail/native_persistent_list.hpp>

namespace jank::runtime::obj
{
  using nil_ref = oref<struct nil>;
  using persistent_list_ref = oref<struct persistent_list>;

  struct persistent_list : object
  {
    using value_type = runtime::detail::native_persistent_list;

    static constexpr object_type obj_type{ object_type::persistent_list };
    static constexpr bool pointer_free{ false };
    static constexpr bool is_sequential{ true };

    /* Create from a sequence. */
    static persistent_list_ref create(object_ref const meta, object_ref const s);
    static persistent_list_ref create(object_ref const s);
    static persistent_list_ref create(persistent_list_ref const s);
    static persistent_list_ref create(nil_ref const s);

    persistent_list();
    persistent_list(persistent_list &&) noexcept = default;
    persistent_list(persistent_list const &) = default;
    persistent_list(value_type const &d);
    persistent_list(jtl::option<object_ref> const &meta, value_type const &d);

    /* TODO: This is broken when `args` is a value_type list we're looking to wrap in another list.
     * It just uses the copy ctor. */
    template <typename... Args>
    persistent_list(std::in_place_t, Args &&...args)
      : object{ obj_type }
      , data{ std::forward<Args>(args)... }
    {
    }

    template <typename... Args>
    persistent_list(object_ref const meta, std::in_place_t, Args &&...args)
      : object{ obj_type }
      , data{ std::forward<Args>(args)... }
      , meta{ meta }
    {
    }

    static persistent_list_ref empty()
    {
      static auto const ret(make_box<persistent_list>());
      return ret;
    }

    /* behavior::object_like */
    bool equal(object const &) const override;
    jtl::immutable_string to_string() const override;
    void to_string(jtl::string_builder &buff) const override;
    jtl::immutable_string to_code_string() const override;
    uhash to_hash() const override;

    /* behavior::metadatable */
    persistent_list_ref with_meta(object_ref const m) const;

    /* behavior::seqable */
    obj::persistent_list_ref seq() const;
    obj::persistent_list_ref fresh_seq() const;

    /* behavior::countable */
    usize count() const;

    /* behavior::conjable */
    persistent_list_ref conj(object_ref const head) const;

    /* behavior::sequenceable */
    object_ref first() const;
    obj::persistent_list_ref next() const;

    /* behavior::sequenceable_in_place */
    obj::persistent_list_ref next_in_place();

    /* behavior::stackable */
    object_ref peek() const;
    persistent_list_ref pop() const;

    /*** XXX: Everything here is immutable after initialization. ***/
    value_type data;
    jtl::option<object_ref> meta;
  };
}
