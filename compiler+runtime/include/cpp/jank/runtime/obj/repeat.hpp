#pragma once

#include <jtl/option.hpp>

#include <jank/runtime/object.hpp>

namespace jank::runtime::obj
{
  using cons_ref = oref<struct cons>;
  using repeat_ref = oref<struct repeat>;

  struct repeat : gc
  {
    static constexpr object_type obj_type{ object_type::repeat };
    static constexpr bool pointer_free{ false };
    static constexpr bool is_sequential{ true };
    static constexpr i64 infinite{ -1 };

    repeat() = default;
    repeat(object_ref value);
    repeat(object_ref count, object_ref value);

    static object_ref create(object_ref value);
    static object_ref create(object_ref count, object_ref value);

    /* behavior::object_like */
    bool equal(object const &) const;
    jtl::immutable_string to_string();
    void to_string(util::string_builder &buff);
    jtl::immutable_string to_code_string();
    uhash to_hash() const;

    /* behavior::seqable */
    repeat_ref seq();
    repeat_ref fresh_seq() const;

    /* behavior::sequenceable */
    object_ref first() const;
    repeat_ref next() const;

    /* behavior::sequenceable_in_place */
    repeat_ref next_in_place();

    /* behavior::conjable */
    obj::cons_ref conj(object_ref head) const;

    /* behavior::metadatable */
    repeat_ref with_meta(object_ref m) const;

    object base{ obj_type };
    object_ref value{};
    object_ref count{};
    jtl::option<object_ref> meta{};
  };
}
