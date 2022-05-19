#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/behavior/seq.hpp>

namespace jank::runtime::type
{
  struct vector : object, behavior::seqable, pool_item_base<vector>
  {
    vector() = default;
    vector(vector &&) = default;
    vector(vector const &) = default;
    vector(runtime::detail::vector_type &&d)
      : data{ std::move(d) }
    { }
    vector(runtime::detail::vector_type const &d)
      : data{ d }
    { }

    runtime::detail::boolean_type equal(object const &) const override;
    runtime::detail::string_type to_string() const override;
    runtime::detail::integer_type to_hash() const override;

    vector const* as_vector() const override;
    behavior::seqable const* as_seqable() const override;

    behavior::sequence_pointer seq() const override;

    runtime::detail::vector_type data;
  };
}
