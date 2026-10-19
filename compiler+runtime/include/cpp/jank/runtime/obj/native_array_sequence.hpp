#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/lazy_meta.hpp>

namespace jank::runtime::obj
{
  using native_array_sequence_ref = oref<struct native_array_sequence>;
  using cons_ref = oref<struct cons>;

  struct native_array_sequence : object
  {
    static constexpr object_type obj_type{ object_type::native_array_sequence };
    static constexpr object_behavior obj_behaviors{ object_behavior::seqable
                                                    | object_behavior::sequence_like
                                                    | object_behavior::sequence_like_in_place };
    static constexpr bool pointer_free{ false };
    static constexpr bool is_sequential{ true };

    native_array_sequence() = delete;
    native_array_sequence(native_array_sequence &&) noexcept = default;
    native_array_sequence(native_array_sequence const &) = default;
    native_array_sequence(object_ref * const arr, usize const size);
    native_array_sequence(object_ref * const arr, usize const index, usize const size);

    template <typename... Args>
    native_array_sequence(object_ref const first, Args const &...rest)
      : object{ obj_type, obj_behaviors }
      , arr{ make_array_box<object_ref>(first, rest...) }
      , size{ sizeof...(Args) + 1 }
    {
    }

    /* behavior::object_like */
    bool equal(object const &o) const override;
    void to_string(jtl::string_builder &buff) const override;
    jtl::immutable_string to_string() const override;
    jtl::immutable_string to_code_string() const override;
    uhash to_hash() const override;

    /* behavior::seqable */
    object_ref seq() const override;
    object_ref fresh_seq() const override;

    /* behavior::countable */
    usize count() const;

    /* behavior::metadatable */
    native_array_sequence_ref with_meta(object_ref const m) const;
    object_ref get_meta() const;
    void set_meta(object_ref const o);

    /* behavior::sequence_like */
    object_ref first() const override;
    object_ref next() const override;

    /* behavior::conjable */
    obj::cons_ref conj(object_ref const head);

    /* behavior::sequence_like_in_place */
    object_ref next_in_place() override;

    /*** XXX: Everything here is immutable after initialization. ***/
    jtl::ptr<object_ref> arr{};
    usize index{};
    usize size{};

    /*** XXX: Everything here is thread-safe. ***/
  protected:
    lazy_meta meta;
  };
}
