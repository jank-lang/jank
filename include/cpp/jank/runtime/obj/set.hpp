#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/behavior/seqable.hpp>

namespace jank::runtime::obj
{
  struct set : object, behavior::seqable, pool_item_base<set>
  {
    set() = default;
    set(set &&) = default;
    set(set const &) = default;
    set(runtime::detail::set_type &&d);
    set(runtime::detail::set_type const &d);
    ~set() = default;

    runtime::detail::boolean_type equal(object const &) const override;
    runtime::detail::string_type to_string() const override;
    runtime::detail::integer_type to_hash() const override;

    set const* as_set() const override;
    behavior::seqable const* as_seqable() const override;

    behavior::sequence_ptr seq() const override;

    runtime::detail::set_type data;
  };
}
