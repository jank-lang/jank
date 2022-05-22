#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/behavior/seq.hpp>

namespace jank::runtime::obj
{
  struct map : object, behavior::seqable, pool_item_base<map>
  {
    map() = default;
    map(map &&) = default;
    map(map const &) = default;
    map(runtime::detail::map_type &&d)
      : data{ std::move(d) }
    { }
    map(runtime::detail::map_type const &d)
      : data{ d }
    { }

    runtime::detail::boolean_type equal(object const &) const override;
    runtime::detail::string_type to_string() const override;
    runtime::detail::integer_type to_hash() const override;

    map const* as_map() const override;
    seqable const* as_seqable() const override;

    behavior::sequence_pointer seq() const override;

    runtime::detail::map_type data;
  };
}
