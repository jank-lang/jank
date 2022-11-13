#pragma once

#include <jank/runtime/object.hpp>

namespace jank::runtime::obj
{
  /* TODO: Seqable. */
  struct string : object, pool_item_base<string>
  {
    string() = default;
    string(string &&) = default;
    string(string const &) = default;
    string(runtime::detail::string_type const &d);
    string(runtime::detail::string_type &&d);
    ~string() = default;

    runtime::detail::boolean_type equal(object const &) const override;
    runtime::detail::string_type to_string() const override;
    runtime::detail::integer_type to_hash() const override;

    string const* as_string() const override;

    runtime::detail::string_type data;
  };
  using string_ptr = runtime::detail::box_type<string>;
}
