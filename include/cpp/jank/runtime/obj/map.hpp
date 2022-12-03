#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/behavior/seqable.hpp>

namespace jank::runtime::obj
{
  struct map : object, behavior::seqable, pool_item_base<map>
  {
    map() = default;
    map(map &&) = default;
    map(map const &) = default;
    map(runtime::detail::map_type &&d);
    map(runtime::detail::map_type const &d);
    template <typename... Args>
    map(std::in_place_t, Args &&...args)
      : data{ std::in_place, std::forward<Args>(args)... }
    { }
    ~map() = default;

    runtime::detail::boolean_type equal(object const &) const override;
    runtime::detail::string_type to_string() const override;
    runtime::detail::integer_type to_hash() const override;

    map const* as_map() const override;
    seqable const* as_seqable() const override;

    behavior::sequence_ptr seq() const override;

    runtime::detail::map_type data;
  };
  using map_ptr = runtime::detail::box_type<map>;
}
