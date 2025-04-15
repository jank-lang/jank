#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/detail/type.hpp>

namespace jank::runtime::obj
{
  using transient_sorted_set_ref = oref<struct transient_sorted_set>;
  using persistent_sorted_set_ref = oref<struct persistent_sorted_set>;
  using persistent_sorted_set_sequence_ref = oref<struct persistent_sorted_set_sequence>;

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
    persistent_sorted_set(object_ref meta, value_type &&d);
    persistent_sorted_set(jtl::option<object_ref> const &meta, value_type &&d);

    template <typename... Args>
    persistent_sorted_set(std::in_place_t, Args &&...args)
      : data{ std::forward<Args>(args)... }
    {
    }

    template <typename... Args>
    persistent_sorted_set(object_ref const meta, std::in_place_t, Args &&...args)
      : data{ std::forward<Args>(args)... }
      , meta{ meta }
    {
    }

    static persistent_sorted_set_ref empty();
    static persistent_sorted_set_ref create_from_seq(object_ref const seq);

    /* behavior::object_like */
    native_bool equal(object const &) const;
    jtl::immutable_string to_string() const;
    void to_string(util::string_builder &buff) const;
    jtl::immutable_string to_code_string() const;
    native_hash to_hash() const;

    /* behavior::metadatable */
    persistent_sorted_set_ref with_meta(object_ref m) const;

    /* behavior::seqable */
    persistent_sorted_set_sequence_ref seq() const;
    persistent_sorted_set_sequence_ref fresh_seq() const;

    /* behavior::countable */
    size_t count() const;

    /* behavior::conjable */
    persistent_sorted_set_ref conj(object_ref head) const;

    /* behavior::callable */
    object_ref call(object_ref);

    /* behavior::transientable */
    obj::transient_sorted_set_ref to_transient() const;

    native_bool contains(object_ref o) const;
    persistent_sorted_set_ref disj(object_ref o) const;

    object base{ obj_type };
    value_type data;
    jtl::option<object_ref> meta;
  };
}

namespace jank::runtime
{
  extern template obj::persistent_sorted_set_ref
  make_box<obj::persistent_sorted_set>(obj::persistent_sorted_set::value_type &&);
  extern template obj::persistent_sorted_set_ref
  make_box<obj::persistent_sorted_set>(obj::persistent_sorted_set::value_type const &);
}
