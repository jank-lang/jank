#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/detail/type.hpp>

namespace jank::runtime::obj
{
  using transient_sorted_set_ptr = native_box<struct transient_sorted_set>;
  using persistent_sorted_set_ptr = native_box<struct persistent_sorted_set>;
  using persistent_sorted_set_sequence_ptr = native_box<struct persistent_sorted_set_sequence>;

  struct persistent_sorted_set : gc
  {
    static constexpr object_type obj_type{ object_type::persistent_sorted_set };
    static constexpr native_bool pointer_free{ false };
    static constexpr native_bool is_set_like{ true };

    using value_type = runtime::detail::native_persistent_sorted_set;

    persistent_sorted_set() = default;
    persistent_sorted_set(persistent_sorted_set &&) noexcept = default;
    persistent_sorted_set(persistent_sorted_set const &) = default;
    persistent_sorted_set(value_type &&d);
    persistent_sorted_set(value_type const &d);
    persistent_sorted_set(object_ptr meta, value_type &&d);
    persistent_sorted_set(option<object_ptr> const &meta, value_type &&d);

    template <typename... Args>
    persistent_sorted_set(std::in_place_t, Args &&...args)
      : data{ std::forward<Args>(args)... }
    {
    }

    template <typename... Args>
    persistent_sorted_set(object_ptr const meta, std::in_place_t, Args &&...args)
      : data{ std::forward<Args>(args)... }
      , meta{ meta }
    {
    }

    static persistent_sorted_set_ptr empty();
    static persistent_sorted_set_ptr create_from_seq(object_ptr const seq);

    /* behavior::object_like */
    native_bool equal(object const &) const;
    native_persistent_string to_string() const;
    void to_string(util::string_builder &buff) const;
    native_persistent_string to_code_string() const;
    native_hash to_hash() const;

    /* behavior::metadatable */
    persistent_sorted_set_ptr with_meta(object_ptr m) const;

    /* behavior::seqable */
    persistent_sorted_set_sequence_ptr seq() const;
    persistent_sorted_set_sequence_ptr fresh_seq() const;

    /* behavior::countable */
    size_t count() const;

    /* behavior::conjable */
    persistent_sorted_set_ptr conj(object_ptr head) const;

    /* behavior::callable */
    object_ptr call(object_ptr);

    /* behavior::transientable */
    obj::transient_sorted_set_ptr to_transient() const;

    native_bool contains(object_ptr o) const;
    persistent_sorted_set_ptr disj(object_ptr o) const;

    object base{ obj_type };
    value_type data;
    option<object_ptr> meta;
  };
}

namespace jank::runtime
{
  extern template obj::persistent_sorted_set_ptr
  make_box<obj::persistent_sorted_set>(obj::persistent_sorted_set::value_type &&);
  extern template obj::persistent_sorted_set_ptr
  make_box<obj::persistent_sorted_set>(obj::persistent_sorted_set::value_type const &);
}
