#pragma once

#include <jank/runtime/behavior/seqable.hpp>
#include <jank/runtime/behavior/countable.hpp>
#include <jank/runtime/behavior/metadatable.hpp>

namespace jank::runtime::obj
{
  struct set
    :
      virtual object,
      behavior::seqable, behavior::countable,
      behavior::metadatable
  {
    set() = default;
    set(set &&) = default;
    set(set const &) = default;
    set(runtime::detail::persistent_set &&d);
    set(runtime::detail::persistent_set const &d);
    ~set() = default;

    native_bool equal(object const &) const final;
    native_string to_string() const final;
    void to_string(fmt::memory_buffer &buff) const final;
    native_integer to_hash() const final;

    set const* as_set() const final;
    behavior::seqable const* as_seqable() const final;

    behavior::sequence_ptr seq() const final;

    size_t count() const final;

    object_ptr with_meta(object_ptr m) const final;
    behavior::metadatable const* as_metadatable() const final;

    runtime::detail::persistent_set data;
  };
}
