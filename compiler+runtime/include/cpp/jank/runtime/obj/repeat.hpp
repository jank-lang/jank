#pragma once

#include <jtl/option.hpp>

#include <jank/runtime/object.hpp>

namespace jank::runtime::obj
{
  using cons_ref = jtl::object_ref<struct cons>;
  using repeat_ref = jtl::object_ref<struct repeat>;

  struct repeat : gc
  {
    static constexpr object_type obj_type{ object_type::repeat };
    static constexpr native_bool pointer_free{ false };
    static constexpr native_bool is_sequential{ true };
    static constexpr native_integer infinite{ -1 };

    repeat() = default;
    repeat(object_ptr value);
    repeat(object_ptr count, object_ptr value);

    static object_ptr create(object_ptr value);
    static object_ptr create(object_ptr count, object_ptr value);

    /* behavior::object_like */
    native_bool equal(object const &) const;
    jtl::immutable_string to_string();
    void to_string(util::string_builder &buff);
    jtl::immutable_string to_code_string();
    native_hash to_hash() const;

    /* behavior::seqable */
    repeat_ref seq();
    repeat_ref fresh_seq() const;

    /* behavior::sequenceable */
    object_ptr first() const;
    repeat_ref next() const;

    /* behavior::sequenceable_in_place */
    repeat_ref next_in_place();

    /* behavior::conjable */
    obj::cons_ref conj(object_ptr head) const;

    /* behavior::metadatable */
    repeat_ref with_meta(object_ptr m) const;

    object base{ obj_type };
    object_ptr value{};
    object_ptr count{};
    jtl::option<object_ptr> meta{};
  };
}
