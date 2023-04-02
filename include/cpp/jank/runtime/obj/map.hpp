#pragma once

#include <jank/runtime/behavior/seqable.hpp>
#include <jank/runtime/behavior/countable.hpp>
#include <jank/runtime/behavior/metadatable.hpp>
#include <jank/runtime/behavior/associatively_readable.hpp>
#include <jank/runtime/behavior/associatively_writable.hpp>

namespace jank::runtime::obj
{
  struct map
    :
      object,
      behavior::seqable, behavior::countable,
      behavior::metadatable,
      behavior::associatively_readable,
      behavior::associatively_writable
  {
    using value_type = runtime::detail::persistent_map;

    map() = default;
    map(map &&) = default;
    map(map const &) = default;
    map(runtime::detail::persistent_map &&d);
    map(runtime::detail::persistent_map const &d);
    template <typename... Args>
    map(runtime::detail::in_place_unique, Args &&...args)
      : data{ runtime::detail::in_place_unique{}, std::forward<Args>(args)... }
    { }
    ~map() = default;

    static native_box<map> create(runtime::detail::persistent_map const &);

    native_bool equal(object const &) const final;
    native_string to_string() const final;
    void to_string(fmt::memory_buffer &buff) const final;
    native_integer to_hash() const final;

    map const* as_map() const final;
    seqable const* as_seqable() const final;

    behavior::sequence_ptr seq() const final;

    size_t count() const final;

    object_ptr with_meta(object_ptr m) const final;
    behavior::metadatable const* as_metadatable() const final;

    behavior::associatively_readable const* as_associatively_readable() const final;
    object_ptr get(object_ptr key) const final;
    object_ptr get(object_ptr key, object_ptr fallback) const final;

    behavior::associatively_writable const* as_associatively_writable() const final;
    object_ptr assoc(object_ptr key, object_ptr val) const final;

    runtime::detail::persistent_map data;
  };
  using map_ptr = map*;
}
