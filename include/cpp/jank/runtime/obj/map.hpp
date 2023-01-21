#pragma once

#include <jank/runtime/behavior/seqable.hpp>
#include <jank/runtime/behavior/countable.hpp>
#include <jank/runtime/behavior/metadatable.hpp>

namespace jank::runtime::obj
{
  struct map : object, behavior::seqable, behavior::countable, behavior::metadatable
  {
    map() = default;
    map(map &&) = default;
    map(map const &) = default;
    map(runtime::detail::persistent_map &&d);
    map(runtime::detail::persistent_map const &d);
    template <typename... Args>
    map(std::in_place_t, Args &&...args)
      : data{ std::in_place, std::forward<Args>(args)... }
    { }
    ~map() = default;

    static native_box<map> create(runtime::detail::persistent_map const &);

    native_bool equal(object const &) const override;
    native_string to_string() const override;
    void to_string(fmt::memory_buffer &buff) const override;
    native_integer to_hash() const override;

    map const* as_map() const override;
    seqable const* as_seqable() const override;

    behavior::sequence_ptr seq() const override;

    size_t count() const override;

    object_ptr with_meta(object_ptr m) const override;
    behavior::metadatable const* as_metadatable() const override;

    runtime::detail::persistent_map data;
  };
  using map_ptr = map*;
}
