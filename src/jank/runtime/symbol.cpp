#include <iostream>
#include <sstream>

#include <jank/runtime/symbol.hpp>

namespace jank::runtime
{
  detail::boolean_type symbol::equal(object const &o) const
  {
    auto const *s(o.as_symbol());
    if(!s)
    { return false; }

    return ns == s->ns && name == s->name;
  }
  detail::string_type symbol::to_string() const
  {
    if(ns.length > 0)
    { return ns + "/" + name; }
    return name;
  }
  detail::integer_type symbol::to_hash() const
  /* TODO: Cache this. */
  { return ns.to_hash() + name.to_hash(); }

  symbol const* symbol::as_symbol() const
  { return this; }
}
