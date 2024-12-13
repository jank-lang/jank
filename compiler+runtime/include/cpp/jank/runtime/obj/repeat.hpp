#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/obj/cons.hpp>
#include <jank/runtime/behavior/seqable.hpp>

namespace jank::runtime::obj
{
  using repeat_ptr = native_box<struct repeat>;

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
    native_persistent_string to_string();
    void to_string(fmt::memory_buffer &buff);
    native_persistent_string to_code_string();
    native_hash to_hash() const;

    /* behavior::seqable */
    repeat_ptr seq();
    repeat_ptr fresh_seq() const;

    /* behavior::sequenceable */
    object_ptr first() const;
    repeat_ptr next() const;

    /* behavior::sequenceable_in_place */
    repeat_ptr next_in_place();

    /* behavior::conjable */
    obj::cons_ptr conj(object_ptr head) const;

    /* behavior::metadatable */
    repeat_ptr with_meta(object_ptr m) const;

    object base{ object_type::repeat };
    object_ptr value{};
    object_ptr count{};
    option<object_ptr> meta{};
  };
}
