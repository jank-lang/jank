#pragma once

#include <jank/runtime/object.hpp>
#include <jank/option.hpp>

namespace jank::runtime::obj
{
  using cons_ptr = native_box<struct cons>;
  using repeat_ptr = native_box<struct repeat>;

  struct repeat : gc
  {
    static constexpr object_type obj_type{ object_type::repeat };
    static constexpr native_bool pointer_free{ false };
    static constexpr native_bool is_sequential{ true };
    static constexpr size_t infinite{ 0 };

    repeat() = default;
    repeat(object_ptr value);
    repeat(size_t count, object_ptr value);

    static object_ptr create(object_ptr value);
    static object_ptr create(object_ptr count, object_ptr value);

    /* behavior::object_like */
    native_bool equal(object const &) const;
    native_persistent_string to_string();
    void to_string(util::string_builder &buff);
    native_persistent_string to_code_string();
    native_hash to_hash() const;

    /* behavior::seqable */
    repeat_ptr seq();
    repeat_ptr fresh_seq() const;

    /* behavior::reduceable */
    object_ptr reduce(std::function<object_ptr(object_ptr, object_ptr)> const &f, object_ptr init) const;

    /* behavior::sequenceable */
    object_ptr first() const;
    repeat_ptr next() const;

    /* behavior::sequenceable_in_place */
    repeat_ptr next_in_place();

    /* behavior::conjable */
    obj::cons_ptr conj(object_ptr head) const;

    /* behavior::metadatable */
    repeat_ptr with_meta(object_ptr m) const;

    object base{ obj_type };
    object_ptr value{};
    size_t count{};
    option<object_ptr> meta{};
  };
}
