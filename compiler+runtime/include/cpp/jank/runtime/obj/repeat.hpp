#pragma once

#include <jank/runtime/behavior/seqable.hpp>

namespace jank::runtime
{
  template <>
  struct static_object<object_type::repeat> : gc
  {
    static constexpr native_bool pointer_free{ false };
    static constexpr native_bool is_sequential{ true };
    static constexpr native_integer infinite{ -1 };

    static_object() = default;
    static_object(object_ptr value);
    static_object(object_ptr count, object_ptr value);

    static object_ptr create(object_ptr value);
    static object_ptr create(object_ptr count, object_ptr value);

    /* behavior::object_like */
    native_bool equal(object const &) const;
    native_persistent_string to_string();
    void to_string(fmt::memory_buffer &buff);
    native_persistent_string to_code_string();
    native_hash to_hash() const;

    /* behavior::seqable */
    native_box<static_object> seq();
    native_box<static_object> fresh_seq() const;

    /* behavior::sequenceable */
    object_ptr first() const;
    native_box<static_object> next() const;

    /* behavior::sequenceable_in_place */
    native_box<static_object> next_in_place();

    /* behavior::conjable */
    obj::cons_ptr conj(object_ptr head) const;

    /* behavior::metadatable */
    native_box<static_object> with_meta(object_ptr m) const;

    object base{ object_type::repeat };
    object_ptr value{};
    object_ptr count{};
    option<object_ptr> meta{};
  };

  namespace obj
  {
    using repeat = static_object<object_type::repeat>;
    using repeat_ptr = native_box<repeat>;
  }
}
