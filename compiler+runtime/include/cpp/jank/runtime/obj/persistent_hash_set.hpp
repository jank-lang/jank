#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/detail/type.hpp>

namespace jank::runtime::obj
{
  using transient_hash_set_ref = oref<struct transient_hash_set>;
  using persistent_hash_set_ref = oref<struct persistent_hash_set>;
  using persistent_hash_set_sequence_ref = oref<struct persistent_hash_set_sequence>;

  struct persistent_hash_set : object
  {
    static constexpr object_type obj_type{ object_type::persistent_hash_set };
    static constexpr object_behavior obj_behaviors{ object_behavior::call | object_behavior::get };
    static constexpr bool pointer_free{ false };
    static constexpr bool is_set_like{ true };

    using value_type = runtime::detail::native_persistent_hash_set;

    persistent_hash_set();
    persistent_hash_set(persistent_hash_set &&) noexcept = default;
    persistent_hash_set(persistent_hash_set const &) = default;
    persistent_hash_set(value_type &&d);
    persistent_hash_set(value_type const &d);
    persistent_hash_set(object_ref const meta, value_type &&d);

    template <typename... Args>
    persistent_hash_set(std::in_place_t, Args &&...args)
      : object{ obj_type, obj_behaviors }
      , data{ std::forward<Args>(args)... }
    {
    }

    template <typename... Args>
    persistent_hash_set(object_ref const meta, std::in_place_t, Args &&...args)
      : object{ obj_type, obj_behaviors }
      , data{ std::forward<Args>(args)... }
      , meta{ meta }
    {
    }

    static persistent_hash_set_ref empty();

    static persistent_hash_set_ref create_from_seq(object_ref const seq);

    /* behavior::object_like */
    bool equal(object const &) const override;
    jtl::immutable_string to_string() const override;
    void to_string(jtl::string_builder &buff) const override;
    jtl::immutable_string to_code_string() const override;
    uhash to_hash() const override;

    /* behavior::metadatable */
    persistent_hash_set_ref with_meta(object_ref const m) const;
    object_ref get_meta() const;

    /* behavior::seqable */
    obj::persistent_hash_set_sequence_ref seq() const;
    obj::persistent_hash_set_sequence_ref fresh_seq() const;

    /* behavior::countable */
    usize count() const;

    /* behavior::conjable */
    persistent_hash_set_ref conj(object_ref const head) const;

    /* behavior::call */
    using object::call;
    object_ref call(object_ref const) const override;

    /* behavior::transientable */
    obj::transient_hash_set_ref to_transient() const;

    /* behavior::get */
    object_ref get(object_ref const key) const override;
    object_ref get(object_ref const key, object_ref const fallback) const override;
    bool contains(object_ref const o) const override;

    persistent_hash_set_ref disj(object_ref const o) const;

    /*** XXX: Everything here is immutable after initialization. ***/
    value_type data;
    object_ref meta;
  };
}
