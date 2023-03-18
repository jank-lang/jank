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

    native_bool equal(object const &) const override;
    native_string to_string() const override;
    void to_string(fmt::memory_buffer &buff) const override;
    native_integer to_hash() const override;

    set const* as_set() const override;
    behavior::seqable const* as_seqable() const override;

    behavior::sequence_ptr seq() const override;

    size_t count() const override;

    object_ptr with_meta(object_ptr m) const override;
    behavior::metadatable const* as_metadatable() const override;

    runtime::detail::persistent_set data;
  };
}
