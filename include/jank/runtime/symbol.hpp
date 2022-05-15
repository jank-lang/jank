#pragma once

#include <jank/runtime/object.hpp>

namespace jank::runtime
{
  struct symbol : object, pool_item_base<symbol>
  {
    symbol() = default;
    symbol(symbol &&) = default;
    symbol(symbol const &) = default;
    symbol(detail::string_type &&d)
      : name{ std::move(d) }
    { }
    symbol(detail::string_type &&ns, detail::string_type &&n)
      : ns{ ns }, name{ std::move(n) }
    { }

    detail::boolean_type equal(object const &) const override;
    detail::string_type to_string() const override;
    detail::integer_type to_hash() const override;

    symbol const* as_symbol() const override;

    detail::string_type ns;
    detail::string_type name;
  };
}
